from pyorbital import orbital
import os.path
from datetime import datetime,timedelta
import numpy as np

def main(argv):

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


    # Ground station coordinates gathered from google maps
    # Altitude gotten from en-gb.topographic-map.com/map-z61h/Denmark/ and www.freemaptools.com/elevation-finder.htm
    # building height estimated by pythagoran therom
    # TODO possibly dynamically load
    latitude,longitude = 55.3675462, 10.4318222
    building_height = 10
    ground_altitude = 21 # compared to sea level

    rotor_min_el = 0
    rotor_max_el = 180 # TODO: Verify
    rotor_min_az = -360 # TODO: Test
    rotor_max_az = 360 # TODO: Test

    rotor_az_max_speed = 2 # degrees per second
    rotor_el_max_speed = 2 # degrees per second


    hours = 24

    pass_times = orb.get_next_passes(datetime.now(), hours, longitude, latitude, ground_altitude + building_height)

    min_elevation = 15

    for p in pass_times:
        rise,fall,max_el = p
        
        if orb.get_observer_look(max_el, longitude, latitude, ground_altitude + building_height)[1] < min_elevation:
            continue

        pass_time = fall-rise
        # print(pass_time.total_seconds())
        # print(dir(pass_time))
        # exit()

        point = 100 # total trajectory points

        ts,azs,els = [],[],[]

        for i in range(point):
            dt = timedelta(seconds=i * pass_time.total_seconds() / point)
            az,el = orb.get_observer_look(rise + dt, longitude, latitude, ground_altitude + building_height)
            ts.append(dt.total_seconds())
            azs.append(float(az))
            els.append(float(el))

        # print(azs)
        # print(els)

        az_dots,el_dots = [],[]
        t_prev,az_prev,el_prev = ts[0],azs[0],els[0]
        for t,az,el in zip(ts[1:], azs[1:], els[1:]):
            az_dots.append((az-az_prev)/(t-t_prev))
            el_dots.append((el-el_prev)/(t-t_prev))

        az_dots.append(az_dots[-1])
        el_dots.append(el_dots[-1])

        # print(az_dots)

        az_too_fast,el_too_fast = [],[]
        for az,el,az_dot,el_dot in zip(azs,els,az_dots,el_dots):
            if abs(az_dot) > rotor_az_max_speed:
                az_too_fast.append([az,el])
            if abs(el_dot) > rotor_el_max_speed:
                el_too_fast.append([az,el])
        
        if True:
        
            xs = np.array([azs,els])

            xs_fast_az = np.array(az_too_fast).T
            xs_fast_el = np.array(el_too_fast).T
            
            from matplotlib import pyplot as plt
            # from matplotlib.colors import ListedColormap, BoundaryNorm

            print(xs_fast_az.shape,xs_fast_el.shape)

            fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
            # ax.axis("equal")
            ax.set_rlim(bottom=90, top=0)
            ax.plot(xs[0,:]*np.pi/180, xs[1,:], marker=" ")
            # ax.scatter(0,0,0, marker="X")
            if xs_fast_az.size != 0:
                ax.plot(xs_fast_az[0,:]*np.pi/180, xs_fast_az[1,:], marker=" ")
            if xs_fast_el.size != 0:
                ax.plot(xs_fast_el[0,:]*np.pi/180, xs_fast_el[1,:], marker=" ")

            plt.show()

        ans = input("Create trajectory? [y/N]")
        if ans.lower() != "y":
            continue

        # create trajectory
        # visualize problem areas?
        #   angular velocity greater than combined axes  velocity limits 

        



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