#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>

#define NumOfCores 8

struct threadArg
{
    int tid;
    double *B;
    double *A_row;
    double *C_row;
    int numthreads;
};

int M, N, P;

void *worker(void *arg)
{
    int i, j;
    struct threadArg *myarg = (struct threadArg *)arg;
    for (i = myarg->tid; i < P; i += myarg->numthreads)
    {
        // 平均分配B的所有列
        myarg->C_row[i] = 0.0;
        for (j = 0; j < N; j++)
        {
            myarg->C_row[i] += myarg->A_row[j] * myarg->B[j * P + i];
            // B中的一列与A行相乘，计算结果存入C一行中的对应位置
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int i, j, myid, numprocs, numsend, sender, numthreads;
    double *A_row, *C_row;
    pthread_t *tids;
    struct threadArg *targs;
    MPI_Status status;
    FILE *matrixA, *matrixB, *matrixC;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    if (!myid)
    {
        matrixA = fopen("input1.txt", "r");
        matrixB = fopen("input2.txt", "r");
        fread(&M, sizeof(int), 1, matrixA);
        fread(&N, sizeof(int), 1, matrixA);
        fread(&N, sizeof(int), 1, matrixB);
        fread(&P, sizeof(int), 1, matrixB);
    }
    // 广播M，N，P
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&P, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    // 声明数组
    double A[M][N], B[N][P], C[M][P];
    int done[1000] = {0};
    if (!myid)
    { // 在0进程内初始化矩阵A和B
        for (i = 0; i < M; i++)
            for (j = 0; j < N; j++)
                fread(&A[i][j], sizeof(double), 1, matrixA);
        for (i = 0; i < N; i++)
            for (j = 0; j < P; j++)
                fread(&B[i][j], sizeof(double), 1, matrixB);
        fclose(matrixA);
        fclose(matrixB);
    }
    MPI_Bcast(B[0], N * P, MPI_DOUBLE, 0, MPI_COMM_WORLD); // 广播矩阵B
    if (!myid)
    { /*0进程：分配任务和回收结果*/
        j = (numprocs - 1) < M ? (numprocs - 1) : M;
        for (i = 1; i <= j; i++)
        {
            MPI_Send(A[i - 1], N, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
            // 给从进程依次依次分配A的一行
        }
        numsend = j;
        for (i = 1; i <= M; i++)
        {
            sender = (i - 1) % (numprocs - 1) + 1;
            MPI_Recv(C[i - 1], P, MPI_DOUBLE, sender, 100, MPI_COMM_WORLD,
                     &status); // 依次回收计算结果
            if (numsend < M)
            { // 如果A还没有分配完,向返回计算结果的从进程再分配A中的一行
                MPI_Send(A[numsend], N, MPI_DOUBLE, sender, 99, MPI_COMM_WORLD);
                numsend++;
            }
            else
            {
                MPI_Send(A[0], N, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
                done[sender] = 1; // 标记进程已经结束计算
            }
        }
        for (i = 1; i <= numprocs - 1; i++)
        {
            if (done[i] == 0)
            { // 把还未发送结束信号的进程补上
                MPI_Send(A[0], N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            }
        }

        matrixC = fopen("solve_out.txt", "w");
        int fsizec = sizeof(int) * 2 + sizeof(double) * M * P;
        char *fstreamc = (char *)malloc(fsizec);
        ((int *)fstreamc)[0] = M;
        ((int *)fstreamc)[1] = P;
        double *pc = (double *)(fstreamc + sizeof(int) * 2);
        for (i = 0; i < M; i++)
            for (j = 0; j < P; j++)
                *(pc + i * P + j) = C[i][j]; // 输出矩阵C
        fwrite(fstreamc, sizeof(char), fsizec, matrixC);
        fclose(matrixC);
        free(fstreamc);
    }
    else
    { // 从进程
        /*从进程(myid > 0)：接收0进程发来的任务， 计算完毕发回主进程*/
        numthreads = NumOfCores;                                    // 从进程所在节点的CPU数
        tids = (pthread_t *)malloc(numthreads * sizeof(pthread_t)); // tids数组用来存放线程ID
        A_row = (double *)malloc(N * sizeof(double));               // 存放主进程分配的A中一行
        C_row = (double *)malloc(P * sizeof(double));               // 存放计算结果C中的一行
        targs = (struct threadArg *)malloc(numthreads * sizeof(struct threadArg)); // 线程参数，由于传入参数比较多，采用结果传递
        for (i = 0; i < numthreads; i++)
        {
          
            targs[i].tid = i;
            targs[i].B = B[0];
            targs[i].A_row = A_row;
            targs[i].C_row = C_row;
            targs[i].numthreads = numthreads;
        }
        while (1)
        {
            MPI_Recv(A_row, N, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                     &status);       // 接收0进程发送来A的一行
            if (status.MPI_TAG == 0) // 若接收到标识为0的消息则退出执行
                break;
            for (i = 0; i < numthreads; i++)
            {
                pthread_create(&tids[i], NULL, worker, &targs[i]); // 创建线程执行计算
            }
            for (i = 0; i < numthreads; i++)
            {
                pthread_join(tids[i], NULL); // 等待线程内的计算完成
            }
            MPI_Send(C_row, P, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD);
        }
        // 释放内存
        free(tids);
        free(A_row);
        free(C_row);
        free(targs);
        // 其他进程接收任务、计算、返回计算结果
    } /*从进程结束*/
    MPI_Finalize();
    return 0;
}