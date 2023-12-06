#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    // 描述通信子集合大小和当前进程序号
    int rank, size;
    char data = 0;

    // 记录时间
    double start_time, stop_time, cost_time;

    // MPI 初始化
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 记录开始时间
    start_time = MPI_Wtime();

    // 向后发送消息，tag = 0
    MPI_Send(&data, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
    // Recv 是阻塞的，接收到消息时回到运行
    MPI_Recv(&data, 1, MPI_INT, (rank - 1 + size) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    stop_time = MPI_Wtime();
    cost_time = stop_time - start_time;

    printf("Process %d: spend  %f seconds for one full circle of transmission\n", rank, cost_time);

    MPI_Finalize();
    return 0;
}