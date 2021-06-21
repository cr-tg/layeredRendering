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
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 736;

namespace
{
	struct Vertex2D
	{
		float x, y;  //Position
		float u, v;  //Uv
	};

	struct DrawElementsCommand
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	struct Matrix
	{
		float a0, a1, a2, a3;
		float b0, b1, b2, b3;
		float c0, c1, c2, c3;
		float d0, d1, d2, d3;
	};

	void setMatrix(Matrix* matrix, const float x, const float y)
	{
		/*
		1 0 0 0
		0 1 0 0
		0 0 1 0
		x y 0 1
		*/
		matrix->a0 = 1;
		matrix->a1 = matrix->a2 = matrix->a3 = 0;

		matrix->b1 = 1;
		matrix->b0 = matrix->b2 = matrix->b3 = 0;

		matrix->c2 = 1;
		matrix->c0 = matrix->c1 = matrix->c3 = 0;

		matrix->d0 = x;
		matrix->d1 = y;
		matrix->d2 = 0;
		matrix->d3 = 1;
	}

	struct SDrawElementsCommand
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	const std::vector<Vertex2D> gQuad = {
		//xy			//uv
		{ 0.0f,0.0f,	0.0f,0.0f },
		{ 0.1f,0.0f,	1.0f,0.0f },
		{ 0.05f, 0.05f, 0.5f, 0.5f},
		{ 0.0f,0.1f,	0.0f,1.0f },
		{ 0.1f,0.1f,	1.0f,1.0f }
	};

	const std::vector<Vertex2D> gTriangle =
	{
		{ 0.0f, 0.0f,	0.0f,0.0f},
		{ 0.05f, 0.1f,	0.5f,1.0f},
		{ 0.1f, 0.0f,	1.0f,0.0f}
	};

	const std::vector<unsigned int> gQuadIndex = {
		0,1,2,
		1,4,2,
		2,4,3,
		0,2,3
	};

	const std::vector<unsigned int> gTriangleIndex =
	{
		0,1,2
	};

	GLuint gVAO(0);
	GLuint gArrayTexture(0);
	GLuint gVertexBuffer(0);
	GLuint gElementBuffer(0);
	GLuint gIndirectBuffer(0);
	GLuint gMatrixBuffer(0);
	GLuint gProgram(0);

	float gMouseX(0);
	float gMouseY(0);

}//Unnamed namespace

void GenerateGeometry()
{
	//Generate 50 quads, 50 triangles
	const unsigned num_vertices = gQuad.size() * 50 + gTriangle.size() * 50;
	std::vector<Vertex2D> vVertex(num_vertices);
	Matrix vMatrix[100];
	unsigned vertexIndex(0);
	unsigned matrixIndex(0);
	//Clipspace, lower left corner = (-1, -1)
	float xOffset(-0.95f);
	float yOffset(-0.95f);
	// populate geometry
	for (unsigned int i(0); i != 10; ++i)
	{
		for (unsigned int j(0); j != 10; ++j)
		{
			//quad
			if (j % 2 == 0)
			{
				for (unsigned int k(0); k != gQuad.size(); ++k)
				{
					vVertex[vertexIndex++] = gQuad[k];
				}
			}
			//triangle
			else
			{
				for (unsigned int k(0); k != gTriangle.size(); ++k)
				{
					vVertex[vertexIndex++] = gTriangle[k];
				}
			}
			//set position in model matrix
			setMatrix(&vMatrix[matrixIndex++], xOffset, yOffset);
			xOffset += 0.2f;
		}
		yOffset += 0.2f;
		xOffset = -0.95f;
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
	//Create a vertex buffer object
	glGenBuffers(1, &gVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * vVertex.size(), vVertex.data(), GL_STATIC_DRAW);

	//Specify vertex attributes for the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, x)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, u)));

	//Create an element buffer and populate it
	int triangle_bytes = sizeof(unsigned int) * gTriangleIndex.size();
	int quad_bytes = sizeof(unsigned int) * gQuadIndex.size();
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_bytes + quad_bytes, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, quad_bytes, gQuadIndex.data());
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, quad_bytes, triangle_bytes, gTriangleIndex.data());

	//Setup per instance matrices
	//Method 1. Use Vertex attributes and the vertex attrib divisor
	glGenBuffers(1, &gMatrixBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gMatrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vMatrix), vMatrix, GL_STATIC_DRAW);
	//A matrix is 4 vec4s
	glEnableVertexAttribArray(3 + 0);
	glEnableVertexAttribArray(3 + 1);
	glEnableVertexAttribArray(3 + 2);
	glEnableVertexAttribArray(3 + 3);

	glVertexAttribPointer(3 + 0, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, a0)));
	glVertexAttribPointer(3 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, b0)));
	glVertexAttribPointer(3 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, c0)));
	glVertexAttribPointer(3 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, d0)));
	//Only apply one per instance
	glVertexAttribDivisor(3 + 0, 1);
	glVertexAttribDivisor(3 + 1, 1);
	glVertexAttribDivisor(3 + 2, 1);
	glVertexAttribDivisor(3 + 3, 1);

	//Method 2. Use Uniform Buffers. Not shown here
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
		GLubyte color[3] = { GLubyte(rand() % 255),GLubyte(rand() % 255),GLubyte(rand() % 255) };

		//Specify i-essim image
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                     //Mipmap number
			0, 0, i,               //xoffset, yoffset, zoffset
			1, 1, 1,               //width, height, depth
			GL_RGB,                //format
			GL_UNSIGNED_BYTE,      //type
			color);                //pointer to data
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void generateDrawCommands()
{
	//Generate draw commands
	SDrawElementsCommand vDrawCommand[100];
	GLuint baseVert = 0;
	for (unsigned int i(0); i < 100; ++i)
	{
		//quad
		if (i % 2 == 0)
		{
			vDrawCommand[i].vertexCount = 12;		//4 triangles = 12 vertices
			vDrawCommand[i].instanceCount = 1;		//Draw 1 instance
			vDrawCommand[i].firstIndex = 0;			//Draw from index 0 for this instance
			vDrawCommand[i].baseVertex = baseVert;	//Starting from baseVert
			vDrawCommand[i].baseInstance = i;		//gl_InstanceID
			baseVert += gQuad.size();
		}
		//triangle
		else
		{
			vDrawCommand[i].vertexCount = 3;		//1 triangle = 3 vertices
			vDrawCommand[i].instanceCount = 1;		//Draw 1 instance
			vDrawCommand[i].firstIndex = 0;			//Draw from index 0 for this instance
			vDrawCommand[i].baseVertex = baseVert;	//Starting from baseVert
			vDrawCommand[i].baseInstance = i;		//gl_InstanceID
			baseVert += gTriangle.size();
		}
	}

	//feed the draw command data to the gpu
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_DYNAMIC_DRAW);

	//feed the instance id to the shader.
	glBindBuffer(GL_ARRAY_BUFFER, gIndirectBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(SDrawElementsCommand), (void*)(offsetof(DrawElementsCommand, baseInstance)));
	glVertexAttribDivisor(2, 1); //only once per instance
}

int main()
{
	CWindow::setWindowName("LAYERD_MODEL_TRIANGLE");
	CWindow::setWindowSize(glm::uvec2(SCR_WIDTH, SCR_HEIGHT));
	GLFWwindow* window = CWindow::getOrCreateWindowInstance()->getGlfwWindow();
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, (1920 - SCR_WIDTH) / 2.0f, (1080 - SCR_HEIGHT) / 2.0f);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	unsigned int ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 100 * sizeof(SDrawElementsCommand), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);




	//定义shader
	const std::string pathstr = "./shaders/";
	Shader indirectDrawShader((pathstr + "IndirectDrawWithComputeShader.vs").c_str(), (pathstr + "IndirectDrawWithComputeShader.fs").c_str());

	GenerateGeometry();
	GenerateArrayTexture();

	//Generate one indirect draw buffer
	glGenBuffers(1, &gIndirectBuffer);
	generateDrawCommands();

	glEnable(GL_DEPTH_TEST);

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

		//populate light uniform
		//glUniform2f(glGetUniformLocation(gProgram, "light_pos"), gMouseX, gMouseY);

		//draw
		glMultiDrawElementsIndirect(GL_TRIANGLES, //type
			GL_UNSIGNED_INT, //indices represented as unsigned ints
			(GLvoid*)0, //start with the first draw command
			100, //draw 100 objects
			0); //no stride, the draw commands are tightly packed

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

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	gMouseX = -0.5f + float(xpos) / float(SCR_WIDTH);
	gMouseY = 0.5f - float(ypos) / float(SCR_HEIGHT);
}

