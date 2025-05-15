from matplotlib import pyplot as plt
# import csv
import numpy as np

import sys

file_names = ["step_response_test_test.csv"]
if len(sys.argv) > 1:
    file_names = sys.argv[1:]

for file_name in file_names:
    with open('test_data/' + file_name) as csvfile:
        lines = csvfile.read().strip().split("\n")
        headers = lines[0].split(",")
        lines = lines[1:]
        lines.insert(1,lines[1].replace("100","0"))
        table = [[] for _ in lines ]
        for i,line in enumerate(lines):
            table[i] = list(map(float, line.split(",")))

    table = np.array(table)

    table = table[:len(table)//5]

    fig, ax1 = plt.subplots()
    ax1.set_xlabel("time (s)")
    ax1.set_ylabel("power [%]", color="b")
    ax1.plot(table[:,0],table[:,2], marker="",color="b")

    ax2 = ax1.twinx()

    ax2.set_ylabel("angle [degrees]", color="r")
    ax2.plot(table[:,0],table[:,4], marker="",color="r")

    plt.show()