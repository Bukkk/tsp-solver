import numpy as np
from matplotlib import pyplot as plt


def pointValues(file):
    points = file.readline()
    path = file.readline()
    pathLength = file.readline()
    time = file.readline()

    path = eval(path)
    path = np.array(path)

    points = eval(points)
    points = np.array(points)

    pathedPoints = np.empty_like(points)
    pathedPoints = np.append(pathedPoints, [[0, 0]], axis = 0)

    i = 0
    for position in path:
        pathedPoints[i] = points[position]
        i += 1

    diffValues = np.empty_like(pathedPoints)
    dimension = diffValues.shape[0]

    for point in range(0, dimension-1):
        diffValues[point] = (pathedPoints[point + 1] - pathedPoints[point])/2

    diffValues[dimension-1] = pathedPoints[dimension-1] - pathedPoints[0]

    x, y = pathedPoints.T
    dx, dy = diffValues.T

    return [x, y, dx, dy, pathLength, time]