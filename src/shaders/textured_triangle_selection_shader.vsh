#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aIdColor;
layout (location = 3) in mat4 transformation;

out vec3 bColor;

uniform mat4 projectionView;

void main()
{
    gl_Position = projectionView*(transformation * vec4(aPos, 1.0));
    bColor = aIdColor;
}