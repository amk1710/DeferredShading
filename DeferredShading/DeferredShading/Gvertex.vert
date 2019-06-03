#version 410 core

//autor: André Mazal Krauss, 05/2019

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 vtColor;
layout (location = 2) in vec3 vtNormal;
layout (location = 3) in vec2 vtTexCoord;
layout (location = 4) in vec3 vtTangent;
layout (location = 5) in vec3 vtBitangent;

out vec3 fgPosition;
out vec4 fgColor;
out vec3 fgNormal;
out vec2 fgTexCoord;

out vec3 fgTangent;
out vec3 fgBitangent;

uniform int useTexture;
uniform int useBumpmap;

//matriz model-view-projection
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 v;
uniform mat4 ITmv; // a inversa transposta da model-view

//eye, em coordenadas do mundo
uniform vec3 eye;

void main()
{
	
	//multiplicação da posição do vértice pela matriz model-view-projection
	
	gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0); //???
	
	fgPosition= vec3(aPos.x, aPos.y, aPos.z);
	
	//passa adiante cor para o fragment shader
	fgColor = vec4(vtColor, 1.0f);
	fgNormal = vtNormal;
	
	if(useTexture == 1 || useBumpmap == 1)
	{
		fgTexCoord = vtTexCoord;
		fgTangent = vtTangent;
		fgBitangent = vtBitangent;
	}
	
}