#include <stdlib.h>
#include <stdio.h>
#include "/usr/include/openmpi-x86_64/mpi.h"
#include <math.h>
#include <string>
#include <random>
#include <chrono>
#include <vector>

#include "vector.h"
#include "body.h"
#include "util.h"

#define ITERS 10000
#define DELTA_T 100000.0
#define FRAMES 2000

void get_accel_sums(std::vector<Vector> &my_accel_sums, Body* my_bodies, Body* my_bodies_othr, int m, int offset)
{
    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < m; ++j)
        {   
            if (i != j || offset != 0)
            {
                my_accel_sums[i] += my_bodies[i].acceleration(my_bodies_othr[j]);
            }
        }
    }
}

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
    Body* my_bodies = new Body[m];
    Body* my_bodies_recv = new Body[m];
    Body* my_bodies_new = new Body[m];
    Vector* my_log = new Vector[m * FRAMES * 2];

    MPI_Scatter(bodies, m, type_body, 
				my_bodies, m, type_body, 
				0, MPI_COMM_WORLD);

    auto time_start = std::chrono::steady_clock::now();

    int frame = 0;
    for (int iter = 0; iter < ITERS; ++iter)
    {
        std::vector<Vector> my_accel_sums(m);

        for (int offset = 0; offset < procs; ++offset)
        {   
            //compute local accel
            if (offset == 0)
            {
                get_accel_sums(my_accel_sums, my_bodies, my_bodies, m, offset);
            }
            //compute accel between procs arrays
            else
            {
                MPI_Sendrecv(my_bodies, m, type_body, (myid + offset) % procs, offset,
					 my_bodies_recv, m, type_body, (myid - offset + procs) % procs, offset,
					 MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

                get_accel_sums(my_accel_sums, my_bodies, my_bodies_recv, m, offset);
            }
        }

        for (int i = 0; i < m; ++i)
        {
            my_bodies_new[i].pos = my_bodies[i].pos + my_bodies[i].vel * DELTA_T + my_accel_sums[i] * (0.5 * DELTA_T * DELTA_T);
            my_bodies_new[i].vel = my_bodies[i].vel + my_accel_sums[i] * DELTA_T;
        }

        if (iter % (ITERS / FRAMES) == 0) {
            for (int i = 0; i < m; ++i) {
                my_log[(i * FRAMES + frame) * 2 + 0] = Vector(my_bodies_new[i].pos);
                my_log[(i * FRAMES + frame) * 2 + 1] = Vector(my_bodies_new[i].vel);
            }
            ++frame;
        }

        Body* tmp = my_bodies_new;
        my_bodies_new = my_bodies;
        my_bodies = tmp;

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Gather(my_bodies, m, type_body, 
			   bodies, m, type_body, 
			   0, MPI_COMM_WORLD);

    MPI_Gather(my_log, m * FRAMES * 2, type_vector,
               log, m * FRAMES * 2, type_vector,
               0, MPI_COMM_WORLD);

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    if (myid == 0)
    {
        write_output(N, FRAMES, log, bodies);

        printf("Required time: %lfs\n", time);
    }

    MPI_Type_free(&type_vector);
    MPI_Type_free(&type_body);

    MPI_Finalize();

    return 0;
}
