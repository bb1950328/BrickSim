#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 transformation;

out VS_OUT {
   vec3 color;
} vs_out;

uniform mat4 projectionView;

void main()
{
   vec3 overrideColor = vec3(transformation[0][3], transformation[1][3], transformation[2][3]);
   if (overrideColor[0]>0 || overrideColor[1]>0 || overrideColor[2]>0) {
      vs_out.color = overrideColor;
      mat4 transf2 = transformation;
      transf2[0][3] = 0;
      transf2[1][3] = 0;
      transf2[2][3] = 0;
      gl_Position = projectionView * transf2 * aPos;
   } else {
      vs_out.color = aColor;
      gl_Position = projectionView * transformation * aPos;
   }
}