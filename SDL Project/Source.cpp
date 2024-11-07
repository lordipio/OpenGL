#include <iostream>
#include <fstream>
#include <SDL.h>
#include <GLAD/glad.h>
#include <vector>
#include "GLM/glm/vec4.hpp"
#include "GLM/glm/vec3.hpp"
#include "GLM/glm/glm.hpp"
#include "GLM/glm/mat4x4.hpp"
#include "GLM/glm/ext/matrix_transform.hpp"
#include "GLM/glm/ext/matrix_clip_space.hpp"
#include "GLM/glm/ext/scalar_constants.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#undef main

#pragma region Definitions
bool ColorTest = true;

int WindowXPos = 0;

int WindowYPos = 0;

int Width = 600;

int Height = 600;

bool AllowExit = false;

std::string FragmentShaderSource;

std::string VertexShaderSource;

SDL_GLContext MainGLContext(nullptr);

SDL_Window* MainWindow(nullptr);

GLuint Program;

GLuint VertexShader;

GLuint FragmentShader;

std::vector<float> Vertices = {
	// Positions        // Colors		     // UVs
   -0.5f, -0.5f, 0.f,  0.5f, 0.0f, 0.0f,   0.f, 0.f,   // Bottom-left vertex
	0.5f, -0.5f, 0.f,  0.0f, 0.5f, 0.0f,   1.f, 0.f,  // Bottom-right vertex
   -0.5f,  0.5f, 0.f,  0.0f, 0.0f, 0.5f,   0.f, 1.f, // Top-Left vertex
    0.5f,  0.5f, 0.f,  0.5f, 0.0f, 0.0f,   1.f, 1.f // Top-Right vertex
};

std::vector<int> VertexIndices = {
	0, 1, 2, 1, 3, 2
};

GLuint VertexBuffer;

GLuint VertexArray;

GLuint IndexBufferObject;

GLuint MeteorTextureID;

glm::mat4 Model;

glm::mat4 Projection;

glm::mat4 View;


float XSpeed = 1.f;

float YSpeed = 1.f;

float PlayerXPosition = 0.f;

float PlayerYPosition = 0.f;

#pragma endregion

void CreateMVP()
{
	Model = glm::mat4(1.f);
	
	Model = glm::scale(Model, glm::vec3(1.f, 1.f, 1.f));

	Model = glm::translate(Model, glm::vec3(PlayerXPosition, PlayerYPosition, -5.f));

	Model = glm::rotate(Model, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));


	glm::vec3 Eye = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 Center = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 UpVector = glm::vec3(0.f, 1.f, 0.f);
	
	View = glm::lookAt(Eye, Center, UpVector);

	Projection = glm::perspective((float)glm::radians(45.f), float(Width/Height), 0.1f, 100.f);
}

void Init(int PosX, int PosY, int Height, int Width)
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Cannot Init SDL!";
		exit(0);
	}

	MainWindow = SDL_CreateWindow("My Window", PosX, PosY, Width, Height, SDL_WINDOW_OPENGL);

	if (!MainWindow)
	{
		std::cout << "Cannot Create Window!";
		exit(0);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	MainGLContext = SDL_GL_CreateContext(MainWindow);

	if (!MainGLContext)
	{
		std::cout << "Cannot Load GLContext!";
		exit(0);
	}


	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		std::cout << "Cannot Load GL Loader!";
		exit(0);
	}

}

std::string ReadShaderFile(std::string filepath)
{
	std::ifstream shaderFile(filepath);
	if (!shaderFile.is_open()) {
		std::cerr << "Could not open shader file: " << filepath << std::endl;
		return std::string("");
	}

	std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

	shaderFile.close();

	return shaderSource;
}

void Input()
{
	SDL_Event Event;

	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_KEYDOWN)
		{
			std::cout << "Down Button Is Pressed!";
		}

		if (Event.type == SDL_QUIT)
		{
			std::cout << "Quite Button Is Pressed!";
			AllowExit = true;
		}
	}
}

void CleanUp()
{
	glDeleteShader(FragmentShader);
	
	glDeleteShader(VertexShader);

	SDL_Quit();

	SDL_DestroyWindow(MainWindow);
}

void CreateProgram()
{
	Program = glCreateProgram();
}

void CreateShader()
{
	VertexShaderSource = ReadShaderFile("Vertex_Shader.glsl");

	FragmentShaderSource = ReadShaderFile("Fragment_Shader.glsl");

	VertexShader = glCreateShader(GL_VERTEX_SHADER);

	const GLchar* Code = VertexShaderSource.c_str();

	glShaderSource(VertexShader, 1, &Code, nullptr);

	glCompileShader(VertexShader);


	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	Code = FragmentShaderSource.c_str();

	glShaderSource(FragmentShader, 1, &Code, nullptr);

	glCompileShader(FragmentShader);


	CreateProgram();

	glAttachShader(Program, VertexShader);

	glAttachShader(Program, FragmentShader);

	glLinkProgram(Program);
}


void CreateVBOAndVAO()
{
	// VBO
	glGenBuffers(1, &VertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);


	// VAO
	glGenVertexArrays(1, &VertexArray);

	glBindVertexArray(VertexArray);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (int*)0);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (int*)(3*sizeof(float)));

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (int*)(6 * sizeof(float)));

	glEnableVertexAttribArray(2);

	// VIO
	glGenBuffers(1, &IndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexIndices.size() * sizeof(int), VertexIndices.data(), GL_STATIC_DRAW);

}

void PreDraw()
{



	// Uniform
	GLuint u_Color = glGetUniformLocation(Program, "u_Color");

	glUniform3f(u_Color, 1.f, 0.f, 0.f);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // handle alphas in shaders
	
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, MeteorTextureID);

	GLuint TextureLocation = glGetUniformLocation(Program, "MeteorTexture");
	if (TextureLocation != -1)
		glUniform1i(TextureLocation, 0);


	// set Matrices
	GLuint ModelLocation = glGetUniformLocation(Program, "u_Model");

	if (ModelLocation != -1)
		glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(Model));

	else
		std::cout << "Can't find Model Location!";

	GLuint ProjectionLocation = glGetUniformLocation(Program, "u_Projection");

	if (ProjectionLocation != -1)
		glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Projection));
	else
		std::cout << "Can't find Projection Location!";

	GLuint ViewLocation = glGetUniformLocation(Program, "u_View");

	if (ViewLocation)
		glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, glm::value_ptr(View));
	else
		std::cout << "Can't find View Location!";


	glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glViewport(GLint(WindowXPos), GLint(WindowYPos), GLint(Width), GLint(Height));

	glClearColor(GLfloat(0.5f), GLfloat(6.f), GLfloat(0.1f), GLfloat(1.f));

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(Program);
}

void Draw()
{
	glBindVertexArray(VertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);

	glEnableVertexAttribArray(0);

	glEnableVertexAttribArray(1);

	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, MeteorTextureID);

	// glDrawArrays(GL_TRIANGLES, 0, 6);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SDL_GL_SwapWindow(MainWindow);
}

void LoadTexture()
{
	glGenTextures(1, &MeteorTextureID);
	
	glBindTexture(GL_TEXTURE_2D, MeteorTextureID);


	glTexParameteri(MeteorTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(MeteorTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(MeteorTextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameteri(MeteorTextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	

	int width, height, nrChannels;
	unsigned char* data = stbi_load("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/Meteor.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		GLuint ColorFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, width, height, 0, ColorFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	stbi_image_free(data);
}

void MoveCharacter()
{

}

int main()
{
	Init(WindowXPos, WindowYPos, Width, Height);

	CreateShader();

	LoadTexture();

	CreateVBOAndVAO();

	CreateMVP();

	while (!AllowExit)
	{
		Input();

		PreDraw();

		Draw();
	}

	CleanUp();
	
	return 0;
}