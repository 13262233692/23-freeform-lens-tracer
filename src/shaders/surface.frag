#version 450 core

in vec3 vWorldPos;
in vec3 vNormal;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uSurfaceColor;
uniform float uAlpha;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vWorldPos);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    float ambient = 0.15;
    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64.0);

    vec3 color = uSurfaceColor * (ambient + diffuse * 0.7) + vec3(1.0) * specular * 0.5;

    FragColor = vec4(color, uAlpha);
}
