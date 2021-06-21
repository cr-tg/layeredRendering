#include "DisplayTexture.h"
#include "Window.h"
#include "Render.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <iomanip>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 736;

namespace
{
	struct SVertex2D
	{
		float x, y;  //Position
		float u, v;  //Uv
	};

	struct SDrawElementsCommand
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	const SVertex2D gQuad[] = { {0.0f,0.0f,0.0f,0.0f},
								{0.1f,0.0f,1.0f,0.0f},
								{0.0f,0.1f,0.0f,1.0f},
								{0.1f,0.1f,1.0f,1.0f}
	};

	const unsigned int gIndex[] = { 0,1,2,1,3,2 };

	GLuint gArrayTexture(0);
	GLuint gVBO(0);
	GLuint gVAO(0);
	GLuint gElementBuffer(0);
	GLuint gProgram(0);

	GLuint gIndirectBuffer(0);
	GLuint gDrawIdBuffer(0);

}//Unnamed namespace

void GenerateGeometry()
{
	//Generate 100 little quads
	SVertex2D vVertex[400];
	int index(0);
	float xOffset(-0.95f);
	float yOffset(-0.95f);
	for (unsigned int i(0); i != 10; ++i)
	{
		for (unsigned int j(0); j != 10; ++j)
		{
			for (unsigned int k(0); k != 4; ++k)
			{
				vVertex[index].x = gQuad[k].x + xOffset;
				vVertex[index].y = gQuad[k].y + yOffset;
				vVertex[index].u = gQuad[k].u;
				vVertex[index].v = gQuad[k].v;
				index++;
			}
			xOffset += 0.2f;
		}
		yOffset += 0.2f;
		xOffset = -0.95f;
	}
	
	glGenVertexArrays(1,&gVAO);
	glBindVertexArray(gVAO);

	//Create a vertex buffer object
	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertex), vVertex, GL_STATIC_DRAW);

	//Specify vertex attributes for the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex2D), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SVertex2D), (GLvoid*)8);//两个数，每个4个字节

	//Create an element buffer
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndex), gIndex, GL_STATIC_DRAW);

	//Generate draw commands
	SDrawElementsCommand vDrawCommand[100];
	for (unsigned int i(0); i < 100; ++i)
	{
		vDrawCommand[i].vertexCount = 6;
		vDrawCommand[i].instanceCount = 1;
		vDrawCommand[i].firstIndex = 0;
		vDrawCommand[i].baseVertex = i * 4;
		vDrawCommand[i].baseInstance = i;
	}

	glGenBuffers(1, &gIndirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_STATIC_DRAW);


	//Generate an instanced vertex array to identify each draw call in the shader
	GLuint vDrawId[100];
	for (GLuint i(0); i < 100; i++)
	{
		vDrawId[i] = i;
	}

	//glGenBuffers(1, &gDrawIdBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, gDrawIdBuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vDrawId), vDrawId, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(2);
	//glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (GLvoid*)0);
	//glVertexAttribDivisor(2, 1);

	//NOTE: Instead of creating a new buffer for the drawID as we just did, 
 //we could use the "baseInstance" field from the gIndirectBuffer to 
 //provide the gl_DrawID to the shader. The code will look like this:
 
	glBindBuffer(GL_ARRAY_BUFFER, gIndirectBuffer );
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(SDrawElementsCommand), (void*)( 4 * sizeof(GLuint)) );
	glVertexAttribDivisor(2, 1);
	glBindVertexArray(0);

}

void GenerateArrayTexture()
{
	//Generate an array texture
	glGenTextures(1, &gArrayTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gArrayTexture);

	//Create storage for the texture. (100 layers of 1x1 texels)
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		1,                    //No mipmaps as textures are 1x1
		GL_RGB8,              //Internal format
		1, 1,                 //width,height
		100                   //Number of layers
	);

	for (unsigned int i(0); i != 100; ++i)
	{
		//Choose a random color for the i-essim image
		GLubyte color[3] = { rand() % 255,rand() % 255,rand() % 255 };

		//Specify i-essim image
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                     //Mipmap number
			0, 0, i,                 //xoffset, yoffset, zoffset
			1, 1, 1,                 //width, height, depth
			GL_RGB,                //format
			GL_UNSIGNED_BYTE,      //type
			color);                //pointer to data
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

int main()
{
	CWindow::setWindowName("LAYERD_MODEL_TRIANGLE");
	CWindow::setWindowSize(glm::uvec2(SCR_WIDTH, SCR_HEIGHT));
	GLFWwindow* window = CWindow::getOrCreateWindowInstance()->getGlfwWindow();

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, (1920 - SCR_WIDTH) / 2.0f, (1080 - SCR_HEIGHT) / 2.0f);

	glEnable(GL_DEPTH_TEST);

	//定义shader
	const std::string pathstr = "./shaders/";
	Shader indirectDrawShader((pathstr + "multiDrawIndirect.vs").c_str(), (pathstr + "multiDrawIndirect.fs").c_str());

	GenerateGeometry();
	GenerateArrayTexture();

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		//glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(gVAO);
		indirectDrawShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, gArrayTexture);


		glMultiDrawElementsIndirect(GL_TRIANGLES,GL_UNSIGNED_INT,
			(GLvoid*)0, //indirect: 包含了绘制参数的地址，如果绑定了GL_DRAW_INDIRECT_BUFFER，那么这个参数就代表是一个offset
			100,//drawcount:indirect buffer中的数组元素个数
			0);//stride:指定绘制参数数组元素之间的单位距离。

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

