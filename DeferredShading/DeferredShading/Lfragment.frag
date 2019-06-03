#version 410 core

//autor: André Mazal Krauss, 05/2019

//Fragment Shader

//matriz model-view-projection
uniform mat4 mvp;

//matriz model-view
uniform mat4 mv;

//matriz de view
uniform mat4 v;

//matriz inversa transposta da model-view. Usada para as normais
uniform mat4 ITmv;

//eye, em coordenadas do mundo
uniform vec3 eye;

struct material
{
  vec4 diffuse;
  vec4 specular;
  vec4 ambient;
};

//material usado pro fragmento
//o valor difuso está sendo dado pela interpolação da cor do vértice, 
//já Ka, Ks e Mshi são uniformes
uniform float Mshi;
uniform vec3 Ka;
uniform vec3 Ks;
//material mymaterial = material(
//  fgColor,
//  vec4(Ks.x, Ks.y, Ks.z, 1.0),
//  vec4(Ka.x, Ka.y, Ka.z, 1.0)
//);

//as texturas, passadas pelo geometry pass:
in vec2 fgtexCoord;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;

//in vec3 fgTangent;
//in vec3 fgBitangent; // ???

out vec4 pixelColor;

void main()
{
	
	pixelColor = texture(gColorSpec, fgtexCoord);
	//pixelColor = vec4(fgtexCoord.x, fgtexCoord.y, 0, 1);
	
}