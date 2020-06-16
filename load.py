import numpy as np
from numpy import pi,sqrt, linspace
import matplotlib.pyplot as plt

plt.rc('text', usetex=True)
plt.rc('text.latex', unicode=True)
plt.rc('text.latex', preamble=r'\usepackage[utf8]{inputenc}')
plt.rc('text.latex', preamble=r'\usepackage[russian]{babel}')
plt.rc('font', family='serif')
f, (ax1, ax2) = plt.subplots(1, 2)



def processing(name,saveto, om, ax):
	plt.cla()
	# with open('r2400_0.2541.tsv', 'r') as datafile:
	# with open('r100.tsv', 'r') as datafile:
	with open(name, 'r') as datafile:
		strings=datafile.read().split()
		del strings[0:4]
		numbers = list(map(float, strings))
		nu=np.array(numbers[0::4])
		uc=np.array(numbers[1::4])
		ur=np.array(numbers[2::4])
		ul=np.array(numbers[3::4])
