import glob
from datetime import datetime, timedelta
from multiprocessing import Process, Lock, Pool
import pytz
import re
import matplotlib.pyplot as plt
import matplotlib.dates as md
import re
import pandas as pd
import matplotlib.pyplot as plt
from numpy import exp, pi, angle, where
import matplotlib.pyplot as plt
from matplotlib.backend_bases import NavigationToolbar2
import matplotlib.dates
import pandas as pd
import numpy as np

def read_field(fn):
    print(fn)
    df = pd.read_csv(fn, sep='\t',names=["Datetime", "Field"])
    df['Datetime'] = pd.to_datetime(df['Datetime'] , format='%Y-%m-%d-%H:%M:%S.%f')
    return df

df  = read_field('test-2021-03-22-09:00:00.txt')

dt = df["Datetime"].to_numpy()
deltas = np.diff(dt).astype('timedelta64[us]').astype(np.int32)
plt.plot(dt[1:],deltas)
plt.show()
# print(np.min(deltas))
# print(np.mean(deltas))
# print(np.max(deltas))
# print(np.std(deltas))
