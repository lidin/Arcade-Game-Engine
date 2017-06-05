#version 410

uniform sampler2D posTexMap;
uniform sampler2D normTexMap;
uniform sampler2D colTexMap;
uniform int showLightArea;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform float attLin;
uniform float attQuad;

layout(location = 0) out vec4 oCol;

void main()
{
  // sample data from geometry pass
  vec2 texCoords = gl_FragCoord.xy / vec2(textureSize(colTexMap, 0));
  vec3 pos       = texture(posTexMap,  texCoords).rgb;
  vec3 norm      = texture(normTexMap, texCoords).rgb;
  vec3 col       = texture(colTexMap, texCoords).rgb;
  
  // calculate distance to light source
  vec3 lightDir = lightPos-pos;
  float dist    = length(lightDir);
  
  // perform lighting calculations
  vec3 lighting = vec3(0.05*showLightArea);
  vec3 diffuse  = max(dot(norm, normalize(lightDir)), 0.0) * col * lightCol;
  float att     = 1.0/(1.0 + attLin*dist + attQuad*dist*dist);
  diffuse      *= att;
  lighting     += diffuse;
  oCol = vec4(lighting, 1.0);
}