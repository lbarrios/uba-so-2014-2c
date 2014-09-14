# -*- encoding: utf-8 -*-
import csv
import matplotlib.pyplot as plt
import numpy as np

cant_procesos = 4

x = []
ys = [[] for p in range(cant_procesos)]

with open('tiempo-final.csv', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    corridas = 0
    for row in reader:
        corridas += 1
        x.append(corridas)
        
        for i in range(cant_procesos):
            if corridas > 1:
                old_sum = ys[i][-1] * (corridas-1)
            else:
                old_sum = 0
            new_avg = (old_sum + int(row[i])) / float(corridas)
            ys[i].append(new_avg)

def plot():
    for i in range(cant_procesos):
        plt.plot(x, ys[i])
    axes = plt.gca()
    plt.xlabel(u'cantidad de corridas')
    plt.ylabel(u'promedio de los ticks que tarda cada proceso en terminar')

plot()
plt.savefig('prueba-tiempo-final.png')
#plt.show(block=False)
