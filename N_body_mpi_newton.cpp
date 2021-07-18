#include <stdlib.h>
#include <stdio.h>
#include "/usr/include/openmpi-x86_64/mpi.h"
#include <math.h>
#include <string>
#include <random>
#include <chrono>
#include <vector>

#include "octree.h"
#include "vector.h"
#include "body.h"
#include "util.h"

#define ITERS 1000
#define DELTA_T 100000.0
#define FRAMES 200

#define THETA 1.0


int main(int argc, char* argv[])
{
    int     myid, procs;
    int     N;
   	Body*   bodies = nullptr;
    Body*   bodies_new = nullptr;
    Vector* forces = nullptr;
    Vector* forces_sum = nullptr;
    Vector* log = nullptr;

    // Init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    
    MPI_Datatype	vector_input_type[1] = {MPI_DOUBLE};                
	int				vector_blocks[1] = {3};
	MPI_Aint		vector_displacement[1] = {0};

	MPI_Datatype	type_vector;    
	MPI_Type_create_struct(1, vector_blocks, vector_displacement, vector_input_type, &type_vector);
	MPI_Type_commit(&type_vector);

    MPI_Datatype	body_input_type[2] = {MPI_DOUBLE, type_vector};                
	int				body_blocks[2] = {1, 2};
	MPI_Aint		body_displacement[2] = {0, sizeof(double)};    

    MPI_Datatype	type_body;    
	MPI_Type_create_struct(2, body_blocks, body_displacement, body_input_type, &type_body);
	MPI_Type_commit(&type_body);

    if (myid == 0)
    {
        read_input(&N, &bodies, &bodies_new);

        log = new Vector[N * FRAMES * 2];

        if (N % procs != 0) {
            printf("N * (N - 1) / 2 has to be divisible by the number of tasks\n");
        }
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int m = (long long)N * (N - 1LL) / (2LL * procs); // Long long to prevent overflow
    forces = new Vector[N];
    forces_sum = new Vector[N];
    if (myid != 0) {
        bodies = new Body[N];
        bodies_new = new Body[N];
    }

    MPI_Bcast(bodies, N, type_body, 0, MPI_COMM_WORLD);

    // Instead of copying mass each iteration
    for (int i = 0; i < N; ++i)
        bodies_new[i].m = bodies[i].m;

    auto time_start = std::chrono::steady_clock::now();
    double compute_time = 0.0;
    double comm_time = 0.0;

    int frame = 0;
    for (int iter = 0; iter < ITERS; ++iter)
    {
        for (int i = 0; i < N; ++i)
            forces[i] = Vector(0.0, 0.0, 0.0);

        auto compute_start = std::chrono::steady_clock::now();

        // N * (N - 1) / 2 = index
        int i0 = myid * m;
        int a = (int)((1.0 + sqrt(1.0 + 8.0 * i0)) / 2.0);
        int b = i0 - a * (a - 1) / 2;
        for (int i = 0; i < m; ++i) {
            Vector force = bodies[a].force(bodies[b]);
            forces[a] += force;
            forces[b] -= force;

            if (++b >= a) {
                b = 0;
                ++a;
            }
        }

        auto comm_start = std::chrono::steady_clock::now();
        compute_time += std::chrono::duration<double>(comm_start - compute_start).count();

        MPI_Allreduce(forces, forces_sum, N * 3, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        comm_time += std::chrono::duration<double>(std::chrono::steady_clock::now() - comm_start).count();

        for (int i = 0; i < N; ++i) {
            Vector accel_sum = forces_sum[i] / bodies[i].m;
            bodies_new[i].pos = bodies[i].pos + bodies[i].vel * DELTA_T + accel_sum * (0.5 * DELTA_T * DELTA_T);
            bodies_new[i].vel = bodies[i].vel + accel_sum * DELTA_T;
        }

        if (myid == 0) {
            if (iter % (ITERS / FRAMES) == 0) {
                for (int i = 0; i < N; ++i) {
                    log[(i * FRAMES + frame) * 2 + 0] = Vector(bodies_new[i].pos);
                    log[(i * FRAMES + frame) * 2 + 1] = Vector(bodies_new[i].vel);
                }
                ++frame;
            }
        }

        Body* tmp = bodies_new;
        bodies_new = bodies;
        bodies = tmp;

        MPI_Barrier(MPI_COMM_WORLD);
    }

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    if (myid == 0)
    {
        write_output(N, FRAMES, log, bodies);

        printf("Required time: %lfs\n", time);
        printf("---------------\n");
        printf("Compute time: %lfs (%.1lf\%)\n", compute_time, 100.0 * compute_time / time);
        printf("Comm time:    %lfs (%.1lf\%)\n", comm_time, 100.0 * comm_time / time);
    }

    MPI_Type_free(&type_vector);
    MPI_Type_free(&type_body);

    MPI_Finalize();

    return 0;
}
