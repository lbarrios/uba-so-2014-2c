# -*- encoding: utf-8 -*-
import csv
import matplotlib.pyplot as plt
import numpy as np

x = []
y_con = []
y_sin = []

with open('../ticks-con.csv', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        tick_number = int(row[0])
        accum_ticks = [int(t) for t in row[1:]]
        x.append(tick_number)
        y_con.append(np.std(accum_ticks))

with open('../ticks-sin.csv', 'r') as f:
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        tick_number = int(row[0])
        accum_ticks = [int(t) for t in row[1:]]
        #x.append(tick_number)
        y_sin.append(np.std(accum_ticks))

def plot():
    plt.plot(x, y_con, label=u'con compensation tickets', color='green', linewidth=4)
    plt.plot(x, y_sin, label=u'sin compensation tickets', color='red', linestyle='--')
    plt.legend(loc='upper left', fontsize=10)
    axes = plt.gca()
    plt.xlabel(u'número de tick')
    plt.ylabel(u'desviación estándar de los ticks asignados a cada proceso')

plot()
plt.savefig('../prueba-fairness.png')
#plt.show(block=False)
