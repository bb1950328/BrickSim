#version 330 core
out vec4 FragColor;

in vec3 bColor;

void main()
{
   FragColor = vec4(bColor, 1.0);
}