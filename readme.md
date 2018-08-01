To view the probability density as a 3D contour graph, run contour_visualiser.exe (Windows only, requires OpenGL 2)  
To regenerate the contour information binary (contours.bin), run contour_generator.py (requires Python 3 with the matplotlib and numpy libraries)  

viewing attributes:  
-radius - this is the radius from the origin that the camera is positioned at. the camera always looks towards the origin  
-height - this is the height at which the camera is  
-line width - this is the width of the contour lines drawn  
-y rotation - the rotation of the contours about the y axis of the world coordinates (not of the contour axes)  

-the x axis is red  
-the y axis is green  
-the z axis is blue  

controls:  
-q - increase height  
-e - decrease height  
-w - reduce radius (radius cannot decrease below 0)  
-s - increase radius  
-a - rotate y clockwise  
-d - rotate y anticlockwise  
-z - move faster: any of the 6 above moment functions will be 5x faster while z is pressed  
-x - cycle between which probability density contours are shown  

-o - switch to orthographic view (movement will switch back to perspective mode)  
-p - switch to perspective view  

-1 - move the camera to height 0 and radius 3.5, looking down the z axis at the xy plane. also sets orthographic mode  
-2 - move the camera to height -3.5 and radius 0, looking down the y axis at the xz plane. also sets orthographic mode  
-3 - move the camera to height 0 and radius 3.5, looking down the x axis at the yz plane. also sets orthographic mode  
-4 - decrease line width by 1  
-5 - increase line width by 1  
