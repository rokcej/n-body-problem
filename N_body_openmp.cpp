#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <omp.h>
#include <chrono>
#include "vector.h"
#include "body.h"
#include "util.h"

#define ITERS 10000000
#define DELTA_T 1.0
#define FRAMES 2000

int main(int argc, char* argv[])
{
    int N;
    Body *bodies, *bodies_new;
    read_input(&N, &bodies, &bodies_new);

    Vector* log = new Vector[N * FRAMES * 2];
    
    auto time_start = std::chrono::steady_clock::now();

    #pragma omp parallel
    {
        int p = omp_get_thread_num();
        int procs = omp_get_num_threads();

        int frame = 0;
        for (int iter = 0; iter < ITERS; ++iter)
        {

            for (int i = p; i < N; i += procs)
            {
                Vector accel_sum = Vector();
                for (int j = 0; j < N; ++j)
                {   
                    if (i != j)
                    {
                        accel_sum += bodies[i].acceleration(bodies[j]);
                    }
                }

                bodies_new[i].pos = bodies[i].pos + bodies[i].vel * DELTA_T + accel_sum * (0.5 * DELTA_T * DELTA_T);
                bodies_new[i].vel = bodies[i].vel + accel_sum * DELTA_T;
            }

            if (iter % (ITERS / FRAMES) == 0) {
                for (int i = p; i < N; i += procs) {
                    log[(i * FRAMES + frame) * 2 + 0] = Vector(bodies_new[i].pos);
                    log[(i * FRAMES + frame) * 2 + 1] = Vector(bodies_new[i].vel);
                }
                ++frame;
            }

            #pragma omp barrier

            #pragma omp master
            {
                Body* tmp = bodies_new;
                bodies_new = bodies;
                bodies = tmp;
            }

            #pragma omp barrier
        }
    }

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    write_output(N, FRAMES, log, bodies);

    printf("Required time: %lfs\n", time);
}
