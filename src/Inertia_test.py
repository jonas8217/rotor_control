from subprocess import run, PIPE



def run_command(command_list : list[str]):
    return run(command_list, check=True, stdout=PIPE).stdout.decode()

def present_angles():
    ret = run_command(["./rotor_control","read","a"])
    a1,a2 = ret.strip().split(",")
    return float(a1),float(a2)

# add the rest of the functions
#   find a way to do intermittent info feedback (assuming the current run_command does not do so, currently assume that it waits for the programt to exit)





def main():
    pass
    
    # Two tests
    # Power test power to steady state velocity

    # Inertia test (impulse response)
    #   Elevation, different start posiitons, going down- and upwards
    #     Upwards starting from horizon (0 degrees), 15 degrees, 30 degrees, ... , 90 degrees. Downwards same positions (except 0 where it would stall)
    #   Azimuth, different elevations to get a profile of inertias dependant on elevation
    #     Test azimuth with elevation at again 0 degrees, 15 degrees, 30 degrees, ... , 90 degrees

    # Order of operations power test
    

if __name__ == "__main__":
    main()

