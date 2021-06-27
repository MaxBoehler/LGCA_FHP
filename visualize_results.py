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

    if type != "all" and type != "mass" and type != "momentum":
        raise Exception("Unkown type! Use either mass, momentum or all!")

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
            plt.colorbar()
            plt.savefig(filename + "_mass_" + str(time) + ".png", dpi=300, bbox_inches="tight")
            plt.close()
            print("Mass -> Timestep {} done!".format(time))
        except Exception:
            print("Mass -> Timestep {} ERROR! File not found".format(time))

    plt.figure(figsize=(8,4))
    plt.title("Mass conversation")
    plt.plot(timeRange, meanMASS)
    plt.xlabel("time steps / - ")
    plt.ylabel("mean mass / -")
    plt.grid()
    plt.savefig(filename + "_mass_conservation.png", dpi=300, bbox_inches="tight")
    plt.close()

    if video:
        generateVideo(path, "mass.mp4", "mass")
        print("Video generated successfully")
        print()

def visualiseMomentum(path, video):
    for file in glob.glob(os.path.join(path, "*.xml")):
        filename = file.replace(".xml", '')
    info = np.fromfile(filename + ".info", np.uint32)

    yDim = info[5]*info[1]/64
    xDim = info[4]*info[0]

    X, Y = np.meshgrid(np.arange(0, int(xDim)), np.arange(0, int(yDim)))

    ratio = yDim / xDim

    meanX = []
    meanY = []
    timeRange = []
    for time in range(0, info[2] + info[3], info[3]):
        try:
            dataVELX = concatenateData("x_momentum_density", info, filename, time)
            dataVELY = concatenateData("y_momentum_density", info, filename, time)

            meanX.append(np.mean(dataVELX))
            meanY.append(np.mean(dataVELY))
            timeRange.append(time)

            plt.figure(figsize=(8, 8 * ratio))
            plt.title("Timestep: {}".format(time))
            plt.quiver(X, Y, dataVELX, -dataVELY, scale=0.1, units='xy')
            plt.gca().invert_yaxis()
            plt.savefig(filename + "_momentum_" + str(time) + ".png", dpi=300, bbox_inches="tight")
            plt.close()
            print("Momentum density -> Timestep {} done!".format(time))
        except Exception:
            print("Momentum density -> Timestep {} ERROR! File not found".format(time))

    plt.figure(figsize=(8,4))
    plt.title("Momentum density conservation")
    plt.plot(timeRange, meanX, label="x momentum")
    plt.plot(timeRange, meanY, label="y momentum")
    plt.grid()
    plt.legend()
    plt.xlabel("time steps / -")
    plt.ylabel("mean momentum density / -")
    plt.savefig(filename + "_momentum_conservation.png", dpi=300, bbox_inches="tight")
    plt.close()


    if video:
        generateVideo(path, "momentum_density.mp4", "momentum")
        print("Video generated successfully")
        print()


def visualiseMomentumMagnitude(path, video):
    for file in glob.glob(os.path.join(path, "*.xml")):
        filename = file.replace(".xml", '')
    info = np.fromfile(filename + ".info", np.uint32)

    yDim = info[5]*info[1]/64
    xDim = info[4]*info[0]

    X, Y = np.meshgrid(np.arange(0, int(xDim)), np.arange(0, int(yDim)))

    ratio = yDim / xDim

    for time in range(0, info[2] + info[3], info[3]):
        try:
            dataVELX = concatenateData("x_momentum_density", info, filename, time)
            dataVELY = concatenateData("y_momentum_density", info, filename, time)

            magnitude = np.zeros(np.shape(dataVELX))

            for i in range(np.shape(magnitude)[0]):
                for j in range(np.shape(magnitude)[1]):
                    magnitude[i,j] = np.sqrt(dataVELX[i,j]**2 + dataVELY[i,j]**2)

            plt.figure(figsize=(8,8*ratio))
            plt.title("Timestep: {}".format(time ))
            plt.imshow(magnitude)
            plt.colorbar()
            plt.savefig(filename + "_momentumMagnitude_" + str(time) + ".png", dpi=300, bbox_inches="tight")
            plt.close()
            print("Momentum magnitue -> Timestep {} done!".format(time))
        except Exception:
            print("Momentum magnitue -> Timestep {} ERROR! File not found".format(time))


    if video:
        generateVideo(path, "momentum_magnitude.mp4", "momentumMagnitude")
        print("Video generated successfully")
        print()




if __name__ == "__main__":
    try:
        path, type, video = readCommandLineArguments()
    except Exception as e:
        print(str(e))
        sys.exit()

    if type == "momentumMagnitude":
        visualiseMomentumMagnitude(path, video=video)
    elif type == "mass":
        visualiseMass(path, video=video)
    elif type == "momentum":
        visualiseMomentum(path, video=video)
    else:
        visualiseMomentum(path, video=video)
        print("")
        visualiseMomentumMagnitude(path, video=video)
        print("")
        visualiseMass(path, video=video)
