# Plot computation results

import matplotlib.pyplot as plt

num_bodies = [256, 1024, 4196, 8192, 16384, 32768, 65536]


t_naive = [
	0.293479,
	1.096000,
	8.904254,
	(31.550425 + 31.589076 + 31.544488) / 3,
	118.480873,
	(46.459492 + 46.47513) / 2,
	182.179171
]
t_newton = [
	0.343271,
	0.893078,
	6.637288,
	(21.857179 + 21.946689 + 21.884060) / 3,
	80.467040,
	(41.302186 + 41.482116) / 2,
	None
]
t_bh = [
	0.310539,
	1.493272,
	5.785979,
	(16.155551 + 16.332684 + 16.073598) / 3,
	44.195155,
	10.736532,
	(24.606230 + 25.941518) / 2
]

plt.title("Comparison of computation times")
plt.grid(alpha=0.2)
plt.plot(num_bodies[:5], t_naive[:5], label="Naive", color='tab:green')
plt.plot(num_bodies[:5], t_newton[:5], label="Newton", color='tab:red')
plt.plot(num_bodies[:5], t_bh[:5], label="Barnes-Hut", color='tab:orange')
# plt.plot(num_bodies, data, label="Total", color='tab:blue', linewidth=2)
plt.ylabel("Computation time [seconds]")
plt.xlabel("Number of bodies")
plt.xscale("log", basex=2)
plt.legend()
plt.show()



s_naive = []
s_newton = []
s_bh = []
for i in range(len(num_bodies)):
	s_naive.append(1.0)
	s_newton.append(t_naive[i] / t_newton[i] if t_newton[i] is not None else None)
	s_bh.append(t_naive[i] / t_bh[i])

plt.title("Comparison of speedups")
plt.grid(alpha=0.2)
plt.plot(num_bodies, s_naive, label="Naive", color='tab:green')
plt.plot(num_bodies, s_newton, label="Newton", color='tab:red')
plt.plot(num_bodies, s_bh, label="Barnes-Hut", color='tab:orange')
plt.ylabel("Speedup")
plt.ylim(bottom=0)
plt.xlabel("Number of bodies")
plt.xscale("log", basex=2)
plt.legend()
plt.show()



p_newton_calc = [
	4.9,
	30.5,
	65.3,
	80.9,
	88.4,
	68.6,
	None
]
p_newton_comm = [
	79.4,
	62.2,
	30.3,
	15.9,
	8.1,
	29.8,
	None
]

plt.title("Newton performance analysis")
plt.grid(alpha=0.2)
plt.plot(num_bodies, p_newton_calc, label="Newton computation", color='tab:cyan')
plt.plot(num_bodies, p_newton_comm, label="Newton communication", color='tab:blue')
plt.ylabel("Percentage of total computation time")
plt.ylim(bottom=0, top=100)
plt.xlabel("Number of bodies")
plt.xscale("log", basex=2)
plt.legend()
plt.show()



p_bh_build = [
	30.3,
	27.7,
	35.2,
	42.0,
	37.2,
	35.3,
	(33.7 + 35.0) / 2
]
p_bh_calc = [
	5.7,
	14.3,
	22.3,
	23.6,
	28.4,
	31.1,
	(34.3 + 34.1) / 2
]
p_bh_dealloc = [
	15.3,
	14.9,
	19.1,
	18.9,
	17.4,
	17.1,
	(16.9 + 17.0) / 2
]
p_bh_comm = [
	36.6,
	40.1,
	21.5,
	14.7,
	16.2,
	15.1,
	(13.9 + 12.8) / 2
]

plt.title("Barnes-Hut performance analysis")
plt.grid(alpha=0.2)
plt.plot(num_bodies, p_bh_build, label="Barnes-Hut octree construction", color='tab:pink')
plt.plot(num_bodies, p_bh_dealloc, label="Barnes-Hut octree deallocation", color='tab:purple')
plt.plot(num_bodies, p_bh_calc, label="Barnes-Hut computation", color='tab:cyan')
plt.plot(num_bodies, p_bh_comm, label="Barnes-Hut communication", color='tab:blue')
plt.ylabel("Percentage of total computation time")
plt.ylim(bottom=0, top=100)
plt.xlabel("Number of bodies")
plt.xscale("log", basex=2)
plt.legend()
plt.show()

