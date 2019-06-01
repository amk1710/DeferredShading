#include <string>
#include <iostream>
#include <fstream>
#include <istream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <time.h>
#include <unordered_map>


#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLWindowManager.h"



using namespace std;

//one callback definition(could not put it inside the class for some reason)
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int GLWindowManager::loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);

		cout << width << "," << height << endl;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


void GLWindowManager::processInput(GLFWwindow *window)
{
	//habilita usuário a fechar a janela apertando escape
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		eye.x -= 0.01;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		eye.x += 0.01;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		scale += 0.001f;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		scale -= 0.001f;
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		angle += 0.001f;
	}

	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		angle -= 0.001f;
	}


}

std::string GLWindowManager::readShaderFile(const char* name)
{
	std::ifstream in(name);
	std::string shader;

	if (in.fail())
	{
		//aborta
		cerr << "IO error opening file on vertex.vert. Aborting" << std::endl;
		exit(1);
	}

	char a;
	while (in.get(a) && a != EOF)
	{
		shader += a;
	}
	shader += '\0';
	in.close();
	return shader;
}

	//updates model-view-projection matrix based on current values for the eye, scale etc.
void GLWindowManager::UpdateMVPMatrix()
{
	//construct model, view and projection matrices:

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	glm::mat4 View = glm::lookAt(eye, cameraTarget, up);

	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(scale)) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));


	
	view = View;
	modelView = View * Model;
	modelViewProjection = Projection * View * Model;

	//a inversa transposta da modelView
	//primeiro inverte e depois tranpõe?
	ITmodelView = glm::transpose(glm::inverse(modelView));
}

GLWindowManager::GLWindowManager()
{
	IsTextureLoaded = false;
	IsBumpmapLoaded = false;
	data = NULL;
	bump_data = NULL;
	
}

GLWindowManager::~GLWindowManager()
{
	//libera memória, se != null
	if (data != NULL)
	{
		stbi_image_free(data);
	}

	if (bump_data != NULL)
	{
		stbi_image_free(bump_data);
	}
	
}

//reference: https://vulkan-tutorial.com/Loading_models
void GLWindowManager::LoadModel(const char* objName, bool randomColors = false)
{
	std::string warn, err;
	
	srand(17);	
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objName)) {
		cout << warn + err << endl;
		throw std::runtime_error(warn + err);
	}

	// o tiny obj loader já garante que todas as faces são triângulos. Dado isso, para obter os vértices, 
	// somente precisamos iterar sobre os vertices e botá-los no vetor
	//assumimos que os vértices são únicos
	//supomos que só tem uma mesh definida no arquivo. Se tiver mais de uma, renderizamos só a primeira
	indices.clear();
	for (const auto& index : shapes[0].mesh.indices)
	{
		indices.push_back(index.vertex_index);
	}

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<int> number_of_faces;

	tangents.resize(attrib.vertices.size() / 3);
	bitangents.resize(attrib.vertices.size() / 3);
	number_of_faces.resize(attrib.vertices.size() / 3);

	cout << shapes[0].mesh.indices.size() << endl;


	//calcula, para cada triângulo, a tangente e a bitangente
	//ps: += tres para ir percorrendo a cada três vértices, ou seja, um triângulo. É certo que vértices serão percorridos mais de uma vez, porém conto com o resultado não ser discrepante o suficiente para impactar
	for (int i = 0; i < shapes[0].mesh.indices.size(); i += 3)
	{
		//adaptado de: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/

		//cout << "triangle:" << i << endl;

		// Shortcuts for vertices
		glm::vec3 v0 = glm::vec3(attrib.vertices[indices[i]], attrib.vertices[indices[i] + 1], attrib.vertices[indices[i] + 2]);
		glm::vec3 v1 = glm::vec3(attrib.vertices[indices[i + 1]], attrib.vertices[indices[i + 1] + 1], attrib.vertices[indices[i + 1] + 2]);
		glm::vec3 v2 = glm::vec3(attrib.vertices[indices[i + 2]], attrib.vertices[indices[i + 2] + 1], attrib.vertices[indices[i + 2] + 2]);

		/*cout << "v1:" << v0.x << v0.y << v0.z << endl;
		cout << "v2:" << v1.x << v1.y << v1.z << endl;
		cout << "v3:" << v2.x << v2.y << v2.z << endl;
		*/

		// Shortcuts for UVs
		glm::vec2 uv0 = glm::vec2(attrib.texcoords[indices[i]], attrib.texcoords[indices[i] + 1]);
		glm::vec2 uv1 = glm::vec2(attrib.texcoords[indices[i + 1]], attrib.texcoords[indices[i + 1] + 1]);
		glm::vec2 uv2 = glm::vec2(attrib.texcoords[indices[i + 2]], attrib.texcoords[indices[i + 2] + 1]);

		//edges of the triangle: position delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
		

		//glm::vec3 normal = glm::vec3(attrib.normals[indices[i]])
		////glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
		//glm::vec3 bitangent = glm::cross(tangent, );

		
		//adiciona valores para as médias de cada vertice
		number_of_faces[indices[i]] += 1;
		number_of_faces[indices[i+1]] += 1;
		number_of_faces[indices[i+2]] += 1;

		tangents[indices[i]] += tangent;
		tangents[indices[i + 1]] += tangent;
		tangents[indices[i + 2]] += tangent;

		bitangents[indices[i]] += bitangent;
		bitangents[indices[i + 1]] += bitangent;
		bitangents[indices[i + 2]] += bitangent;

	}

	//para cada tangente e bitangente, tira a media e normaliza
	for (int i = 0; i < number_of_faces.size(); i++)
	{
		//cout << "vertex: " << i << ", number of faces: " << number_of_faces[i] << endl;

		tangents[i] = glm::vec3(tangents[i].x / number_of_faces[i], tangents[i].y / number_of_faces[i], tangents[i].z / number_of_faces[i]);
		bitangents[i] = glm::vec3(bitangents[i].x / number_of_faces[i], bitangents[i].y / number_of_faces[i], bitangents[i].z / number_of_faces[i]);

		glm::normalize(tangents[i]);
		glm::normalize(bitangents[i]);

	}
	
	//preenche vbo:
	vertices.clear();
	for (int i = 0; i < attrib.vertices.size() / 3; i++)
	{
		vertices.push_back(attrib.vertices[3*i]);
		vertices.push_back(attrib.vertices[3*i+1]);
		vertices.push_back(attrib.vertices[3*i+2]);

		//colors
		if (randomColors)
		{
			vertices.push_back((static_cast <float> (rand() / 2.0) / static_cast <float> (RAND_MAX)));
			vertices.push_back((static_cast <float> (rand() / 2.0) / static_cast <float> (RAND_MAX)));
			vertices.push_back((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
		}
		else
		{
			vertices.push_back(attrib.colors[3 * i]);
			vertices.push_back(attrib.colors[3 * i + 1]);
			vertices.push_back(attrib.colors[3 * i + 2]);
		}
		
		//normals
		vertices.push_back(attrib.normals[3 * i]);
		vertices.push_back(attrib.normals[3 * i + 1]);
		vertices.push_back(attrib.normals[3 * i + 2]);

		/*glm::vec3 normal = glm::cross(tangents[i], bitangents[i]);
		vertices.push_back(normal.x);
		vertices.push_back(normal.y);
		vertices.push_back(normal.z);
*/

		//texture coordinates
		if (attrib.texcoords.size() != 0)
		{
			vertices.push_back(attrib.texcoords[2 * i]);
			vertices.push_back(attrib.texcoords[2 * i + 1]);
		}
		else
		{
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
		}

		// bi/tangents, pre calculadas acima
		vertices.push_back(tangents[i].x);
		vertices.push_back(tangents[i].y);
		vertices.push_back(tangents[i].z);

		vertices.push_back(bitangents[i].x);
		vertices.push_back(bitangents[i].y);
		vertices.push_back(bitangents[i].z);



	}
	
	
	//tem mais coordenadas de textura do que vértices!

	printf("# of vertices infos  = %d\n", (int)(vertices.size()));
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n", (int)shapes.size());

}


//pra usar a unordered map abaixo, precisamos implementar o == e uma função hash:
struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	bool operator==(const Vertex& other) const {
		return position == other.position && color == other.color && texcoord == other.texcoord; //não exigimos que as normais, tangentes e bitangentes sejam iguais
	}

};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return	((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texcoord) << 1);
		}
	};
}




void GLWindowManager::LoadModel2(const char* objName, bool randomColors = false)
{
	std::string warn, err;

	srand(17);

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objName)) {
		cout << warn + err << endl;
		throw std::runtime_error(warn + err);
	}

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<int> number_of_faces;

	std::unordered_map<Vertex, uint32_t> unique_vertices = {};
	std::vector<Vertex> aux_vertices = {};
	indices.clear();
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			//o tinyobj loader garante que são todos triangulos
			glm::vec3 pos = glm::vec3(attrib.vertices[index.vertex_index * 3], attrib.vertices[index.vertex_index * 3 + 1], attrib.vertices[index.vertex_index * 3 + 2]);
			glm::vec3 color = glm::vec3(attrib.colors[index.vertex_index*3], attrib.colors[index.vertex_index*3 + 1], attrib.colors[index.vertex_index*3 + 2]);
			glm::vec3 normal = glm::vec3(attrib.normals[index.normal_index*3], attrib.normals[index.normal_index*3 + 1], attrib.normals[index.normal_index*3 + 2]);
			glm::vec2 texcoord = glm::vec2(attrib.texcoords[index.texcoord_index*2], attrib.texcoords[index.texcoord_index*2 + 1]);
			Vertex vertex = {
				//pos, color, normal, texcoord, tangents[index.vertex_index], bitangents[index.vertex_index]  //tangente e bitangente foram calculados acima
				pos, color, normal, texcoord, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)   //tangente e bitangente são calculados abaixo
			};

			if (unique_vertices.count(vertex) == 0)
			{
				//se não há nenhum vértice como este até agora, o adiciona a lista de vertices únicos
				unique_vertices[vertex] = static_cast<uint32_t>(aux_vertices.size());
				aux_vertices.push_back(vertex);
			}

			indices.push_back(unique_vertices[vertex]);
		}
	}


	number_of_faces.resize(aux_vertices.size());
	//calcula, para cada triângulo, a tangente e a bitangente
	//ps: += tres para ir percorrendo a cada três vértices, ou seja, um triângulo. É certo que vértices serão percorridos mais de uma vez, porém conto com o resultado não ser discrepante o suficiente para impactar
	for (int i = 0; i < indices.size(); i += 3)
	{
		//adaptado de: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/

		//cout << "triangle:" << i << endl;
		// Shortcuts for vertices
		Vertex v0 = aux_vertices[indices[i + 0]];
		Vertex v1 = aux_vertices[indices[i + 1]];
		Vertex v2 = aux_vertices[indices[i + 2]];

		//edges of the triangle: position delta
		glm::vec3 deltaPos1 = v1.position - v0.position;
		glm::vec3 deltaPos2 = v2.position - v0.position;

		// UV delta
		glm::vec2 deltaUV1 = v1.texcoord - v0.texcoord;
		glm::vec2 deltaUV2 = v2.texcoord - v0.texcoord;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		//adiciona valores para as médias de cada vertice
		number_of_faces[indices[i]] += 1;
		number_of_faces[indices[i + 1]] += 1;
		number_of_faces[indices[i + 2]] += 1;

		aux_vertices[indices[i]].tangent += tangent;
		aux_vertices[indices[i + 1]].tangent += tangent;
		aux_vertices[indices[i + 2]].tangent += tangent;

		aux_vertices[indices[i]].bitangent += bitangent;
		aux_vertices[indices[i + 1]].bitangent += bitangent;
		aux_vertices[indices[i + 2]].bitangent += bitangent;

	}

	//para cada tangente e bitangente, tira a media e normaliza
	for (int i = 0; i < aux_vertices.size(); i++)
	{
		//cout << "vertex: " << i << ", number of faces: " << number_of_faces[i] << endl;

		aux_vertices[i].tangent = glm::vec3(aux_vertices[i].tangent.x / number_of_faces[i], aux_vertices[i].tangent.y / number_of_faces[i], aux_vertices[i].tangent.z / number_of_faces[i]);
		aux_vertices[i].bitangent = glm::vec3(aux_vertices[i].bitangent.x / number_of_faces[i], aux_vertices[i].bitangent.y / number_of_faces[i], aux_vertices[i].bitangent.z / number_of_faces[i]);

		glm::normalize(aux_vertices[i].tangent);
		glm::normalize(aux_vertices[i].bitangent);

	}


	//simply dump aux_vertices into vertex buffer:
	//preenche vbo:
	vertices.clear();
	for (int i = 0; i < aux_vertices.size(); i++)
	{
		vertices.push_back(aux_vertices[i].position.x); 
		vertices.push_back(aux_vertices[i].position.y); 
		vertices.push_back(aux_vertices[i].position.z);
		
		//colors
		if (randomColors)
		{
			vertices.push_back((static_cast <float> (rand() / 2.0) / static_cast <float> (RAND_MAX)));
			vertices.push_back((static_cast <float> (rand() / 2.0) / static_cast <float> (RAND_MAX)));
			vertices.push_back((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
		}
		else
		{
			vertices.push_back(aux_vertices[i].color.x);
			vertices.push_back(aux_vertices[i].color.y);
			vertices.push_back(aux_vertices[i].color.z);
		}
		//normals
		vertices.push_back(aux_vertices[i].normal.x);
		vertices.push_back(aux_vertices[i].normal.y);
		vertices.push_back(aux_vertices[i].normal.z);

		//texcoords
		vertices.push_back(aux_vertices[i].texcoord.x);
		vertices.push_back(aux_vertices[i].texcoord.y);


		// bi/tangents, pre calculadas acima
		vertices.push_back(aux_vertices[i].tangent.x);
		vertices.push_back(aux_vertices[i].tangent.y);
		vertices.push_back(aux_vertices[i].tangent.z);

		vertices.push_back(aux_vertices[i].bitangent.x);
		vertices.push_back(aux_vertices[i].bitangent.y);
		vertices.push_back(aux_vertices[i].bitangent.z);

	}


	//tem mais coordenadas de textura do que vértices!

	printf("# of vertices infos  = %d\n", (int)(vertices.size()));
	printf("# of vertices in vector aux  = %d\n", (int)(aux_vertices.size()));
	printf("# of triangles  = %d\n", (int)(indices.size()) / 3);
	//printf("# of triangles v2 = %d\n", (int)(tangents.size()));
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n", (int)shapes.size());


}

void GLWindowManager::LoadTexture(const char* filepath)
{
	data = stbi_load(filepath, &width, &height, &nrChannels, 0);
	if (data)
	{
		IsTextureLoaded = true;

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	
}

void GLWindowManager::LoadBumpmap(const char* filepath)
{
	bump_data = stbi_load(filepath, &width_bm, &height_bm, &nrChannels_bm, 0);
	if (bump_data)
	{
		IsBumpmapLoaded = true;
		
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

}


void GLWindowManager::InitializeSceneInfo()
{
	// reference: https://learnopengl.com/Getting-started/Hello-Window, https://learnopengl.com/Getting-started/Hello-Triangle

	//open model
	//LoadModel("stones/stones.obj", false);


	//default values for eye, camera target and up. 
	scale = 1.0f;
	angle = 0.0f;
	eye = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);	

	//values for up to 4 lights
	nLights = 1;
	lights = { 0,3,4,    0,0,1,   0.5f,0.5f,1.5f};

	//initialize opengl
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create glfw window
	window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);

	//ask GLAD to get opengl function pointers for us
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(1);
	}

	//set viewport's size
	glViewport(0, 0, 800, 600);
	//set window's resize callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	////cria objeto VAO para guardar o que vai ser configurado abaixo:
	glGenVertexArrays(1, &VAO);
	//bind VAO
	glBindVertexArray(VAO);

	//enables depth testing
	glEnable(GL_DEPTH_TEST);

	//cria buffer object:
	//vertex buffer object: 
	//(por si só é usado para desenhar triângulos especificando somente vértices consecutivos, o que pode causar vértices especificados múltiplas vezes)
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//aqui estão todas as infos: de posição do vértice, cor e normais. Depois defino lá em baixo como é a navegação por essas infos (glVertexAttribPointer)
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), static_cast<void*>(vertices.data()), GL_STATIC_DRAW);


	//element buffer object:
	//nesse caso, usamos um element buffer object pra poder informar a ordem de desenhar os triângulos, e não os vértices múltiplas vezes
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), static_cast<void*>(indices.data()), GL_STATIC_DRAW);


	//COMO PASSAR DUAS TEXTURAS AO SHADER?


	////textura de imagem:
	////reference: https://learnopengl.com/Getting-started/Textures
	//unsigned int textures[2];
	//glGenTextures(2, textures);
	//glBindTexture(GL_TEXTURE_2D, textures[0]);

	////seta parametros para wrap, filtro de minificação e magnização
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // opções: GL_NEAREST, GL_LINEAR
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // opções: GL_NEAREST, GL_LINEAR , GL_NEAREST_MIPMAP_NEAREST , GL_LINEAR_MIPMAP_LINEAR

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//glGenerateMipmap(GL_TEXTURE_2D);

	////frees the image, now that we've used it:
	////stbi_image_free(data);
	////data = NULL;
	//
	////textura de bumpmap
	//glBindTexture(GL_TEXTURE_2D, textures[1]);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // opções: GL_NEAREST, GL_LINEAR
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // opções: GL_NEAREST, GL_LINEAR , GL_NEAREST_MIPMAP_NEAREST , GL_LINEAR_MIPMAP_LINEAR

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_bm, height_bm, 0, GL_RGB, GL_UNSIGNED_BYTE, bump_data);
	////glGenerateMipmap(GL_TEXTURE_2D);

	//VERTEX SHADER:
	//lê file do vertex shader e o compila dinamicamente
	std::string shaderString = readShaderFile("vertex.vert");

	//cria objeto shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//attach shader source code to the shader object and compile the shader
	const char *c_str = shaderString.c_str(); // glShaderSource espera no 3o param um array de c-like strings. Nesse caso, só temos uma string nesse array. No 2o param está explícita esta informação
	glShaderSource(vertexShader, 1, &c_str, NULL);
	glCompileShader(vertexShader);

	//checks for errors:
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(1);
	}

	//FRAGMENT_SHADER
	//lê file do vertex shader e o compila dinamicamente
	shaderString = readShaderFile("fragment.frag");

	//cria objeto shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//attach shader source code to the shader object and compile the shader
	const char *c_str2 = shaderString.c_str(); // glShaderSource espera no 3o param um array de c-like strings. Nesse caso, só temos uma string nesse array. No 2o param está explícita esta informação
	glShaderSource(fragmentShader, 1, &c_str2, NULL);
	glCompileShader(fragmentShader);

	//checks for errors:
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(1);
	}

	glActiveTexture(GL_TEXTURE0);
	tex1 = loadTexture("stones/stones.jpg");
	//tex1 = loadTexture("golfball/golfball.jpg");
	glActiveTexture(GL_TEXTURE1);
	tex2 = loadTexture("stones/stones_norm.jpg");
	//tex2 = loadTexture("golfball/golfball.jpg");
	IsTextureLoaded = true;
	IsBumpmapLoaded = true;

	// SHADER PROGRAM
	shaderProgram = glCreateProgram();
	//attach shader to program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//check for linking errors:
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::LINKING_ERROR\n" << infoLog << std::endl;
	}
	glUseProgram(shaderProgram);

	//deleta os objetos shader, que já cumpriram sua função
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//o buffer é composto de: 
	//coordenadas(3) + cores(3) + normais(3) + coord_texturas(2) + tangentes(3) + bitangente(3)

	////inform openGL about our vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//inform openGL about our color attributes
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	//inform openGL about our normal attributes
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//inform openGL about our texture coordinate attributes
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	//inform openGL about our tangent attributes
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	//inform openGL about our bi-tangent attributes
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 17 * sizeof(float), (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);


	//unbind VAO
	glBindVertexArray(0);

	cout << shaderProgram << endl;
}

// reference: https://learnopengl.com/Getting-started/Hello-Window, https://learnopengl.com/Getting-started/Hello-Triangle
	
//will start the render loop and finish executing only when the window is closed
void GLWindowManager::StartRenderLoop()
{
	//the render loop
	cout << shaderProgram << endl;
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		//clear pixels
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clears color and depth testing buffers

		glUseProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2);


		glBindVertexArray(VAO); //bind

		//atualiza matriz model-view-projection
		UpdateMVPMatrix();
		//passa as matrizes mvp pra placa
		int mvpParam = glGetUniformLocation(shaderProgram, "mvp");
		glUniformMatrix4fv(mvpParam, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
		int mvParam = glGetUniformLocation(shaderProgram, "mv");
		glUniformMatrix4fv(mvParam, 1, GL_FALSE, glm::value_ptr(modelView));
		int vParam = glGetUniformLocation(shaderProgram, "v");
		glUniformMatrix4fv(vParam, 1, GL_FALSE, glm::value_ptr(view));
		
		//é uma mat3?
		int ITmvParam = glGetUniformLocation(shaderProgram, "ITmv");
		glUniformMatrix4fv(ITmvParam, 1, GL_FALSE, glm::value_ptr(ITmodelView));

		//passa eye, light positions e nLights
		int eyeParam = glGetUniformLocation(shaderProgram, "eye");
		glUniform3f(eyeParam, eye.x, eye.y, eye.z);

		int nLightsParam = glGetUniformLocation(shaderProgram, "nLights");
		glUniform1i(nLightsParam, nLights);

		int lightParam = glGetUniformLocation(shaderProgram, "lightPositions");
		glUniform3fv(lightParam, 4, &lights[0]);

		//passa texturas:


		//se textura foi setada, usa textura
		if (IsTextureLoaded)
		{
			//informa pro shader que ele deve usar textura
			int boolParam = glGetUniformLocation(shaderProgram, "useTexture");
			glUniform1i(boolParam, 1);
		}

		if (IsBumpmapLoaded)
		{
			//informa pro shader que ele deve usar bumpmap
			int boolParam = glGetUniformLocation(shaderProgram, "useBumpmap");
			glUniform1i(boolParam, 1);
		}

		glUniform1i(glGetUniformLocation(shaderProgram, "texture_data"), 0);
		glUniform1i(glGetUniformLocation(shaderProgram, "bumpmap"), 1);

		//glUniform3f(glGetUniformLocation(shaderProgram, "Ka"), materials[0].ambient[0], materials[0].ambient[1], materials[0].ambient[2]);
		//glUniform3f(glGetUniformLocation(shaderProgram, "Ks"), materials[0].specular[0], materials[0].specular[1], materials[0].specular[2]);
		//glUniform1f(glGetUniformLocation(shaderProgram, "Mshi"), 128.0f);

		glUniform3f(glGetUniformLocation(shaderProgram, "Ka"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "Ks"), 0.3f, 0.3f, 0.3f);
		glUniform1f(glGetUniformLocation(shaderProgram, "Mshi"), 100.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2);
		
		//devo passar indices.size, que é a qtd de indices usados (3 para cada triangulo)
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		
		//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glBindVertexArray(0); //unbind

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	std::cout << "window closed" << std::endl;
	glfwTerminate();
	return;
}