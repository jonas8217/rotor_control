
def main(argv):
    from pyorbital import orbital
    import os.path

    if len(argv) < 2:
        print("No sattelite name given")
        return


    tle_file = os.path.abspath(os.path.dirname(__file__) + "/../tles_files/sample_tles.txt")
    try:
        orb = orbital.Orbital(argv[1].upper(),tle_file)
    except Exception as e:
        print(e)
        return

    # from pyorbital import tlefile
    from datetime import datetime,timedelta
    import numpy as np


    # Ground station coordinates provided from google maps
    # Altitude gotten from en-gb.topographic-map.com/map-z61h/Denmark/ and www.freemaptools.com/elevation-finder.htm
    # building height estimated by pythagoran therom
    # TODO possibly dynamically load
    latitude,longitude = 55.3675462, 10.4318222
    building_height = 10
    ground_altitude = 21

    hours = 10

    pass_times = orb.get_next_passes(datetime.now(), hours, longitude, latitude, ground_altitude + building_height)

    if len(pass_times) > 0:
        rise,fall,max_el = pass_times[0]
        
        delta = fall-rise
        # print(delta.seconds)
        # print(dir(delta))
        # exit()

        points = 200 # points per second

        azs,els = [],[]

        for i in range(points):
            t = rise + timedelta(seconds=i * delta.seconds / points)
            az,el = orb.get_observer_look(t, longitude, latitude, ground_altitude + building_height)
            azs.append(az)
            els.append(el)

        # print(azs)
        # print(els)


    if True:
    
        xs = np.array([azs,els])

        # print(xs.shape)
        # exit()
        
        from matplotlib import pyplot as plt

        # fig = plt.figure()

        # ax = fig.add_subplot(projection="3d")
        fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
        # ax.axis("equal")
        ax.plot(xs[0,:],xs[1,:], marker="_")
        # ax.scatter(0,0,0, marker="X")

        plt.show()


if False:
    tle_file=os.path.abspath(os.path.dirname(__file__) + "/../tles_files/sample_tles.txt")

    # Examples
    orb2 = orbital.Orbital("BDSAT-2",tle_file)
    orb3 = orbital.Orbital("AO-73",tle_file)
    orb4 = orbital.Orbital("ISS (ZARYA)",tle_file)

    orbs = [orb1,orb2,orb3,orb4]

    # Ground station coordinates provided from google maps
    # Altitude gotten from en-gb.topographic-map.com/map-z61h/Denmark/ and www.freemaptools.com/elevation-finder.htm
    # building height estimated by pythagoran therom
    latitude,longitude = 55.3675462, 10.4318222
    building_height = 10
    ground_altitude = 21

    for orb in orbs:
        az,el = orb.get_observer_look(datetime.now(),longitude, latitude, ground_altitude + building_height)
        print(f"Current position of {orb.satellite_name}:")
        print("Az:", round(az,4), ", El:", round(el,4), "Currently overhead!" if el > 0 else "")
        hours = 10
        pass_times = orb.get_next_passes(datetime.now(), hours, longitude, latitude, ground_altitude + building_height)
        next_pass_time = pass_times
        print(f"Satellite pass rise and fall datetimes for the {hours} hours:")
        for rise,fall,max_el in pass_times:
            print(f"Rise: {rise}, Fall: {fall}")
        if len(pass_times) > 0:
            print(f"Next pass in {str(pass_times[0][0] - datetime.now()).split('.')[0]}")
        print()

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



import sys
if __name__ == "__main__":
    main(sys.argv)