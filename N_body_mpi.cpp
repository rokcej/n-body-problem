#include <stdlib.h>
#include <stdio.h>
#include "/usr/include/openmpi-x86_64/mpi.h"
#include <math.h>
#include <string>
#include <random>
#include "vector.h"
#include "body.h"
#include <chrono>
#include <vector>

#define KAPPA 6.673e-11
#define EPS 1e-14
#define ITERS 1000
#define DELTA_T 0.01
#define DEBUG false

Vector force(Body b_1, Body b_2)
{
    Vector diff = b_1.pos - b_2.pos;
    double dist = diff.length() + EPS;

    return diff * ((b_1.m * b_2.m) / (dist * dist * dist) * -KAPPA);
}

Vector acceleration(Body b_1, Body b_2)
{
    Vector diff = b_1.pos - b_2.pos;
    double dist = diff.length() + EPS;

    return diff * ((b_2.m) / (dist * dist * dist) * -KAPPA);
}

void print_states(double N_bodies, Body* bodies)
{
    for (int i = 0; i < N_bodies; ++i)
    {
        printf("Body %d\n", i);
        printf("Current position: ");
        bodies[i].pos.print_vector();
        printf("Current velocity: ");
        bodies[i].vel.print_vector();
    }
}

void get_accel_sums(std::vector<Vector> &my_accel_sums, Body* my_bodies, Body* my_bodies_othr, int m, int offset)
{
    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < m; ++j)
        {   
            if (i != j || offset != 0)
            {
                my_accel_sums[i] += acceleration(my_bodies[i], my_bodies_othr[j]);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    int         myid, procs;
   	Body*       bodies = nullptr;
    Body*       bodies_new = nullptr;

    // Init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    if (argc < 2)
    {
        return 1;
    }
    int N = std::stoi(argv[1]);

    if (N % procs != 0)
    {
        printf("Number of objects has to be divisible by the number of tasks\n");
    }
    
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
        bodies = new Body[N];
        bodies_new = new Body[N];

        std::random_device rd;
        std::mt19937 gen(165432);
        std::uniform_real_distribution<double> distrib_mass(1.0e24, 1.0e30); // from earth mass to sun mass
        std::uniform_real_distribution<double> distrib_pos(-1.5e10, 1.5e10);
        std::uniform_real_distribution<double> distrib_vel(0.0, 0.0);
        
        for (int i = 0; i < N; ++i)
        {
            bodies[i].m = distrib_mass(gen);
            bodies[i].pos = Vector(distrib_pos(gen), distrib_pos(gen), distrib_pos(gen));
            bodies[i].vel = Vector(distrib_vel(gen), distrib_vel(gen), distrib_vel(gen));
        }

        if (DEBUG && myid == 0)
        {
            print_states(N, bodies);
            fflush(stdout);
        }
    }

    int m = N / procs;
    Body* my_bodies = new Body[m];
    Body* my_bodies_recv = new Body[m];
    Body* my_bodies_new = new Body[m];

    MPI_Scatter(bodies, m, type_body, 
				my_bodies, m, type_body, 
				0, MPI_COMM_WORLD);

    auto time_start = std::chrono::steady_clock::now();

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

        Body* tmp = my_bodies_new;
        my_bodies_new = my_bodies;
        my_bodies = tmp;

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Gather(my_bodies, m, type_body, 
			   bodies, m, type_body, 
			   0, MPI_COMM_WORLD);

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    if (myid == 0)
    {
        if (DEBUG) print_states(N, bodies);
        printf("Required time: %lfs\n", time);
    }

    MPI_Type_free(&type_vector);
    MPI_Type_free(&type_body);

    MPI_Finalize();

    return 0;
}

/*
mpic++ N_body_mpi.cpp -o N_body_mpi
srun --ntasks=16 --nodes=1 --reservation=fri --mpi=pmix N_body_mpi 480
*/