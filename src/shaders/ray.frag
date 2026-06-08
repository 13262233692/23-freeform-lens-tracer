#version 450 core

in vec3 vColor;

uniform float uRayAlpha;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, uRayAlpha);
}
