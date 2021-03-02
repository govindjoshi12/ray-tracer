Govind Joshi (gvj84)
Albin Shrestha (as89652)

Triangle-Ray Intersection

    In order to determine if a point has intersected with a
    triangle, we determine t based on Badouel's Algorithm which
    we learned from the following paper:

    https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.127.8440&rep=rep1&type=pdf

    We then determine the ray position at t and check that the point is to the
    left of each of the triangle's sides. If it is, then there is an intersection.

Phong Interpolation of Normals

    In order to do phong interpolation of Normals, we followed 
    the formulas learned in class. These formulas 
    were dervied from the interpolation formula for the vector 
    space R^2, but with dotting a term to each side of the Equation
    to ensure that an inverse could be found for the resulting
    matrices. Once we found barycentric coordinates, we used them
    as weights for the per-vertex normals for finding the new normal. 
    We followed the same formula for finding the new interpolated material.

Implement the Whitted illumination model

    For each light, we calculate the shadow attenuation, distance attenuation,
    diffuse, specular, ambient and emissive components and sum them for the 
    resulting color. In order to distance attenuation, we clamp the values 
    of (1 / (constantTerm + linear + quadratic) between 0 and 1. 
    
    For shadow attentuation, we shoot rays recursively each time we hit 
    an object:

    O-------->O---->O-------->Light 

    And only multiply the attenuation term kt^d when the shadow ray is leaving
    a translucent object. This is checked by checking that the dot product of 
    normal and the shadow is >= 0. Otherwise, no change is made to the vector
    and another shadow ray is shot towards the point light. 

    For the other components, we followed the in-class formulas.

    For refraction, 

Basic Anti-aliasing

    Our implementation of Anti-aliasing was a very but effective one. 
    Firstly we created a new method called traceAAPixel(i, j);.
    Then we call this method inside aaImage().
    This traceAAPixel is similar to tracePixel however the key 
    difference is that we look at the surrounding pixels from the pixel
    we are looking at and find the sorting pixels in the x and y direction.
    Like this:
        x = (i + double(a) / double(tempSamples)) / double(buffer_width);
        y = (j + double(b) / double(tempSamples)) / double(buffer_height);
    Doing this allows us to diminish the jaggedness of our images and gives 
    us a better image. 