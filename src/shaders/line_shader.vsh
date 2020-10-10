#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 transformation;

out vec3 bColor;

uniform mat4 projectionView;

void main()
{
   gl_Position = projectionView * transformation * aPos;
   bColor = aColor;
}