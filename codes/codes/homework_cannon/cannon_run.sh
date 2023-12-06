#!/bin/bash
#SBATCH -J waysome_cannon   # 作业名
#SBATCH -p cpu-1024
#SBATCH -N 1               # 1 节点
#SBATCH -n 9               # 8 核
#SBATCH -o waysome_cannon.out # 将标准输出结果
#SBATCH -e waysome_cannon.err # 将错误输出结果

module load intel/19.0.5.281
export I_MPI_HYDRA_TOPOLIB=ipl

# 创建 machinefile.${SLURM_JOB_ID}
srun hostname | sort > machinefile.${SLURM_JOB_ID}

# 循环调用 ./cannon 100 次
for i in {0..99}; do
    NP=$(cat machinefile.${SLURM_JOB_ID} | wc -l)
    mpirun -genv I_MPI_FABRICS shm:dapl -np ${NP} -f ./machinefile.${SLURM_JOB_ID} ./bin/cannon \
    ./test/testcase_${i}/input/A \
    ./test/testcase_${i}/input/B \
    ./test/testcase_${i}/output/C

    # 使用 ./bin/comp 比较结果并记录到日志
    ./bin/comp ./test/testcase_${i}/input/C ./test/testcase_${i}/output/C | tee ./test/testcase_${i}/output/comparison_log.txt
done
