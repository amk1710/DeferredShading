#pragma once
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <tiny_obj_loader.h>


using namespace std;

class GLWindowManager
{

private:
	GLFWwindow* window;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int colorBuffer;

	unsigned int screenWidth = 800;
	unsigned int screenHeight = 600;



	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;


	std::vector<float> vertices;
	std::vector<float> colors;
	std::vector<uint32_t> indices;

	bool IsTextureLoaded;
	bool IsBumpmapLoaded;
	int width;
	int height;
	int nrChannels;
	unsigned char *data;

	unsigned int tex1;
	unsigned int tex2;

	int width_bm;
	int height_bm;
	int nrChannels_bm;
	unsigned char *bump_data;

	unsigned int texture;
	unsigned int bumpmap;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 modelView;
	glm::mat4 ITmodelView;
	glm::mat4 modelViewProjection;

	float angle;

	int nLights;
	std::array<float, 3 * 32> lights;
	std::array<float, 3 * 32> lightsColors;

	//aux functions
	std::string GLWindowManager::readShaderFile(const char* name);
	void GLWindowManager::UpdateMVPMatrix();
	unsigned int GLWindowManager::loadTexture(char const * path);
	void GLWindowManager::FBO_2_file();
	void GLWindowManager::renderQuad();
	void GLWindowManager::BuildShader(const char* filepath, unsigned int* shaderID, int shader_enum);


	//NEW STUFF
	//gBuffer framebuffer stuff
	unsigned int gPass;
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gColorSpec, gTangent, gBitangent;

	unsigned int rboDepth;

	unsigned int lPass;

	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	
	

public:
	GLWindowManager();
	~GLWindowManager();
	void InitializeSceneInfo();
	void GLWindowManager::LoadModel(const char* objName, bool randomColors);
	void GLWindowManager::LoadTexture(const char* filepath);
	void GLWindowManager::LoadBumpmap(const char* filepath);
	void GLWindowManager::StartRenderLoop();


	float scale;
	glm::vec3 eye;
	glm::vec3 cameraTarget;
	glm::vec3 up;

	void processInput(GLFWwindow *window);
};