#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec3 bNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   fragPos = vec3(model * aPos);
   bNormal = mat3(transpose(inverse(model))) * aNormal;

   gl_Position = projection * view * vec4(fragPos, 1.0);
}