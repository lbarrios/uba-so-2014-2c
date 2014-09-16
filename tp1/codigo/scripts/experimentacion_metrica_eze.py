# -*- encoding: utf-8 -*-
import csv
import matplotlib.pyplot as plt
import numpy as np

TIEMPO_ESPERADO = 40

x = []
y_median = []
y_min = []
y_max = []

with open('tiempo-final.csv', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        quantum = int(row[0])
        tiempos_ponderados = [int(z)-TIEMPO_ESPERADO for z in row[1:-1]]
        x.append(quantum)
        y_median.append(np.median(tiempos_ponderados))
        y_min.append(min(tiempos_ponderados))
        y_max.append(max(tiempos_ponderados))


def plot():
    plt.plot(x, y_median, label='median')
    plt.plot(x, y_min, label='min')
    plt.plot(x, y_max, label='max')
    axes = plt.gca()
    plt.xlabel(u'quantum (ticks)')
    plt.ylabel(u'tiempo residual de ejecución')
    plt.legend(loc='bottom left')


plot()
plt.savefig('plot-ejecucion.png')
#plt.show(block=False)
