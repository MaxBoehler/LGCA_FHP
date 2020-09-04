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
    if amount < 4:
        raise Exception("Not enough input Parameters! Exiting program ...")

    path = sys.argv[1]
    type = sys.argv[2]
    video = sys.argv[3]

    if type != "all" and type != "mass" and type != "velocity":
        raise Exception("Unkown type! Use either mass, velocity or all!")

    if video == "yes":
        video = True
    elif video == "no":
        video = False
    else:
        raise Exception("Wrong input for video choice! Choose yes or no.")

    return path, type, video

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

def getBinary(filename, info):
    ySize = int(info[5]/64)
    xSize = info[4]
    data = np.zeros((ySize, xSize))
    dataRAW = np.fromfile(filename, np.float64)
    for i in range(ySize):
        for j in range(xSize):
            data[i,j] = dataRAW[i*info[4] + j]

    return data

def concatenateData(type, info, filename, time):
    yData = []
    for MPIY in range(info[1]):
        xData = []
        for MPIX in range(info[0]):
            xData.append(getBinary(filename + "_{:06d}_{:03d}_{:03d}.".format(time, MPIX, MPIY) + type, info))

        dataMPIX = xData[0]
        for i in range(1, len(xData)):
            dataMPIX = np.concatenate((dataMPIX,xData[i]),axis=1)


        yData.append(dataMPIX)

    DATA = yData[0]
    for i in range(1, len(yData)):
        DATA = np.concatenate((DATA,yData[i]))

    return DATA

def visualiseMass(path, video):
    for file in glob.glob(os.path.join(path, "*.xml")):
        filename = file.replace(".xml", '')
    info = np.fromfile(filename + ".info", np.uint32)

    maxVals = []
    for file in sorted(glob.glob(os.path.join(path, "*.mass"))):
        data = getBinary(file, info)
        maxVals.append(np.amax(data))
    maxVal = np.amax(maxVals)

    yDim = info[5]*info[1]/64
    xDim = info[4]*info[0]
    ratio = (yDim) / xDim

    meanMASS = []
    timeRange = []
    for time in range(0, info[2] + info[3], info[3]):
        try:
            dataMASS = concatenateData("mass", info, filename, time)
            timeRange.append(time)
            meanMASS.append(np.mean(dataMASS))
            plt.figure(figsize=(8,8*ratio))
            plt.title("Timestep: {}; mean mass = {:.3f}".format(time, np.mean(dataMASS)))
            plt.imshow(dataMASS, vmin=0, vmax=maxVal)
            plt.savefig(filename + "_mass_" + str(time) + ".png", dpi=300, bbox_inches="tight")
            plt.close()
            print("Mass -> Timestep {} done!".format(time))
        except Exception:
            print("Mass -> Timestep {} ERROR! File not found".format(time))

    plt.figure(figsize=(8,4))
    plt.title("Mean mass over time")
    plt.plot(timeRange, meanMASS)
    plt.xlabel("time")
    plt.ylabel("mass")
    plt.savefig(filename + "_mean_mass.png", dpi=300, bbox_inches="tight")
    plt.close()

    if video:
        generateVideo(path, "velocity.mp4", "velocity")
        print("Video generated successfully")
        print()

def visualiseVelocity(path, video):
    for file in glob.glob(os.path.join(path, "*.xml")):
        filename = file.replace(".xml", '')
    info = np.fromfile(filename + ".info", np.uint32)

    yDim = info[5]*info[1]/64
    xDim = info[4]*info[0]

    X, Y = np.meshgrid(np.arange(0, int(xDim)), np.arange(0, int(yDim)))

    ratio = yDim / xDim

    for time in range(0, info[2] + info[3], info[3]):
        try:
            dataVELX = concatenateData("velocityX", info, filename, time)
            dataVELY = concatenateData("velocityY", info, filename, time)

            meanX = np.mean(dataVELX)
            meanY = np.mean(dataVELY)

            plt.figure(figsize=(8, 8 * ratio))
            plt.title("Timestep: {}".format(time))
            plt.quiver(X,Y,dataVELX,-dataVELY, scale=1, units='xy')
            plt.gca().invert_yaxis()
            plt.savefig(filename + "_velocity_" + str(time) + ".png", dpi=300, bbox_inches="tight")
            plt.close()
            print("Velocity -> Timestep {} done!".format(time))
        except Exception:
            print("Velocity -> Timestep {} ERROR! File not found".format(time))

    if video:
        generateVideo(path, "velocity.mp4", "velocity")
        print("Video generated successfully")
        print()


if __name__ == "__main__":
    try:
        path, type, video = readCommandLineArguments()
    except Exception as e:
        print(str(e))
        sys.exit()

    if type == "all":
        visualiseVelocity(path, video=video)
        print("")
        visualiseMass(path, video=video)
    elif type == "mass":
        visualiseMass(path, video=video)
    else:
        visualiseVelocity(path, video=video)
