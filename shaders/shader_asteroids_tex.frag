#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{         
    float ratio = 1.00 / 1.42;    
    vec3 I = normalize(Position - cameraPos);
    //vec3 R = reflect(I, normalize(Normal));
	vec3 RA = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(skybox, RA).rgb, 0.5);
}