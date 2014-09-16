# -*- encoding: utf-8 -*-
import csv
import matplotlib.pyplot as plt
import numpy as np

x = []
y = []

with open('../ticks.csv', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        tick_number = int(row[0])
        accum_ticks = [int(t) for t in row[1:]]
        x.append(tick_number)
        y.append(np.std(accum_ticks))

def plot():
    plt.plot(x, y)
    axes = plt.gca()
    plt.xlabel(u'número de tick')
    plt.ylabel(u'desviación estándar de los ticks asignados a cada proceso')

plot()
plt.savefig('../prueba-fairness.png')
#plt.show(block=False)
