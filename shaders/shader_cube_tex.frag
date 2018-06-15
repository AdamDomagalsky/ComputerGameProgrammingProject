#version 430 core

uniform samplerCube cubeSampler;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 interpTexCoord;
in vec3 cubeTexCoord;

void main()
{
	vec3 cubeColor = texture(cubeSampler, cubeTexCoord).rgb;
	gl_FragColor = vec4(cubeColor, 1.0);
}
