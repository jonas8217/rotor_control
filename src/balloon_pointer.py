import sys
import numpy as np
import json

# Esrange hill (radar hill??)
lat_gs,long_gs = 67.8755167, 21.0629957
alt_gs = 462.9

def earth_R(lat: float): # input radians
    R_eq = 6378137 # radius at equator
    R_p = 6356752 # radius at the poles
    f = (R_eq - R_p) / R_eq
    return R_eq * (1 - f*np.sin(lat)**2)


def geo2vec(lat : float, long : float): # input degrees
    r = earth_R(lat)
    la = (90-lat)*np.pi/180
    lo = long*np.pi/180
    return np.array([
        r * np.sin(la) * np.cos(lo),
        r * np.sin(la) * np.sin(lo),
        r * np.cos(la)
    ])

def vec2geo(v):
    return (
        float(-np.arccos(v[2]/np.linalg.norm(v)) * 180/np.pi + 90),
        float(np.atan2(v[1],v[0]) * 180/np.pi)
    )

def vec2AzEl(v):
    res = vec2geo(v)
    return (-res[1], res[0])

def gcd_arc(geo1 : tuple[float, float], geo2 : tuple[float, float]):
    v1 = geo2vec(*geo1)
    v2 = geo2vec(*geo2)
    theta = np.arccos(np.dot(v1,v2)/(np.linalg.norm(v1)*np.linalg.norm(v2)))
    # using the average latitude of the two geographic coordinates to estimate the radius of the earth in that region. 
    # This idea is flawed if the shortest path does not cross this average latitude. Eg. when the shortest distance crosses near one of the poles.
    # A better method could involve using half the vector between the two gepgraphic points to re-project onto the sphere, thus finding the real center point between the geographic coordinates
    return theta * earth_R((geo1[0]+geo2[0])/2)


# Rotation matricies
R_x = lambda a: np.array([
    [1,           0,           0],
    [0, np.cos(a),-np.sin(a)],
    [0, np.sin(a), np.cos(a)]
])
R_y = lambda a: np.array([
    [ np.cos(a), 0, np.sin(a)],
    [           0, 1,           0],
    [-np.sin(a), 0, np.cos(a)]
])
R_z = lambda a: np.array([
    [np.cos(a),-np.sin(a), 0],
    [np.sin(a), np.cos(a), 0],
    [0,           0,           1]
])

# scew-matrix operator
# scew = lambda v: np.array([
#     [    0,-v[2], v[1]],
#     [ v[2],    0,-v[0]],
#     [-v[1], v[0],    0]
# ])

# def aa2R(axis,angle): # angle axis to rotation matrix
#     v = axis/np.linalg.norm(axis)
#     return np.eye(3) + scew(v) * np.sin(angle) + (1-np.cos(angle)) * scew(v) @ scew(v)

xyz2yzx = np.array([
    [0,0,1],
    [1,0,0],
    [0,1,0]
])

def world2gs(lat : float, long : float, az_offset: float = 0): # rotation matrix from geographic coordinate system world frame to ground station frame
    return R_z(long*np.pi/180) @ R_y(-lat*np.pi/180) @ xyz2yzx @ R_z(90*np.pi/180) @ R_z(-az_offset*np.pi/180)
    

def main(argv : list[str]):
    # load ground station config
    fname = "gs_config.json"
    with open(fname, "r") as f:
        data = json.load(f)

    lat_gs = data["latitude"]
    long_gs = data["longitude"]
    alt_gs = data["altitude"]
    az_offset = data["azimuth_offset"]

    # Get the balloon coordinates from the input arguments
    geo_b = (float(argv[1]),float(argv[2]))
    alt_b = float(argv[3])

    # Calculate the vectors  
    v_gs = geo2vec(lat_gs,long_gs)
    v_b = geo2vec(*geo_b)
 
    # Extend vectors by their altitudes
    extend = lambda vec,a : (1+a/np.linalg.norm(vec))*vec
    v_gs = extend(v_gs,alt_gs)
    v_b = extend(v_b,alt_b)

    # get the pointing vector between the ground station and the balloon
    v_point = v_b - v_gs
    v_point = v_point/np.linalg.norm(v_point)

    # get the pointing vector in the ground station frame
    gs_v_point = np.transpose(world2gs(lat_gs,long_gs,az_offset)) @ v_point


    # calculate the azimuth and elevation from the vector in the ground station frame
    az,el = vec2AzEl(gs_v_point)
    print(az,el)



if __name__ == "__main__":
    main(sys.argv)