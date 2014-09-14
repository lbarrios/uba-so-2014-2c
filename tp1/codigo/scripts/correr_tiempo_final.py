import os
import random

for i in range(1000):
    command = '../simusched ../prueba.tsk 1 1 1 SchedLottery 4 %d > /dev/null 2>> tiempo-final.csv' % random.randint(0, 99999)
    os.system(command)
