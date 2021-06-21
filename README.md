# N-body

## Input

```
number_of_bodies
mass pos_x pos_y pos_z vel_x vel_y vel_z
```

## Output

```
number_of_bodies
number_of_steps
mass_1
...
mass_n
body_1_step_1
...
body_1_step_m
body_2_step_1
...
body_n_step_m
```

## Implementations

* Sequential
```bash
g++ -O2 N_body.cpp -o N_body
srun --ntasks=1 --nodes=1 --time=10:00 --reservation=fri N_body
```

* OpenMP
```bash
g++ -O2 -fopenmp N_body_openmp.cpp -o N_body_openmp
sbatch --wait N_body_openmp.sh
```

* MPI
```bash
mpic++ -O2 N_body_mpi.cpp -o N_body_mpi
srun --ntasks=16 --nodes=1 --time=10:00 --reservation=fri --mpi=pmix N_body_mpi
```
