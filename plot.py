import matplotlib.pyplot as plt
import sys
import numpy as np
import re


with open('log.txt', 'r') as f:
	data = f.read()

prefix = "[inf] [log]"
regular2 = r"[+-]? *(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?"
m=[]
for line in data.split('\n'):
	if line[:len(prefix)] != prefix:
		continue

	t,torque,theta,phi,dtheta,dphi,x,y = re.findall(regular2, line)
	m += [(t,torque,theta,phi,dtheta,dphi,x,y)]

m = np.array(m, dtype=float)
print(m[:,5][10])

plt.subplot(2, 2, 1)
plt.plot(m[:,0], m[:,2], label='theta')
plt.plot(m[:,0], m[:,3], label='phi')
plt.plot(m[:,0], m[:,4], label='dtheta')
plt.plot(m[:,0], m[:,5], label='dphi')
plt.grid()
plt.legend(loc=2)

plt.subplot(2, 2, 3)
plt.plot(m[:,3], m[:,5], label='dphi')
plt.grid()
plt.legend(loc=2)

plt.subplot(2, 2, 4)
plt.plot(m[:,2], m[:,4], label='dtheta')
plt.grid()
plt.legend(loc=2)


plt.show()
