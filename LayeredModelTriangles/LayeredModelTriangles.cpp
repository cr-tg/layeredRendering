#include "DisplayTexture.h"
#include "Window.h"
#include "Render.h"
#include <glad/glad.h>
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void setShaderViewAndProj(Shader& vShader, const glm::mat4& view, const glm::mat4& proj);

#define listLayer 20
GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

// settings
const unsigned int SCR_WIDTH = 1280; 
const unsigned int SCR_HEIGHT = 736; 
const unsigned int SHADOW_MAP_WIDTH = 1280;
const unsigned int SHADOW_MAP_HEIGHT = 736;
//gbuffer中的数据
unsigned int gPositionTex;
unsigned int gNormalTex;
unsigned int gRboDepth;
unsigned int gBufferFBO;
void setGBuffer();


GLuint64 maxListNodeNum = SHADOW_MAP_WIDTH * SHADOW_MAP_HEIGHT * listLayer;
GLuint64 ssboNodeSize = sizeof(glm::uvec4);
unsigned int layerNumImage;
unsigned int headPtrImage;
unsigned int ssbo;
void setLLImagesAndssbo();

GLuint headPtrInitPBO;
GLuint layerNumFBO;
unsigned int atomicBuffer;
const int one = 1;
void setAboAndPbo();
void initImages();
void initAbo();//这个函数只能放在生成了linklist以后调用，否则有片段闪烁


// camera
Camera camera(glm::vec3(0.0f, 0.2f, 5.5f));


float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int cubeVAO;
unsigned int cubeVBO;
unsigned int quadVAO;
unsigned int quadVBO;
unsigned int lightProjVAO;
unsigned int lightProjVBO;

int main()
{
	CWindow::setWindowName("LAYERD_MODEL_TRIANGLE");
	CWindow::setWindowSize(glm::uvec2(SCR_WIDTH, SCR_HEIGHT));
	GLFWwindow* window = CWindow::getOrCreateWindowInstance()->getGlfwWindow();

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, (1920 - SCR_WIDTH) / 2.0f, (1080 - SCR_HEIGHT) / 2.0f);

	CRender* dllRender = CRender::getOrCreateWindowInstance()->getRender();

	glEnable(GL_DEPTH_TEST);

	setLLImagesAndssbo();
	setAboAndPbo();

	//定义shader

	const std::string pathstr = "./shaders/";
	Shader linkListGenerateShader((pathstr + "LinkListGenerate.vs").c_str(), (pathstr + "LinkListGenerate.fs").c_str(), (pathstr + "LinkListGenerate.gs").c_str());
	Shader linkListSortBlendShader((pathstr + "LinkListSortBlend.vs").c_str(), (pathstr + "LinkListSortBlend.fs").c_str());

	Shader ourShader((pathstr + "model_loading.vs").c_str(), (pathstr + "model_loading.fs").c_str());

	linkListGenerateShader.use(); 
	linkListGenerateShader.setInt("uMaxListNode", maxListNodeNum);

	//Model singleQuadModel(FileSystem::getPath("resources/objects/test/cube.obj"));
	Model singleQuadModel(FileSystem::getPath("resources/objects/basicModel/singleQuad.obj"));
	Model complexQuadModel(FileSystem::getPath("resources/objects/basicModel/ComplexQuad.obj"));


	float camera_nearPlane = 0.01f, camera_farPlane = 15.0f;

	while (!glfwWindowShouldClose(window))
	{
		initImages();
		initAbo();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		glm::mat4 cameraView = camera.GetViewMatrix();
		glm::mat4 cameraProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, camera_nearPlane, camera_farPlane);

		auto observedView = cameraView;
		auto observedProj = cameraProjection;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.2f, 4.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
		//model = glm::scale(model, glm::vec3(0.1f));

		//auto mvp = observedProj * observedView * model;

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//根据三角片元到相机的距离产生mesh

		//生成linklist
		glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		linkListGenerateShader.use();
		linkListGenerateShader.setFloat("time", glfwGetTime());
		linkListGenerateShader.setBool("offset", true);
		linkListGenerateShader.setVec3("cameraPos", camera.Position);
		setShaderViewAndProj(linkListGenerateShader, observedView, observedProj);
		linkListGenerateShader.setMat4("model", model);


		glDepthMask(GL_FALSE);
		//singleQuadModel.Draw(linkListGenerateShader);
		complexQuadModel.Draw(linkListGenerateShader);
		
		//dllRender->renderSingTriangle(linkListGenerateShader, glm::vec3(0.0f, 0.2f, 4.0f), glm::vec4(0.0, 1.0, 0.0, 0.5));
		glDepthMask(GL_TRUE);
		glMemoryBarrierByRegion(GL_SHADER_STORAGE_BARRIER_BIT);

		/*unsigned int layerNum = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, layerNumFBO);
		glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &layerNum);*/


		//排序linklist
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		linkListSortBlendShader.use();
		linkListSortBlendShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
		//linkListSortBlendShader.setInt("uLayerNum", layerNum);
		dllRender->renderQuad();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &quadVBO);

	glfwTerminate();
	return 0;
}

void setGBuffer()
{
	glGenTextures(1, &gPositionTex);
	glBindTexture(GL_TEXTURE_2D, gPositionTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gNormalTex);
	glBindTexture(GL_TEXTURE_2D, gNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &gRboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gRboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);

	glGenFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTex, 0);
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void setLLImagesAndssbo()
{
	glGenTextures(1, &headPtrImage);
	glBindTexture(GL_TEXTURE_2D, headPtrImage);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindImageTexture(0, headPtrImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	glGenTextures(1, &layerNumImage);
	glBindTexture(GL_TEXTURE_2D, layerNumImage);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindImageTexture(1, layerNumImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxListNodeNum * ssboNodeSize, NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenFramebuffers(1, &layerNumFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, layerNumFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layerNumImage, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setAboAndPbo()
{
	glGenBuffers(1, &atomicBuffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint64), nullptr, GL_DYNAMIC_COPY);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint64), &one);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	glGenBuffers(1, &headPtrInitPBO);
	glBindBuffer(GL_ARRAY_BUFFER, headPtrInitPBO);
	glBufferData(GL_ARRAY_BUFFER, SHADOW_MAP_WIDTH * SHADOW_MAP_WIDTH * sizeof(GL_R32UI), NULL, GL_STATIC_DRAW);
	void* initData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memset(initData, 0, SHADOW_MAP_WIDTH * SHADOW_MAP_WIDTH * sizeof(GL_R32UI));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initImages()
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, headPtrInitPBO);
	glBindTexture(GL_TEXTURE_2D, headPtrImage);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, layerNumImage);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}

void initAbo()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint64), &one);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void setShaderViewAndProj(Shader& vShader, const glm::mat4& view, const glm::mat4& proj)
{
	vShader.use();
	vShader.setMat4("projection", proj);
	vShader.setMat4("view", view);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
	{
	}
}

// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
