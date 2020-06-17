import usb.core
import usb.util
import usb.legacy as usb
import time
import numpy as np
from numpy import pi,sqrt, linspace
import matplotlib.pyplot as plt
import matplotlib.dates as md
import numpy as np
import datetime as dt
import time

dev = usb.core.find(idVendor=0x0403, idProduct=0xf245)

if dev is None:
	raise ValueError('Device not found')


def usb_control_out(bRequest, wValue, wIndex):
	dev.ctrl_transfer(0x40, bRequest, wValue, wIndex, 0)

def usb_control_in(bRequest, wValue, wIndex,wLength):
	dev.ctrl_transfer(0xC0, bRequest, wValue, wIndex, wLength)

def usb_read():
	return dev.read(0x81, 1024, timeout = 3000)

dev.set_configuration()


usb_control_out(0, 0, 0)
usb_control_in (5, 0, 0, 2)
usb_control_in (10, 0, 0, 1)
usb_control_out(9, 2, 0)
usb_control_out(9, 0, 0)
usb_control_out(0, 0, 0)
usb_control_in (5, 0, 0, 2)
usb_control_out(0, 0, 0)
usb_control_out(0, 2, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(3, 0x4138, 0)
usb_control_out(4, 8, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 1, 0)
usb_control_out(0, 2, 0)

strBuf = ''

fig = plt.figure()
ax = fig.add_subplot(111)
# some X and Y data
x = np.zeros(30)

li, = ax.plot(x)

plt.ylim([-200,200])
ymax=200
# draw and show it
ax.relim() 
ax.autoscale_view(True,True,True)
fig.canvas.draw()
plt.show(block=False)

while 1:
	ret=usb_read()
	sret = ''.join([chr(x) for x in ret])
	# print(sret)
	for s in sret:
		if s == "$":
			if len(strBuf)==5:
				if (strBuf[0] in '+-'):
					print(strBuf)
					if strBuf[1:].isnumeric():
						x[0:-1]=x[1:]
						x[-1]=int(strBuf)
						if abs(x[-1])>ymax:
							ymax=abs(x[-1])
							plt.ylim([-ymax,ymax])
						li.set_ydata(x)
						fig.canvas.draw()
						plt.pause(0.00001) 
					else:
						print('AAA')
			strBuf=""
		else:
			if (s in '+-0123456789')&(len(strBuf)<5):
				strBuf+=s
	time.sleep(0.000001)
