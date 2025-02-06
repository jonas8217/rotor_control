from pyorbital import orbital
from pyorbital import tlefile
from datetime import datetime,timedelta
import os.path
import numpy as np


tle_file=os.path.abspath(os.path.dirname(__file__) + "/../tles_files/sample_tles.txt")

# Examples
orb1 = orbital.Orbital("BRO-4",tle_file) 
orb2 = orbital.Orbital("BDSAT-2",tle_file)
orb3 = orbital.Orbital("AO-73",tle_file)
orb4 = orbital.Orbital("ISS (ZARYA)",tle_file)

orbs = [orb1,orb2,orb3,orb4]

# Ground station coordinates provided from google maps
# Altitude gotten from en-gb.topographic-map.com/map-z61h/Denmark/ and www.freemaptools.com/elevation-finder.htm
# building height estimated by pythagoran therom
latitude,longitude = 55.3675462, 10.4318222
building_height = 10
altitude = 21

for orb in orbs:
    az,el = orb.get_observer_look(datetime.now(),longitude, latitude, altitude + building_height)
    print(f"Current position of {orb.satellite_name}:")
    print("Az:", round(az,4), ", El:", round(el,4), "Currently overhead!" if el > 0 else "")
    hours = 10
    pass_times = orb.get_next_passes(datetime.now(), hours, longitude, latitude, altitude + building_height)
    next_pass_time = pass_times
    print(f"Satellite pass rise and fall datetimes for the {hours} hours:")
    for rise,fall,max_el in pass_times:
        print(f"Rise: {rise}, Fall: {fall}")
    if len(pass_times) > 0:
        print(f"Next pass in {str(pass_times[0][0] - datetime.now()).split('.')[0]}")
    print()


if False:
    ### Tests Visualize 2 orbits over time from "now"
    for orb in [orb1, orb2]:
        xs = []

        for i in range(1):
            pos,vel = orb.get_position(datetime.now()+timedelta(minutes=i*3),normalize=False)
            xs.append(pos)
    
        xs = np.array(xs)
        
        from matplotlib import pyplot as plt

        fig = plt.figure()

        ax = fig.add_subplot(projection="3d")
        ax.axis("equal")
        ax.scatter(xs[:,0],xs[:,1],xs[:,2], marker="o")
        ax.scatter(0,0,0, marker="X")

        plt.show()


