import matplotlib.pyplot as plt
import numpy as np 
import pandas as pd
import re
from mpl_toolkits.mplot3d import Axes3D
data = pd.read_csv('final.csv')  # Use the correct delimiter
data = data.to_numpy()

# print(data.shape)
data = data.transpose()
time = (data[0])
x = (data[1])
y = (data[2])
z = (data[3])
#adjust size here! 
# plt.figure(figsize=(100,60))
ax = plt.axes(projection='3d')

ax.plot3D(x, y, z)

plt.show()
