#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aDiffuseColor;
layout (location = 3) in float aAmbientFactor;
layout (location = 4) in float aSpecularBrightness;
layout (location = 5) in float aShininess;
layout (location = 6) in vec4 t0;
layout (location = 7) in vec4 t1;
layout (location = 8) in vec4 t2;
layout (location = 9) in vec4 t3;

out vec3 fragPos;
out vec3 bNormal;
out vec3 bDiffuseColor;
out float bAmbientFactor;
out float bSpecularBrightness;
out float bShininess;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   mat4 transformation = mat4(t0, t1, t2, t3);
   fragPos = vec3(transformation * aPos);
   bNormal = mat3(transpose(inverse(transformation))) * aNormal;

   gl_Position = projection * view * vec4(fragPos, 1.0);

   bDiffuseColor = aDiffuseColor;
   bDiffuseColor[0] = transformation[3][0];
   bDiffuseColor[1] = transformation[3][1];
   bDiffuseColor[2] = transformation[3][2];
   bAmbientFactor = aAmbientFactor;
   bSpecularBrightness = aSpecularBrightness;
   bShininess = aShininess;
}