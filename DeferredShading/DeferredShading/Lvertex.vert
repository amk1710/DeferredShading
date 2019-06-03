#version 410 core

//autor: André Mazal Krauss, 05/2019

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 vertex_texCoord;

out vec2 fgtexCoord;
uniform sampler2D gPosition;


void main()
{
	fgtexCoord = vertex_texCoord;	
	gl_Position = vec4(aPos, 1);
}