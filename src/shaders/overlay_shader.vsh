#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aIdColor;

out vec3 bColor;

uniform int drawSelection;//todo make dedicated shader for that

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    if (drawSelection>0) {
        bColor = aIdColor;
    } else {
        bColor = aColor;
    }
}