Govind Joshi (gvj84)
Albin Shrestha (as89652)


Cube Mapping:

    Our cube map is a selectable skybox that allows us to look at an infinite sky in our 
    project. This means we can display a 360-degree background using 6 images.
    Implementing cube mapping was fairly simple, but we did encounter some implementation
    errors with our logic. Getting the right face, u, v values was a challenge with the
    six different conditions/faces we must consider. This wiki article helped us understand
    this concept a lot: https://en.wikipedia.org/wiki/Cube_mapping.

aaImage() and Multiple ways of anti-aliasing:

    We modified our project's antialiasing to include two more situations 
    stochastic (jittered) supersampling and antialiasing by adaptive supersampling.
    These modes can be changed by using the boolean values provided at the top
    of RayTracer.cpp. Turning one of these flags tells the program which method
    of aa to use. We got most of the information for these methods through the book
    plus some online sources. 

    traceJitteredPixel(): A method of sampling random pixels are looked at around 
    our pixel and used to make an approximation of the pixel's color.

    traceAdaptivePixel(): A method of sampling where we compare pixels around our 
    pixel and see if they are within a certain threshold of similarity to our pixel.
    If they are then we just return our pixel, else we take into account more samples
    until we get a good value for our pixel.

Acceleration Data Structure: BVH

    We implemented a BVH Tree in order to speed up our
    ray-object intersection code. 

    We are using a splitting heuristic which splits the list
    into two equally sized sets. This heuristic was learned from
    the pbr book: http://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Bounding_Boxes.html#Bounds3f

Creative Scenes:
    
    planets.ray
    We created a representation of the Solar System with
    material values which reflected the behavior of the planets at
    this distance. The sizes are to scale, but the distances are note.
    This scene should be paired with the cubemap in the galaxy folder
    in assets for the desired effect.

    