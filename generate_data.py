import random
import math

num_bodies = 512

with open("data/input.txt", "w") as f:
	f.write(f"{num_bodies}\n")

	for b in range(1, num_bodies):
		m = random.uniform(1e10, 1e15)
		r = random.uniform(1e10, 1e11)
		phi = random.random() * 2 * math.pi

		x = r * math.sin(phi)
		z = r * math.cos(phi)
		y = random.uniform(-1, 1) * 1e10

		vx = random.uniform(-1, 1) * 1e2
		vy = 0.0
		vz = random.uniform(-1, 1) * 1e2

		f.write(f"{m} {x} {y} {z} {vx} {vy} {vz}\n")

	f.write(f"{1e24} {0} {0} {0} {0} {0} {0}\n")

