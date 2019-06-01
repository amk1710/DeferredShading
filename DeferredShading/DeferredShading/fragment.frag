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

in vec4 fgPosition;
in vec4 fgColor;
in vec3 fgNormal;

//até 4 luzes
uniform int nLights;
uniform vec3 lightPositions[4];
struct lightSource
{
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

//luz ambiente
vec4 scene_ambient = vec4(0.2, 0.2, 0.2, 1.0);

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


out vec4 pixelColor;

void main()
{
	//iluminação de phong
	//vec4 usados para multiplicar pelas matrizes 4x4. depois, seu w será usado para dividir as demais componentes as serem usadas nos vec3
	vec4 lpos4;
	vec4 fgpos4;
	vec3 normal;
	vec3 lpos;
	vec3 fgpos;
	vec3 L;
	vec3 ambientColor = vec3(0.0, 0.0, 0.0);
	vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
	vec3 specularColor = vec3(0.0, 0.0, 0.0);
	int i;
	float dotp;
	vec4 eye4;
	vec3 eyeU;
	
	//calcula transformações sobre o eye. Efetivamente iguala o eyeU a vec(0,0,0). 
	//eye4 = vec4(eye, 1);
	//eye4 = v * eye4;
	//eyeU = vec3(eye4 / eye4[3]);
	eyeU = vec3(0,0,0);
	
	//usada se formos fazer bumpmapp
	vec3 tng = normalize(mat3(mv) * fgTangent); //já vem no cameraspace
	vec3 bitng = normalize(mat3(mv) * fgBitangent); //idem
	vec3 t_normal = normalize(mat3(mv) * fgNormal);
	
	mat3 TBN = (transpose(mat3(tng, bitng, t_normal)));
		
	//para cada luz,
	for(i = 0; i < nLights; i++)
	{
		float intensity = 1.0f;
		lightSource light = lightSource(
			vec4(lightPositions[i], 1.0),
			vec4(intensity, intensity, intensity, 1.0),
			vec4(intensity, intensity, intensity, 1.0)
		);
		
		//transforma o que for necessário
		lpos4 = v * light.position;
		lpos4 = lpos4 / lpos4[3]; //divide por w para obter a posição em vec3
		fgpos4 = mv * fgPosition;
		fgpos4 = fgpos4 / fgpos4[3];
			
		//normais, posições de luz, posições do fragmento e vetor L usados na fórmula
		lpos = vec3(lpos4);
		fgpos = vec3(fgpos4);
		
		//L = vetor unitário apontando do fragmento para a fonte de luz
		L = normalize(lpos - fgpos);
		
		normal = normalize( vec3( ITmv * vec4(fgNormal, 0)));
		
		//vetor unitário apontando do fragmento para a camera
		vec3 V = normalize(eyeU - fgpos);
		
		if(useBumpmap == 1)
		{
			//se eu devo usar o bumpmap
			//uso a normal vinda da textura,
			vec3 normalTex = vec3(texture(bumpmap, fgTexCoord));
			normal = expand(normalTex);

			//além disso, tenho que recalcular o eye e o vetor L para estarem no espaço da textura
			L = TBN * L;
			V = TBN * V;

			eyeU = TBN * eyeU;
					
		}
		
		dotp = dot(L, normal);
		if(dotp > 0.0)
		//se o cosseno entre o fragmento e a luz for maior que 0, ou seja, se a Luz ilumina esse fragmento:
		{
			//cálculos do modelo de Phong
			//cor difusa
			diffuseColor += vec3(mymaterial.diffuse * dotp * light.diffuse);

			vec3 R = normalize(reflect(L, normal));
			
			//cor especular
			specularColor += vec3(mymaterial.specular * pow(dot(R, V), Mshi) * light.specular);
			
		}
	}
	
	//cor ambiente
	ambientColor = vec3(scene_ambient * mymaterial.ambient);
	
	
	
	if(useTexture == 1)
	{
		//se estou usando textura, a cor final é:
		pixelColor = vec4(vec3(texture(texture_data, fgTexCoord)) * (ambientColor + diffuseColor) + specularColor, 1);
	}
	else
	{
		//cor final é a soma das três
		pixelColor = vec4(diffuseColor + ambientColor + specularColor, 1.0);
		
	}
	
}