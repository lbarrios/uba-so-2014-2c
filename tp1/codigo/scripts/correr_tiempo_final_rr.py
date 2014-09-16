import os
import random

os.system('rm tiempo-final.csv')

for i in range(1, 40):
    #command = '../simusched ../in.tsk 4 1 2 SchedRR %d %d %d %d 1> corrida.%d 2>> tiempo-final.csv' % (i,i,i,i,i)
    command = '../simusched ../in.tsk 1 1 2 SchedRR %d 1> corrida.%d 2>> tiempo-final.csv' % (i,i)
    print command
    os.system(command)
    os.system('echo "" >> tiempo-final.csv')
