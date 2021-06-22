# N-body

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
module load mpi
export OMPI_MCA_btl_openib_allow_ib=1
mpic++ -O2 N_body_mpi.cpp -o N_body_mpi
srun --ntasks=16 --nodes=1 --time=10:00 --reservation=fri --mpi=pmix N_body_mpi
```


## Examples

* [Three body simulation](https://mb0484.github.io/N-body/visualization/?data=three.txt)
* [Solar system simulation](https://mb0484.github.io/N-body/visualization/?data=solar.txt)
* ["Galaxy" simulation](https://mb0484.github.io/N-body/visualization/?data=solar.txt)


## Results

N=256, ITERS=10000
Seq: 10.838952s
OpenMP:
	64: 0.766956s
	32: 0.691735s (15.7x speedup)
	16: 1.039368s
MPI:
	64: 2.618054s
	32: 2.176248s
	16: 1.901016s (5.7x speedup)

N=512, ITERS=10000
Seq: 43.288410s
OpenMP:
	64: 1.653087s (26.2x speedup)
	32: 2.282885s
	16: 3.948944s
MPI:
	64: 5.250112s
	32: 4.751729s (9.1x speedup)
	16: 5.637103

N=1024, ITERS=10000
Seq: 174.416487s
OpenMP:
	64: 4.813251s (36.2x speedup)
	32: 8.388583s
	16: 15.440097s
MPI:
	64: 10.945437s (15.9x speedup)
	32: 13.129810s 
	16: 20.237591s


## Specification

### Input

```
number_of_bodies
mass pos_x pos_y pos_z vel_x vel_y vel_z
```

### Output

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

