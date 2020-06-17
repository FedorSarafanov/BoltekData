import numpy as np
from numpy import pi,sqrt, linspace
import matplotlib.pyplot as plt
import matplotlib.dates as md
import numpy as np
import datetime as dt
import time



def processing(name):
	with open(name, 'r') as datafile:
		strings=datafile.read().split()

		numbers = list(map(str, strings))
		T=np.array(numbers[0::2])

		timestamp = [time.mktime(dt.datetime.strptime(s[:-7], "%Y-%m-%d-%H:%M:%S").timetuple())+float(s[-6:])/10**6 for s in T]
		dates=[dt.datetime.fromtimestamp(ts) for ts in timestamp]

		amplitude=np.array(numbers[1::2], dtype=np.int)


		ax=plt.gca()
		# xfmt = md.DateFormatter('%Y-%m-%d %H:%M:%S')
		xfmt = md.DateFormatter('%H:%M:%S')

		plt.xticks( rotation=25 )
		ax.xaxis.set_major_formatter(xfmt)
		plt.plot(dates,amplitude)
		plt.tight_layout()
		# plt.plot(timestamp,amplitude)
		plt.show()
processing('2020-06-17-10:37:00.txt')
