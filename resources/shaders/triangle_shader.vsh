#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aDiffuseColor;
layout (location = 3) in float aAmbientFactor;
layout (location = 4) in float aSpecularBrightness;
layout (location = 5) in float aShininess;
layout (location = 6) in vec3 aIdColor;
layout (location = 7) in mat4 transformation;

out vec3 fragPos;
out vec3 bNormal;
out vec3 bDiffuseColor;
out float bAmbientFactor;
out float bSpecularBrightness;
out float bShininess;

uniform mat4 projectionView;

void main()
{
   fragPos = vec3(transformation * aPos);
   bNormal = mat3(transpose(inverse(transformation))) * aNormal;

   gl_Position = projectionView * vec4(fragPos, 1.0);

   bDiffuseColor = aDiffuseColor;
   bAmbientFactor = aAmbientFactor;
   bSpecularBrightness = aSpecularBrightness;
   bShininess = aShininess;
}