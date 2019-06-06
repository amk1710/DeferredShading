#version 410 core

//autor: André Mazal Krauss, 05/2019

//Fragment Shader

//matriz model-view-projection
uniform mat4 mvp;

//matriz model-view
uniform mat4 mv;

//matriz de view
uniform mat4 v;

//matriz de model
uniform mat4 m;

//matriz inversa transposta da model-view. Usada para as normais
uniform mat4 ITmv;

//eye, em coordenadas do mundo
uniform vec3 eye;

const int N_LIGHTS = 32;

uniform int nLights; //o numero de luzes a ser realmente usado
uniform vec3 lightPositions[N_LIGHTS];
uniform vec3 lightColors[N_LIGHTS];
vec3 lightSpecular = vec3(0.5, 0.5, 0.5);
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

//as texturas, passadas pelo geometry pass:
in vec2 fgtexCoord;
uniform sampler2D gPosition;
uniform sampler2D gNormal; //a normal do bumpmap.
uniform sampler2D gColorSpec;
uniform sampler2D gTangent;
uniform sampler2D gBitangent;


//in vec3 fgTangent;
//in vec3 fgBitangent; // ???

out vec4 pixelColor;

//luz ambiente
vec4 scene_ambient = vec4(0.2, 0.2, 0.2, 1);

void main()
{
	
	vec3 position = vec3(texture(gPosition, fgtexCoord)); //model
	vec3 bumpNormal = vec3(texture(gNormal, fgtexCoord)); //tangent
	vec4 fgColor = texture(gColorSpec, fgtexCoord); // ...
	vec3 tangent = vec3(texture(gTangent, fgtexCoord)); // ...
	vec3 bitangent = vec3(texture(gBitangent, fgtexCoord)); // ...
	
	material mymaterial = material(
	  fgColor,
	  vec4(Ks.x, Ks.y, Ks.z, 1.0),
	  vec4(Ka.x, Ka.y, Ka.z, 1.0)
	);

	//calculo a normal "default", a da superfície, usando a tangente e a bitangente
	//espaço da tangente
	vec3 defNormal = cross(tangent, bitangent);
	//defNormal = (m * vec4(defNormal,1)).xyz;

	//com a normal, tangente e bitangente, construo a matrix TBN transposta:
	tangent = normalize(mat3(m) * tangent);
	bitangent = normalize(mat3(m) * bitangent);
	defNormal = normalize(mat3(m) * defNormal);

	mat3 TBN = transpose(mat3(tangent, bitangent, defNormal));

	//calculo V em coordenadas da textura

	vec3 eyeT = TBN * eye;

	vec4 position4 = m * vec4(position, 1);
	vec3 pos3 = TBN * position4.xyz;
	
	vec3 V = normalize(eyeT - pos3);

	//cor ambiente
	vec4 ambientColor = scene_ambient * mymaterial.ambient;
	//cores difusa e especular
	vec3 diffuse = vec3(0, 0, 0);
	vec3 specularColor = vec3(0, 0, 0);

	float reductionFactor = 1;

	//cálculo da iluminação
	for(int i = 0; i < nLights; i++)
	{
		//calculo vetor L nas coordenadas de textura
		vec3 Lpos = TBN * lightPositions[i];
		vec3 L = normalize(lightPositions[i] - pos3);
		
		//uso TBN pra passar L pra coordenadas de textura
		//L = TBN * L;

		//cálculo da iluminação (Phong)
		float dotp = dot(L, bumpNormal);
		if(dotp > 0)
		{
			diffuse += lightColors[i] * dotp * mymaterial.diffuse * reductionFactor;

			vec3 R = normalize(reflect(L, bumpNormal));
			vec3 lightS = lightSpecular * lightColors[i];
			specularColor += vec3( mymaterial.specular * pow(dot(R, V), Mshi) * lightS * reductionFactor);
		}

	}

	pixelColor = ambientColor + vec4(diffuse + specularColor, 1);
	
}