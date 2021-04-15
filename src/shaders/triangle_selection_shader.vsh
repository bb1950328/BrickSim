#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aDiffuseColor;
layout (location = 3) in float aAmbientFactor;
layout (location = 4) in float aSpecularBrightness;
layout (location = 5) in float aShininess;
layout (location = 6) in vec3 aIdColor;
layout (location = 7) in mat4 transformation;


out vec3 bColor;

uniform mat4 projectionView;

void main()
{
   vec3 fragPos = vec3(transformation * aPos);

   gl_Position = projectionView * vec4(fragPos, 1.0);

   bColor = aIdColor;
}