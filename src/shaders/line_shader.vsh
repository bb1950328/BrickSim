#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 transformation;

out vec3 bColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * transformation * aPos;
   bColor = aColor;
}