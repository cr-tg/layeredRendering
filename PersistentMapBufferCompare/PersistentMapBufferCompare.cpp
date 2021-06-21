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
const unsigned int SCR_WIDTH = 800;//1280 736
const unsigned int SCR_HEIGHT = 800;

namespace
{
  struct SVertex2D
  {
    float x;
    float y;
  };
                                                
  const SVertex2D gTrianglePosition[] = { {-0.5f,-0.5f}, {0.5f,-0.5f}, {0.0f,0.5f} };
  GLfloat gAngle = 0.0f;
  
  GLuint gVAO(0);
  GLuint gVertexBuffer(0);  
  SVertex2D* gVertexBufferData(0);
         
}//Unnamed namespace


int main( int argc, char** argv )
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
    Shader persistentMappedBufferShader((pathstr + "PersistentMapBufferCompare.vs").c_str(), (pathstr + "PersistentMapBufferCompare.fs").c_str());

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	//Create a vertex buffer object
	glGenBuffers(1, &gVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gTrianglePosition), gTrianglePosition, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	                    //参数5是步长，当VBO中的数据都表示一种意义的时候，可以将步长设置为0，来让openGL自动决定步长的大小。
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(gVAO);
		persistentMappedBufferShader.use();
		gAngle += 0.001f;
		//2960 - 3020 FPS
		persistentMappedBufferShader.setFloat("uAngle", gAngle);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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