from matplotlib import pyplot as plt
# import csv
import numpy as np
import os
import sys

file_names = ["step_response_test_test.csv"]
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
        lines.insert(1,lines[1].replace("100","0"))
        table = [[] for _ in lines ]
        for i,line in enumerate(lines):
            table[i] = list(map(float, line.split(",")))

    table = np.array(table)

    table = table[:len(table)//5]

    fig, ax1 = plt.subplots()
    plt.title("az" + file_name.split("Az")[1])
    ax1.set_xlabel("time (s)")

    ax2 = ax1.twinx()

    if abs(table[0,3] - table[-1,3]) > 0.2:
        ax1.set_ylabel("power [%]", color="b")
        ax1.plot(table[:,0],table[:,1], marker="",color="b")
        ax2.set_ylabel("angle azimuth [degrees]", color="r")
        ax2.plot(table[:,0],table[:,3], marker="",color="r")
    else:
        ax1.set_ylabel("power [%]", color="b")
        ax1.plot(table[:,0],table[:,2], marker="",color="b")
        ax2.set_ylabel("angle elevation [degrees]", color="r")
        ax2.plot(table[:,0],table[:,4], marker="",color="r")

    plt.show()