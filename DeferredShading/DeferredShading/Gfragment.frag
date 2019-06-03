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

in vec3 fgPosition;
in vec4 fgColor;
in vec3 fgNormal;

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
material mymaterial = material(
  fgColor,
  vec4(Ks.x, Ks.y, Ks.z, 1.0),
  vec4(Ka.x, Ka.y, Ka.z, 1.0)
);

//a textura:
in vec2 fgTexCoord;
uniform sampler2D texture_data;
uniform sampler2D bumpmap;
uniform int useTexture;
uniform int useBumpmap;


in vec3 fgTangent;
in vec3 fgBitangent;

//função auxiliar para ajuste das coordanadas do bumpmap
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2;
}


layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColorSpec;


void main()
{
	gPosition = fgPosition; // ???

	if(useBumpmap == 1)
	{
		//usando mapeamento de normais
		vec3 normalTex = vec3(texture(bumpmap, fgTexCoord)); //precisa do vec3 cast nessa linha?
		gNormal = expand(normalTex); 
	}
	else
	{
		gNormal = normalize(fgNormal);
	}

	if(useTexture == 1)
	{
		gColorSpec.rgb = vec3(texture(texture_data, fgTexCoord));
		gColorSpec.a = 1; //componente especular, alpha, depende do material?
	}
	else
	{
		gColorSpec = fgColor;
	}

	//pixelColor = vec4(fgPosition.x, fgPosition.y, fgPosition.z, 1);
	//pixelColor = vec4(0, 0, 1, 1);

	//gPosition = gNormal;
	//gPosition = vec3(gColorSpec.r, gColorSpec.g, gColorSpec.b);



	
}