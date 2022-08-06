import numpy as np
from matplotlib import pyplot as plt
import sys
from singlePlot import pointValues

# python ./src/output_data_processing ./sciezka/do/pliku

if __name__ == "__main__":
    argsNumber = len(sys.argv) - 1
    if argsNumber == 1:
        fig, a = plt.subplots(1)
        plots = [a]
    elif argsNumber == 2:
        fig, (a, b) = plt.subplots(2)
        plots = [a, b]
    elif argsNumber == 3 or argsNumber == 4:
        fig, ((a, b), (c, d)) = plt.subplots(2, 2)
        plots = [a, b, c, d]
    else:
        print("Wrong number of files.")
        sys.exit()
    
    i = 0
    while i < argsNumber:        
        [x, y, dx, dy, pathLegth, time] = pointValues(open(sys.argv[i+1], 'r'))
        dimension = x.size

        for arrow in range (0, dimension):
            plots[i].arrow(x[arrow], y[arrow], dx[arrow], dy[arrow], head_width=(np.max(x) + np.max(y))/400, head_length=(np.max(x) + np.max(y))/200)
        
        plots[i].plot(x, y)
        plots[i].scatter(x, y, color='red')
        plots[i].set_title(sys.argv[i+1].replace("data/", '') + ": dlugosc: " + pathLegth + "czas w ms: " + time)
        i += 1
    plt.setp(plots, xticks=[], yticks=[])
    plt.show()
    # plt.savefig('sefsd', dpi=3000)
    

