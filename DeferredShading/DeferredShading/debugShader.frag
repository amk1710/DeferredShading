#version 410 core

//autor: André Mazal Krauss, 05/2019

//Fragment Shader

uniform int selectTexture;

//as texturas, passadas pelo geometry pass:
in vec2 fgtexCoord;
uniform sampler2D gPosition;
uniform sampler2D gNormal; //a normal do bumpmap.
uniform sampler2D gColorSpec;
uniform sampler2D gTangent;
uniform sampler2D gBitangent;

out vec4 pixelColor;

void main()
{
	
	vec3 position = vec3(texture(gPosition, fgtexCoord)); //model
	vec3 bumpNormal = vec3(texture(gNormal, fgtexCoord)); //tangent
	vec4 fgColor = texture(gColorSpec, fgtexCoord); // ...
	vec3 tangent = vec3(texture(gTangent, fgtexCoord)); // ...
	vec3 bitangent = vec3(texture(gBitangent, fgtexCoord)); // ...
	
	if(selectTexture == 1)
	{
		pixelColor = vec4(position, 1);
	}
	else if(selectTexture == 2)
	{
		pixelColor = vec4(bumpNormal, 1);
	}
	else if(selectTexture == 3)
	{
		pixelColor = fgColor;
	}
	else if(selectTexture == 4)
	{
		pixelColor = vec4(tangent, 1);
	}
	else if(selectTexture == 5)
	{
		pixelColor = vec4(bitangent, 1);
	}
	else
	{
		pixelColor = vec4(position, 1);
	}

	
}