#!/bin/sh
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --constraint=AMD
#SBATCH --time=00:05:00
#SBATCH --output=N_body_benchmark.log
#SBATCH --reservation=fri

export OMP_PLACES=cores
export OMP_PROC_BIND=TRUE 
export OMP_NUM_THREADS=32

./N_body_openmp $1
