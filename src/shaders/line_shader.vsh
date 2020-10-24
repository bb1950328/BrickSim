#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aColor;
//layout (location = 2) in mat4 transformation;
layout (location = 2) in vec4 arr2;

out vec3 bColor;

uniform mat4 projectionView;

void main()
{
   gl_Position = projectionView */* transformation **/ aPos;
   bColor = aColor;
   //bColor = vec3(transformation[0][0]*100, transformation[1][1]*-100, transformation[1][1]*-100);
   bColor = vec3(arr2);
}