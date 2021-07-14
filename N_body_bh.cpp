// Serial implementation of the Barnes-Hut algorithm

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <chrono>
#include "octree.h"
#include "vector.h"
#include "body.h"
#include "util.h"

#define ITERS 10000
#define DELTA_T 100000.0
#define FRAMES 2000

#define THETA 1.0

int main(int argc, char* argv[])
{
    int N;
    Body *bodies, *bodies_new;
    read_input(&N, &bodies, &bodies_new);

    Vector* log = new Vector[N * FRAMES * 2];

    auto time_start = std::chrono::steady_clock::now();

    int frame = 0;
    for (int iter = 0; iter < ITERS; ++iter)
    {
        
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

        for (int i = 0; i < N; ++i) {
            Vector accel_sum = root->get_acceleration(&(bodies[i]), THETA);

            bodies_new[i].pos = bodies[i].pos + bodies[i].vel * DELTA_T + accel_sum * (0.5 * DELTA_T * DELTA_T);
            bodies_new[i].vel = bodies[i].vel + accel_sum * DELTA_T;
        }

        delete root;

        if (iter % (ITERS / FRAMES) == 0) {
            for (int i = 0; i < N; ++i) {
                log[(i * FRAMES + frame) * 2 + 0] = Vector(bodies_new[i].pos);
                log[(i * FRAMES + frame) * 2 + 1] = Vector(bodies_new[i].vel);
            }
            ++frame;
        }

        Body* tmp = bodies_new;
        bodies_new = bodies;
        bodies = tmp;
    }

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    write_output(N, FRAMES, log, bodies);

    printf("Required time: %lfs\n", time);
}
