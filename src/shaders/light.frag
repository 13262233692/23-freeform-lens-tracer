#version 450 core

uniform vec3 uLightColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(uLightColor, 1.0);
}
