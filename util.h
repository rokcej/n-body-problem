#pragma once

#include "body.h"
#include "vector.h"
#include <fstream>
#include <iostream>

void read_input(int *N, Body **bodies, Body **bodies_new) {
	std::ifstream in_file;
    in_file.open("data/input.txt");

    in_file >> *N;

    *bodies = new Body[*N];
    *bodies_new = new Body[*N];

    for (int i = 0; i < *N; ++i)
    {
        double m, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z;
        in_file >> m >> pos_x >> pos_y >> pos_z >> vel_x >> vel_y >> vel_z;

        (*bodies)[i].m = m;
        (*bodies_new)[i].m = m;
        (*bodies)[i].pos = Vector(pos_x, pos_y, pos_z);
        (*bodies)[i].vel = Vector(vel_x, vel_y, vel_z);
    }

    in_file.close();
}

void write_output(int N, int FRAMES, Vector *log, Body *bodies) {
	std::ofstream out_file;
    out_file.open("data/output.txt");
    out_file << N << "\n" << FRAMES << "\n";
	for (int b = 0; b < N; ++b)
		out_file << bodies[b].m << "\n";
    for (int s = 0; s < FRAMES; ++s) {
        for (int b = 0; b < N; ++b) {
            int idx = (s * N + b) * 2;
            out_file << log[idx + 0].x << " " << log[idx + 0].y << " " << log[idx + 0].z << " "; // Position
            out_file << log[idx + 1].x << " " << log[idx + 1].y << " " << log[idx + 1].z << "\n"; // Velocity
        }
    }
    out_file.close();
}
