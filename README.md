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
srun --ntasks=64 --nodes=1 --time=10:00 --constraint=AMD --mpi=pmix N_body_mpi

mpic++ -O2 N_body_mpi_newton.cpp -o N_body_mpi_newton
srun --ntasks=64 --nodes=1 --time=10:00 --constraint=AMD --mpi=pmix N_body_mpi_newton

mpic++ -O2 N_body_mpi_bh.cpp -o N_body_mpi_bh
srun --ntasks=64 --nodes=1 --time=10:00 --constraint=AMD --mpi=pmix N_body_mpi_bh
```


## Examples

* [Three body simulation](https://mb0484.github.io/N-body/visualization/?data=three.txt)
* [Solar system simulation](https://mb0484.github.io/N-body/visualization/?data=solar.txt)
* ["Galaxy" simulation](https://mb0484.github.io/N-body/visualization/?data=galaxy.txt)


## Updated results

N=256, ITERS=1000
NODES=1, TASKS=64
MPI:		0.293479s
MPI Newton:	0.343271s
	Compute time: 0.016862s (4.9%)
	Comm time:    0.272573s (79.4%)
MPI BH:		0.310539s
	Build time:   0.092551s (30.3%)
	Compute time: 0.017495s (5.7%)
	Dealloc time: 0.046896s (15.3%)
	Comm time:    0.111999s (36.6%)

N=1024, ITERS=1000
NODES=1, TASKS=64
MPI:		1.096000s
MPI Newton:	0.893078s
	Compute time: 0.272614s (30.5%)
	Comm time:    0.555303s (62.2%)
MPI BH:		1.493272s
	Build time:   0.413852s (27.7%)
	Compute time: 0.213384s (14.3%)
	Dealloc time: 0.222156s (14.9%)
	Comm time:    0.598205s (40.1%)

N=4196, ITERS=1000
NODES=1, TASKS=64
MPI:		8.904254s
MPI Newton:	6.637288s
	Compute time: 4.332786s (65.3%)
	Comm time:    2.013649s (30.3%)
MPI BH:		5.785979s
	Build time:   2.035822s (35.2%)
	Compute time: 1.288856s (22.3%)
	Dealloc time: 1.107330s (19.1%)
	Comm time:    1.246339s (21.5%)

N=8192, ITERS=1000
NODES=1, TASKS=64
MPI:		31.589076s, 31.544488s
MPI Newton:	21.946689s, 21.884060s
MPI BH:		16.332684s, 16.073598s

N=16384, ITERS=1000
NODES=1, TASKS=64
MPI:		118.480873s
MPI Newton:	80.467040s
	Compute time: 71.139077s (88.4%)
	Comm time:    6.547080s (8.1%)
MPI BH: 	44.195155s
	Build time:   16.432055s (37.2%)
	Compute time: 12.549165s (28.4%)
	Dealloc time: 7.670168s (17.4%)
	Comm time:    7.155848s (16.2%)

N=16384, ITERS=1000
NODES=2, TASKS=64
MPI:		176.299700s
MPI Newton:	86.232749s
	Compute time: 70.705843s (82.0%)
	Comm time:    12.897818s (15.0%)
MPI BH: 	55.186187s
	Build time:   16.139072s (29.2%)
	Compute time: 12.140654s (22.0%)
	Dealloc time: 7.433197s (13.5%)
	Comm time:    18.478468s (33.5%)


N=32768, ITERS=100
NODES=1, TASKS=64
MPI:		46.459492s, 46.475134s
MPI Newton:	41.302186s, 41.482116s
	Compute time: 28.346066s (68.6%)
	Comm time:    12.312477s (29.8%)
MPI BH: 	10.736532s
	Build time:   3.788876s (35.3%)
	Compute time: 3.343336s (31.1%)
	Dealloc time: 1.834416s (17.1%)
	Comm time:    1.625227s (15.1%)

N=65536, ITERS=100
NODES=1, TASKS=64
MPI:		182.179171s
MPI Newton:	FAILS
MPI BH: 	24.606230s, 25.941518s
	Build time:   8.296424s (33.7%), 9.070618s (35.0%)
	Compute time: 8.440754s (34.3%), 8.851621s (34.1%)
	Dealloc time: 4.168720s (16.9%), 4.409707s (17.0%)
	Comm time:    3.426208s (13.9%), 3.323741s (12.8%)


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

