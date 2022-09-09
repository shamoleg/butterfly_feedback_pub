import matplotlib.pyplot as plt
import sys
import numpy as np
import re


with open('log.2.txt', 'r') as f:
	data = f.read()

def makere(_s):
	# return s.subs('[', '\[')
	f = r'([\deE\+\-\.]+)'
	s = _s.replace('[', R'\[')
	s = s.replace(']', R'\]')
	s = s.replace('(', R'\(')
	s = s.replace(')', R'\)')
	s = s.replace('%f', f)
	return s

m = []
expr = makere('[info] at %fs: t=%f,torque=%f,theta=%f,phi=%f,dtheta=%f,dphi=%f,x=%f,y=%f')
for line in data.split('\n'):
	ans = re.match(expr, line)
	if ans is None:
		continue
	_,t,torque,theta,phi,dtheta,dphi,x,y = ans.groups()
	m += [(t,torque,theta,phi,dtheta,dphi,x,y)]

m = np.array(m, dtype=float)
print m[:,1]
plt.plot(m[:,0], m[:,1]*100, label='torque')

plt.plot(m[:,0], m[:,2], label='theta')
plt.plot(m[:,0], m[:,3], label='phi')

plt.plot(m[:,0], m[:,4], label='dtheta')
plt.plot(m[:,0], m[:,5], label='dphi')

plt.grid()
plt.legend()
plt.show()
