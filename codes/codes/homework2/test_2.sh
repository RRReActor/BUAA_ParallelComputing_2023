#!/bin/bash
#SBATCH -J waysome_hw2   #作业名
#SBATCH -p cpu-quota
#SBATCH -N 4            #4 节点
#SBATCH -n 4           #28 核
#SBATCH -o waysome_hw2.out # 将标准输出结果
#SBATCH -e waysome_hw2.err # 将错误输出结果

srun hostname | sort > machinefile.${SLURM_JOB_ID}
NP=`cat machinefile.${SLURM_JOB_ID} | wc -l`
module load intel/19.0.5.281
export I_MPI_HYDRA_TOPOLIB=ipl
mpirun -genv I_MPI_FABRICS shm:dapl -np ${NP} -f ./machinefile.${SLURM_JOB_ID} ./tes


