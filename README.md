# Point-and-Line-Editor
Graphical Point and Line Editor in C++ using OpenGL
The purpose of the task is to refresh basic knowledge of coordinate geometry and familiarize oneself with OpenGL. With the program to be developed, on a tastefully chosen gray background, we can create points with mouse clicks, fit lines to the points, move the lines, and finally create new points from the intersection points of selected lines. The point size is 10 and the line thickness is 3.

The world coordinate system corresponds to the normalized device coordinate system, meaning the resulting image (viewport) corresponds to a square with corners (-1,-1) and (1,1). The image completely covers the 600x600 resolution user window. In the CPU program, 2D geometry should be handled with an external perspective, meaning it should be embedded into 3D space. This means assigning vec3 type to 2D points and vectors, with z = 1 for points and z = 0 for vectors. Then in the shader, we embed the 3D geometry into 4D with w=1.

From the user side, the program has 4 states which can be selected in order using the keys 'p', 'l', 'm', 'i':

‘p’: Point drawing, which places a red point at the cursor's location when the left mouse button is pressed.
‘l’: Line drawing, which requires clicking on two existing red points with the left mouse button.
‘m’: Line translation, which first selects the line with the left mouse button, and if successful, the line follows the cursor, i.e., the movement of the mouse button pressed, until the mouse button is released.
‘i’: Intersection point, which places a new red point at the intersection point of two selected lines (if it exists). The program prints the Cartesian coordinates of the resulting points, as well as the implicit and parametric equations of the resulting lines with printf to the console.
