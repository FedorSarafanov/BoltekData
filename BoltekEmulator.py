import os, tty, termios
import time
master, slave = os.openpty()
tty.setraw(master, termios.TCSANOW)
print("Connect to:", os.ttyname(slave))

N=0
while True:
    N+=1
    try:
        if N<100:
            os.write(master,b"$+19.0\n")
            print(1)
        else:
            os.write(master,b"$+18.55\n")
            print(2)
        time.sleep(0.2)
    except:
        break

