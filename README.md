# CS-535-Project-3-Ray-Tracing
This project was completed for the CS 535: Intermediate Graphics course at the University of Kentucky. The project had students implement their own ray tracing algorithms to render a scene that fit within a predetermined set of constraints. The scene needed to illustrate at least 4 spheres of varying reflectivity and transparency.

The only file in this implementation is the "source.cpp" file. It functions by defining the needed ray and sphere objects with their appropriate parameters, like size, reflectivity, and transparency. The program then calls the object functions within the main() function to setup the scene and calculate the ray tracing results.

The result of the source.cpp render sends the image data to an external file, called "535RayTrace.ppm", that allows viewing of the rendered image. The resulting render of the current configuration can be seen below:

![535RayTrace.ppm](https://imgur.com/a/iMhn0Kb)
