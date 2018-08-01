import numpy as np
import matplotlib.pyplot as plt
from struct import pack
from time import time
from os import path
    
write_bin = lambda sequence, data_type, file: file.write(pack(data_type*len(sequence), *sequence))

RES = 150

def main():
    pdf_3d = np.load(path.join("src", "PDF_3D.npy"))

    #dimensions of variables
    x_pts = np.linspace(-1, 1, RES)
    y_pts = np.linspace(-1, 1, RES)

    contour_levels = [10000, 1000, 100, 10, 1]
    contour_sets = []
    for contour_set in range(len(contour_levels)):
        print("generating contour set at density " + str(contour_levels[contour_set]))
        contour_sets.append([])
        for level in range(RES):
            z_slice = pdf_3d[:,:,level]
            contour_sets[contour_set].append([])
            if (np.amax(z_slice) > contour_levels[contour_set]):
                for contour in plt.contour(x_pts, y_pts, z_slice, [contour_levels[contour_set]]).collections[0].get_segments():
                    contour_sets[contour_set][level].append(contour)


    print("saving contours to contours.bin...", end="")
    #write contours to contours.bin
    with open("contours.bin", "wb") as f:
        #firstly write number of contour sets as a 32 bit int, then write the number of levels for each contour set
        write_bin([len(contour_levels)], 'i', f)
        write_bin([RES], 'i', f)
        for contour_set in contour_sets:
            for level in contour_set:
                write_bin([len(level)], 'i', f)
                for contour in level:
                    write_bin([len(contour)], 'i', f)
                    write_bin([item for sublist in contour for item in sublist], 'f', f)
    print(" done!")

if __name__ == "__main__":
    main()
