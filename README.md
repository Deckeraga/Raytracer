# Raytracer
Ray tracer written in C for a computer graphics course at the university of Minnesota. Uses phong shading and has support for textures, reflection, and refraction.

Usage
-----
The raytracer can be compiled with the included makefile.
The compiled raytracer takes one argument: a file containing the parameters on the image to be drawn
ex. ```./Raytracer <imageFile>```

Image files
-----------
Note: an example imagefile can be found in the "test Files" folder
# Required parameters
- *eye*: Three floats to specify the location of the camera in space ex ```eye 0.0 0.0 0.0```
- *updir*: Three floats to specify a vector in the upward direction of the camera
- *viewdir*: Thee floats to specify a vector in the direction the camera is facing
- *fovh*: one float representig the horizontal field of view of the camera in degrees
- *imsize*: two integers represenfing the height and width of the image in pixels
- *bkgcolor*: three floats (between 0 and 1) specifying the r/g/b values of the background color

# Optional Parameters
- *light*: takes six parameters: the frist three specify the x y and z locations of the light, the fourth specifies whether the light is a point light (0) or a directional light (1) (if the light is dierectional, the first three parameters specify the direction rather than the location). The final three parameters specify the color of the light (rgb).
ex ```light 0 0 0 0 1 1 1``` specifies a white point light at 0,0,0 
- *mtlcolor*: takes twelve arguments:
    (1 - 3): These specify the diffuse color of the object in rgb
    (4 - 6): These specify the speculat color of the object in rgb
    (7 - 9): These specify the ambient, diffuse, and specular constants (Ka, Kd, Ks)
    (10): the "shininess" factor
    (11): Opacity of the object
    (12): Index of refraction of the object
    This material color will ba applied to all subsequent objects drawn, until a new material color is specified

- *sphere*: Four arguments, the first three specify the location of the sphere, the final one specifies it's radius
- *texture*: takes one input, path to a .ppm file. This texture will ba applied to all subsequent objects until a new texture is applied
- *v*: takes three arguments specifying a location of a vertex
- *vn*: takes three arguments specifying a vector for a vertex normal
    note: All verticies and vertex normals are referred to by a nunber cooresponding to the order of their creation
- *vt*: two integers specifying a set of texture cordinates
- *f*: Specifies a triangle. Takes three arguments of the format #/#/# or #//#. The first number corresponds to the vertex number for a point on the tringale (1 for first vertex created) the second (optional) number specifies the texture coordinates for that vertex, and the third specifies the vertex normal (used in phong calculations) for that point.

For more clarity see the example image files in the 'test Files directory'


