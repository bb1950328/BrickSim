#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 bNormal;
in vec3 bDiffuseColor;
in float bAmbientFactor;
in float bSpecularBrightness;
in float bShininess;

struct Light {
   vec3 position;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};

uniform vec3 viewPos;
uniform Light light;
uniform int drawSelection;

uniform vec3 foo;

void main()
{
   if (drawSelection>0) {
      FragColor = vec4(bDiffuseColor, 1.0);
   } else {
      // ambient
      vec3 ambient = light.ambient * bDiffuseColor*bAmbientFactor;

      // diffuse
      vec3 norm = normalize(bNormal);
      vec3 lightDir = normalize(light.position - fragPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = light.diffuse * (diff * bDiffuseColor);

      // specular
      vec3 viewDir = normalize(viewPos - fragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), bShininess);
      vec3 specular = light.specular * (spec * vec3(1.0)*bSpecularBrightness);

      vec3 result = ambient + diffuse + specular;
      FragColor = vec4(result, 1.0);
      //FragColor = vec4(foo, 1.0);
   }
}