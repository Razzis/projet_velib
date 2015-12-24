# -*- coding: utf-8 -*-
"""
Éditeur de Spyder

Ce script temporaire est sauvegardé ici :
/home/hassan/.spyder2/.temp.py
"""

import csv
import matplotlib.pyplot as plt
import numpy as np

plt.clf()
cr = csv.reader(open("Recuit2.csv","rb"))
cr2 = csv.reader(open("Recuit2.csv","rb"))

row_count = sum(1 for row2 in cr2)


Iter = np.linspace(1,1,row_count-1)
T = np.linspace(1,1,row_count-1)
Cost = np.linspace(1,1,row_count-1)
Tau = np.linspace(1,1,row_count-1)
p = np.linspace(1,1,row_count-1)
i=0
for row in cr:
    #print row
    if(i!=0):
        Iter[i-1] = row[0]
        T[i-1] = row[1]
        Cost[i-1] = row[2]
        Tau[i-1] = row[3]
        p[i-1] = row[4]
    i+=1
        

plt.figure(1)
plt.subplot(221)

plt.plot(Iter,Cost)  # on utilise la fonction sinus de Numpy
plt.ylabel('Cost')
plt.xlabel("iter")


plt.subplot(222)
plt.plot(Iter,T)  # on utilise la fonction sinus de Numpy
plt.ylabel('Temperature')
plt.xlabel("iter")



plt.subplot(223)
plt.plot(Iter,Tau)  # on utilise la fonction sinus de Numpy
plt.ylabel("Taux d'acceptation")
plt.xlabel("iter")
plt.show()

plt.subplot(224)
plt.plot(Iter,p, '.')  # on utilise la fonction sinus de Numpy
plt.ylabel("probabilité d'acceptation")
plt.xlabel("iter")
plt.show()


plt.savefig("recuit.eps")