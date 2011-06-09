#!/usr/bin/env python2
import random
from array import array
from sys import argv
count=int(argv[1])
a=array('d',(random.gauss(0,1) for x in xrange(count)))
f=open("gaussian.dat","wb")
a.tofile(f)
