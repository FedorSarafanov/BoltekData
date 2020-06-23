#!/usr/bin/python3
import numpy as np
from numpy import pi,sqrt, linspace
import matplotlib.pyplot as plt
import matplotlib.dates as md
import numpy as np
import datetime as dt
import time
import sys


ax=plt.gca()
def processing(name):
	with open(name, 'r') as datafile:
		strings=datafile.read().split()

		numbers = list(map(str, strings))
		T=np.array(numbers[0::2])
		amplitude=np.array(numbers[1::2], dtype=np.int)

		l=np.min([len(T),len(amplitude)])
		T=T[0:(l-5)]
		amplitude=amplitude[0:(l-5)]
		timestamp = [time.mktime(dt.datetime.strptime(s[:-7], "%Y-%m-%d-%H:%M:%S").timetuple())+float(s[-6:])/10**6 for s in T]
		dates=[dt.datetime.fromtimestamp(ts) for ts in timestamp]



	
		# xfmt = md.DateFormatter('%Y-%m-%d %H:%M:%S')
		xfmt = md.DateFormatter('%H:%M:%S')

		plt.xticks( rotation=25 )
		ax.xaxis.set_major_formatter(xfmt)
		plt.plot(dates,amplitude,'b')
		# plt.plot(timestamp,amplitude)

if len(sys.argv)==2:
	processing(sys.argv[1])

if len(sys.argv)>2:
	for file in sys.argv[1:]:
		processing(file)
plt.grid(which='both')
plt.tight_layout()
plt.show()

