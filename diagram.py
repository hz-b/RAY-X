#!/usr/bin/python3 -B

import matplotlib.pyplot as plt
import numpy as np

def plot(rayx_times, rayui_times, beamlines):
	assert(len(rayx_times) == len(rayui_times))

	N = len(rayx_times)

	ind = np.arange(N)

	rects_x = plt.bar(ind, rayx_times, 0.15, color='#ff0000', label='RAY-X')
	rects_ui = plt.bar(ind + 0.15, rayui_times, 0.15, color='#00ff00', label='RAY-UI')

	plt.xlabel('Beamlines')
	plt.xticks([r + 0.15/2 for r in range(3)], beamlines)

	plt.ylabel('time (ms)')
	plt.legend()
	plt.show()

plot([1, 3, 5], [2, 4, 6], ["A", "B", "C"])
