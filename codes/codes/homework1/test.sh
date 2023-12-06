#!/bin/bash
#SBATCH -J hpc_homework1_test_1   #作业名
#SBATCH -p cpu-quota
#SBATCH -N 3            #3个节点
#SBATCH -n 28           #28个cpu
#SBATCH -o homework1.out # 将屏幕的输出结果保存到当前文件夹的test_hpc_1.out 
#SBATCH -e homework1.err # 将屏幕的输出结果保存到当前文件夹的test_hpc_1.err

srun hostname | sort > machinefile.${SLURM_JOB_ID}
NP=`cat machinefile.${SLURM_JOB_ID} | wc -l`
module load intel/19.0.5.281
export I_MPI_HYDRA_TOPOLIB=ipl
mpirun -genv I_MPI_FABRICS shm:dapl -np ${NP} -f ./machinefile.${SLURM_JOB_ID} ./tes

