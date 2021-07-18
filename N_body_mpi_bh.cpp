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
            printf("Number of objects has to be divisible by the number of tasks\n");
        }
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int m = N / procs;
    if (myid != 0) {
        bodies = new Body[N];
        bodies_new = new Body[N];
    }

    MPI_Bcast(bodies, N, type_body, 0, MPI_COMM_WORLD);

    // Instead of copying mass each iteration
    for (int i = 0; i < N; ++i)
        bodies_new[i].m = bodies[i].m;

    auto time_start = std::chrono::steady_clock::now();
    double build_time = 0.0;
    double compute_time = 0.0;
    double dealloc_time = 0.0;
    double comm_time = 0.0;

    int frame = 0;
    for (int iter = 0; iter < ITERS; ++iter)
    {
        auto build_start = std::chrono::steady_clock::now();

        Vector pos_min = Vector(bodies[0].pos);
        Vector pos_max = Vector(pos_min);
        for (int i = 1; i < N; ++i) {
            if (bodies[i].pos.x < pos_min.x) pos_min.x = bodies[i].pos.x;
            else if (bodies[i].pos.x > pos_max.x) pos_max.x = bodies[i].pos.x;
            if (bodies[i].pos.y < pos_min.y) pos_min.y = bodies[i].pos.y;
            else if (bodies[i].pos.y > pos_max.y) pos_max.y = bodies[i].pos.y;
            if (bodies[i].pos.z < pos_min.z) pos_min.z = bodies[i].pos.z;
            else if (bodies[i].pos.z > pos_max.z) pos_max.z = bodies[i].pos.z;
        }
        
        Octant *root = new Octant(pos_min, pos_max);
        for (int i = 0; i < N; ++i)
            root->insert(&(bodies[i]));
        root->compute_mass_distribution();

        auto compute_start = std::chrono::steady_clock::now();
        build_time += std::chrono::duration<double>(compute_start - build_start).count();

        for (int i = myid * m; i < (myid + 1) * m; ++i) {
            Vector accel_sum = root->get_acceleration(&(bodies[i]), THETA);

            bodies_new[i].pos = bodies[i].pos + bodies[i].vel * DELTA_T + accel_sum * (0.5 * DELTA_T * DELTA_T);
            bodies_new[i].vel = bodies[i].vel + accel_sum * DELTA_T;
        }

        auto dealloc_start = std::chrono::steady_clock::now();
        compute_time += std::chrono::duration<double>(dealloc_start - compute_start).count();

        delete root;

        auto comm_start = std::chrono::steady_clock::now();
        dealloc_time += std::chrono::duration<double>(comm_start - dealloc_start).count();

        MPI_Allgather(MPI_IN_PLACE, m, type_body, bodies_new, m, type_body, MPI_COMM_WORLD);

        comm_time += std::chrono::duration<double>(std::chrono::steady_clock::now() - comm_start).count();

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
        printf("Build time:   %lfs (%.1lf\%)\n", build_time, 100.0 * build_time / time);
        printf("Compute time: %lfs (%.1lf\%)\n", compute_time, 100.0 * compute_time / time);
        printf("Dealloc time: %lfs (%.1lf\%)\n", dealloc_time, 100.0 * dealloc_time / time);
        printf("Comm time:    %lfs (%.1lf\%)\n", comm_time, 100.0 * comm_time / time);
    }

    MPI_Type_free(&type_vector);
    MPI_Type_free(&type_body);

    MPI_Finalize();

    return 0;
}
