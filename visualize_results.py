#!/usr/bin/python3

try:
    import matplotlib.pyplot as plt
    plt.rcParams.update({'font.size': 6})
    import numpy as np
    import glob
    import re
    import os
    import subprocess
    import sys
except Exception as e:
    print("Error while loading necessary packages:")
    print(str(e))

def readCommandLineArguments():
    amount = len(sys.argv)
    if amount < 5:
        raise Exception("Not enough input Parameters! Exiting program ...")

    path = sys.argv[1]
    type = sys.argv[2]
    video = sys.argv[3]
    skip = sys.argv[4]

    if type != "all" and type != "mass" and type != "velocity":
        raise Exception("Unkown type! Use either mass, velocity or all!")

    if video == "yes":
        video = True
    elif video == "no":
        video = False
    else:
        raise Exception("Wrong input for video choice! Choose yes or no.")

    if skip == "yes":
        skip = True
    elif skip == "no":
        skip = False
    else:
        raise Exception("Wrong input for skip choice! Choose yes or no.")

    return path, type, video, skip

def getCSVfiles(path):
    files = []
    path = os.path.join(path, "*.png")
    for file in sorted(glob.glob(path)):
        files.append(file.replace(".png", ".csv"))

    return files

def generateVideo(path, filename, unit):
    print("Preparing Video...")
    proc = subprocess.Popen(
        [
            "ffmpeg",
            "-framerate",
            "1",
            "-pattern_type",
            "glob",
            "-i",
            os.path.join(path, "*_" + unit + "_*.png"),
            "-c:v",
            "libx264",
            "-r",
            "30",
            "-pix_fmt",
            "yuv420p",
            os.path.join(path, filename),
            "-y"
        ]
    )

def visualiseMass(path, video):
    maxVals = []
    pathCSV = os.path.join(path, "*mass*.csv")
    for file in sorted(glob.glob(pathCSV)):
        data = np.genfromtxt(file, delimiter=",")
        maxVals.append(np.amax(data))
    maxVal = np.amax(maxVals)

    ratio = (np.shape(data)[0]) / np.shape(data)[1]


    for file in sorted(glob.glob(pathCSV)):
        timestep = re.findall('\d+', file.split("_")[-1])[0]

        data = np.genfromtxt(file, delimiter=",")
        plt.figure(figsize=(8,8 * ratio))
        plt.title("Timestep: {}; mean mass = {:.3f}".format(timestep, np.mean(data)))
        plt.imshow(data, vmin=0, vmax=maxVal)#, origin='lower')
        plt.savefig(file.replace(".csv", ".png"), dpi=300, bbox_inches="tight")
        plt.close()
        print("Mass -> Timestep {} done!".format(timestep))

    if video:
        generateVideo(path, "velocity.mp4", "velocity")
        print("Video generated successfully")
        print()

def visualiseVelocity(path, video, skipOld):
    pathCSV = os.path.join(path, "*velocity_*.csv")
    for file in sorted(glob.glob(pathCSV)):
        data = open(file).readlines()
        xDim = int(data[0].strip())
        yDim = int(data[1].strip())
        Y = []
        X = []
        for y in range(int(yDim / 64)):
            for x in range(xDim):
                X.append(x)
                Y.append(y)

        if skipOld:
            doneFiles = getCSVfiles(path)
        else:
            doneFiles = []

        break

    ratio = (yDim / 64) / xDim

    for file in sorted(glob.glob(pathCSV)):
        timestep = re.findall('\d+', file.split("_")[-1])[0]

        if file.replace("dat/", "") in doneFiles:
             print("Velocity -> Timestep {} already done -> Skip".format(timestep))
        else:
            data = np.genfromtxt(file, delimiter=",", skip_header=2)

            meanX = np.mean(data[0])
            meanY = np.mean(data[1])

            plt.figure(figsize=(8, 8 * ratio))
            plt.title("Timestep: {}".format(timestep))
            plt.quiver(X,Y,data[0],-data[1], scale=1, units='xy')
            plt.gca().invert_yaxis()
            plt.savefig(file.replace(".csv", ".png"), dpi=200, bbox_inches="tight")
            plt.close()
            print("Velocity -> Timestep {} done!".format(timestep))

    if video:
        generateVideo(path, "velocity.mp4", "velocity")
        print("Video generated successfully")
        print()



if __name__ == "__main__":
    try:
        path, type, video, skip = readCommandLineArguments()
    except Exception as e:
        print(str(e))
        sys.exit()

    if type == "all":
        visualiseVelocity(path, video=video, skipOld = skip)
        print("")
        visualiseMass(path, video=video)
    elif type == "mass":
        visualiseMass(path, video=video)
    else:
        visualiseVelocity(path, video=video, skipOld = skip)
