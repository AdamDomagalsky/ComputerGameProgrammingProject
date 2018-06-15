#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 cubeViewProjectionMatrix;
uniform mat4 cubeMatrix;

out vec3 interpNormal;
out vec2 interpTexCoord;

out vec3 cubeTexCoord;

void main()
{
	gl_Position = cubeViewProjectionMatrix * vec4(vertexPosition, 1.0);
	cubeTexCoord = vertexPosition;
}
