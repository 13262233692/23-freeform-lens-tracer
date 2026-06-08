#version 450 core

uniform vec3 uRayColor;
uniform float uRayAlpha;

out vec4 FragColor;

void main()
{
    FragColor = vec4(uRayColor, uRayAlpha);
}
