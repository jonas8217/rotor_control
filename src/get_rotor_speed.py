
import numpy as np
import os
import sys

file_names = []
if len(sys.argv) > 1:
    if sys.argv[1] == "all":
        file_names = sorted(os.listdir('test_data'))
    else:
        file_names = sys.argv[1:]

for file_name in file_names:
    with open('test_data/' + file_name) as csvfile:
        lines = csvfile.read().strip().split("\n")
        headers = lines[0].split(",")
        lines = lines[1:]
        table = [[] for _ in lines ]
        for i,line in enumerate(lines):
            table[i] = list(map(float, line.split(",")))
    
    table = np.array(table)

    if abs(table[0,3] - table[-1,3]) > 0.2:
        az_init = v = table[0,3]
        i = 0
        while az_init - table[i,3]:
            i += 1
        
        az_speed = (table[-1,3] - table[i,3])/(table[-1,0]-table[i,0])
        print("az_speed", az_speed)
    else:
        el_init = v = table[0,4]
        i = 0
        while el_init - table[i,4]:
            i += 1
        
        el_speed = (table[-1,4] - table[i,4])/(table[-1,0]-table[i,0])
        print("el_speed", el_speed)