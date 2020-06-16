import numpy as np
from numpy import pi,sqrt, linspace
import matplotlib.pyplot as plt



def processing(name):
	with open(name, 'r') as datafile:
		strings=datafile.read().split()
		del strings[0:4]
		numbers = list(map(str, strings))
		time=np.array(numbers[0::2])
		amplitude=np.array(numbers[1::2], dtype=np.int)
		print(amplitude)
		# ur=np.array(numbers[2::4])
		# ul=np.array(numbers[3::4])

processing('2020-06-16-12:27:00.txt')
