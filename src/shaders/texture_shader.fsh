#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 bIdColor;

// texture sampler
uniform sampler2D texture1;

uniform int drawSelection;

void main()
{
    if (drawSelection>0) {
        FragColor = vec4(bIdColor, 1.0);
    } else {
        FragColor = texture(texture1, TexCoord);
    }
}