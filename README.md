# Subdivision Pipeline

This repository contains an implementation of the Catmull-Clark subdivision surface algorithm.
The implementation supports infinetly-sharp and semi-sharp creases, extraordinary primitives (triangles, n-gons)
and a parser for an extended OBJ format used in [OpenSubdiv](https://graphics.pixar.com/opensubdiv/docs/intro.html)'s provided demo models.

## Examples

### Iterative subdivision of a cube
![](images/cube.png)

### Smooth, semi-sharp and infinitely-sharp edges
![](images/creases.png)

### Comparison of base mesh (front), smooth subdivision (back) and usage of creases (middle)
![](images/knives.png)

### Aliens with different subdivision levels
![](images/aliens.png)

### Aliens rendered with Direct Illumination
![](images/aliens_rendered.png)
