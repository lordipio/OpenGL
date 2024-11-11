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
#include <cstdlib>;
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/glm/gtx/string_cast.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

#undef main

#define GLM_ENABLE_EXPERIMENTAL

#define ASSERT(x) if (!(x)) __debugbreak();

#define GLCall(x) GLClearErrors();\
	x;\
	ASSERT(GLLogError(#x, __FILE__, __LINE__))

static void GLClearErrors()
{
	while (glGetError() != GL_NO_ERROR);
}


static bool GLLogError(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
		return false;
	}

	return true;
}



#pragma region Definitions
bool ColorTest = true;

int WindowXPos = 10;

int WindowYPos = 10;

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


float XSpeed = 0.015f;

float PlayerXTranslation = 0.f;

float PlayerXTranslationTreshold = 0.08f;

struct Enemy
{
	GLuint Texture;
	glm::vec3 SpawnLocation;
	float Scale;
	float Speed;
};

struct Player
{

};


#pragma endregion


class RenderObject
{
public:

	RenderObject()
	{

	}

	void SetProgram(GLuint Program)
	{
		this->Program = Program;
		glUseProgram(Program);
	}

	void CreateModel(glm::vec3 Scale, glm::vec3 Translate)
	{
		this->Scale = Scale;

		this->Translate = Translate;

		this->Model = glm::mat4(1);

		this->Model = glm::scale(this->Model, Scale);

		this->Model = glm::rotate(this->Model, 0.f, glm::vec3(1.f, 0.f, 0.f));

		this->Model = glm::translate(this->Model, Translate);

		//this->Model = glm::scale(Model, Scale);

		//this->Model = glm::translate(Model, Translate);

		//this->Model = glm::rotate(Model, glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));

	}

	void UpdateScale(glm::vec3 Scale)
	{
		this->Scale = Scale;

		this->Model = glm::scale(this->Model, Scale);
	}

	void UpdateRotation(glm::vec3 RotationAxis, float RotationValue)
	{
		this->Model = glm::rotate(this->Model, glm::radians(RotationValue), RotationAxis);
	}

	void UpdateTranslate(glm::vec3 Translate)
	{
		this->Translate = Translate;
		this->Model = glm::translate(this->Model, Translate);
		// this->Model = glm::translate(glm::mat4(1), glm::vec3(0.1, 0.f, 0.f)) * glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
	}

	GLuint SetTexture(std::string TexturePath, float AlphaReductionScale)
	{
		glGenTextures(1, &TextureID);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		int width, height, nrChannels;
		unsigned char* data = stbi_load(TexturePath.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cout << "Texture Data was not found!";
			return 0;
		}
		// make it transparent
		if (nrChannels == 4)  // If the texture has an alpha channel (RGBA)
		{
			// Modify alpha (reduce transparency)
			for (int i = 0; i < width * height * 4; i += 4) {
				data[i + 3] = data[i + 3] * AlphaReductionScale; // reduce alpha
			}
		}
		GLuint ColorFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, width, height, 0, ColorFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return TextureID;
	}

	void CreateBuffers(std::vector<float> Vertices, std::vector<int> VerticesIndex)
	{
		// VBO
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

		// VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// IBO
		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, VerticesIndex.size() * sizeof(float), VerticesIndex.data(), GL_STATIC_DRAW);

	}

private:
	void PreDraw(glm::vec3 Color)
	{
		// Uniform
		GLuint u_Color = glGetUniformLocation(Program, "u_Color");

		glUniform3f(u_Color, Color.r, Color.g, Color.b);

		// glEnable(GL_BLEND);

		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // handle alphas in shaders

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		GLuint TextureLocation = glGetUniformLocation(Program, "MeteorTexture");

		if (TextureLocation == -1)
			std::cout << "Texture Location was not found!";

		if (TextureLocation != -1)
			glUniform1i(TextureLocation, 0);


		// set Matrices
		// this->Model = TranslateMat * RotationMat * ScaleMat;
		GLuint ModelLocation = glGetUniformLocation(Program, "u_Model");

		if (ModelLocation != -1)
			glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(this->Model));

		else
			std::cout << "Can't find Model Location!";

		// std::cout << glm::to_string(TranslateMat) << std::endl;
		//this->Model = glm::mat4(1);
		//this->RotationMat = glm::mat4(1);
		//this->ScaleMat = glm::mat4(1);
		//this->TranslateMat = glm::mat4(1);

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

		// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glUseProgram(Program);
	}
public:

	void Draw(glm::vec3 Color)
	{
		PreDraw(Color);

		// DRAW
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glEnableVertexAttribArray(0);

		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// SDL_GL_SwapWindow(MainWindow);
	}



	__forceinline GLuint GetTextureID() { return TextureID; }
	__forceinline GLuint GetVAO() { return VAO; }
	__forceinline GLuint GetVBO() { return VBO; }
	__forceinline GLuint GetIBO() { return IBO; }

	glm::mat4 Model = glm::mat4(1);
	glm::mat4 TranslateMat;

	glm::mat4 ScaleMat;

	glm::mat4 RotationMat;
private:

	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	// GLuint Program;
	GLuint TextureID;
	GLuint Program;
	// std::vector<float> Vertices = {};

	glm::vec3 Scale = glm::vec3(3.f, 3.f, 3.f);
	glm::vec3 Translate = glm::vec3(0.f, 0.f, 0.f);




};

class Background // implement
{
public:

	Background(GLuint Program)
	{
		this->Program = Program;
		glUseProgram(Program);
	}

	void CreateModel(glm::vec3 Scale, glm::vec3 Translate)
	{
		this->Scale = Scale;

		this->Translate = Translate;

		this->Model = glm::mat4(1);

		this->Model = glm::scale(Model, Scale);

		this->Model = glm::rotate(Model, glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));

		this->Model = glm::translate(Model, Translate);
	}

	GLuint SetTexture(std::string TexturePath)
	{
		glGenTextures(1, &TextureID);

		glBindTexture(GL_TEXTURE_2D, TextureID);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(TexturePath.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cout << "Texture Data was not found!";
			return 0;
		}
		// make it transparent
		if (nrChannels == 4)  // If the texture has an alpha channel (RGBA)
		{
			// Modify alpha (reduce transparency)
			for (int i = 0; i < width * height * 4; i += 4) {
				data[i + 3] = data[i + 3] * 0.4f; // Reduce alpha (50% opacity)
			}
		}
		GLuint ColorFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, width, height, 0, ColorFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return TextureID;
	}

	void CreateBuffers(std::vector<float> Vertices, std::vector<int> VerticesIndex)
	{
		// VBO
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

		// VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// IBO
		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, VerticesIndex.size() * sizeof(float), VerticesIndex.data(), GL_STATIC_DRAW);

		// Texture
		//glGenTextures(1, &TextureID);
		//glBindTexture(GL_TEXTURE_2D, TextureID);
		// glTextureBuffer();

	}

private:
	void PreDraw()
	{
		// GLEnable(GL_DEPTH_TEST);
		// Uniform
		GLuint u_Color = glGetUniformLocation(Program, "u_Color");

		glUniform3f(u_Color, 1.f, 1.f, 1.f);

		//glEnable(GL_BLEND);

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // handle alphas in shaders

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		GLuint TextureLocation = glGetUniformLocation(Program, "MeteorTexture");

		if (TextureLocation == -1)
			std::cout << "Texture Location was not found!";

		if (TextureLocation != -1)
			glUniform1i(TextureLocation, 1);


		// set Matrices
		GLuint ModelLocation = glGetUniformLocation(Program, "u_Model");

		if (ModelLocation != -1)
			glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(this->Model));

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

		// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glUseProgram(Program);
	}
public:

	void Draw()
	{
		PreDraw();

		// DRAW
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

		glEnableVertexAttribArray(0);

		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// SDL_GL_SwapWindow(MainWindow);
	}



	__forceinline GLuint GetTextureID() { return TextureID; }
	__forceinline GLuint GetVAO() { return VAO; }
	__forceinline GLuint GetVBO() { return VBO; }
	__forceinline GLuint GetIBO() { return IBO; }

private:

	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	// GLuint Program;
	GLuint TextureID;
	GLuint Program;
	// std::vector<float> Vertices = {};

	glm::vec3 Scale = glm::vec3(3.f, 3.f, 3.f);
	glm::vec3 Translate = glm::vec3(0.f, 0.f, 0.f);
	glm::mat4 Model = glm::mat4(1);


};



void CreateMVP()
{

	// Player
	Model = glm::mat4(1.f);

	Model = glm::scale(Model, glm::vec3(1.f, 1.f, 1.f));

	Model = glm::translate(Model, glm::vec3(0.f, -4.2f, -10.f));

	Model = glm::rotate(Model, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));


	glm::vec3 Eye = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 Center = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 UpVector = glm::vec3(0.f, 1.f, 0.f);

	View = glm::lookAt(Eye, Center, UpVector);

	Projection = glm::perspective((float)glm::radians(45.f), float(Width / Height), 0.1f, 100.f);
}

void Init()
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Cannot Init SDL!";
		exit(0);
	}

	MainWindow = SDL_CreateWindow("My Window", WindowXPos, WindowYPos, Width, Height, SDL_WINDOW_OPENGL);

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


	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(GLint(0), GLint(0), GLint(Width), GLint(Height));

	glClearColor(GLfloat(0.1f), GLfloat(0.1f), GLfloat(0.1f), GLfloat(1.f));

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
		const Uint8* InputEventState = SDL_GetKeyboardState(nullptr);
		if (InputEventState[SDL_SCANCODE_LEFT])
		{
			std::cout << "Left Button Is Pressed!" << std::endl;
			PlayerXTranslation = -XSpeed;
		}

		if (Event.type == SDL_QUIT)
		{
			std::cout << "Quit Button Is Pressed!" << std::endl;

			AllowExit = true;
		}

		if (InputEventState[SDL_SCANCODE_RIGHT])
		{
			std::cout << "Right Button Is Pressed!" << std::endl;
			PlayerXTranslation = XSpeed;
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

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (int*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (int*)(6 * sizeof(float)));

	glEnableVertexAttribArray(2);

	// IBO
	glGenBuffers(1, &IndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexIndices.size() * sizeof(int), VertexIndices.data(), GL_STATIC_DRAW);

}

void PreDraw()
{
	// Uniform
	GLuint u_Color = glGetUniformLocation(Program, "u_Color");

	glUniform3f(u_Color, 1.f, 0.f, 0.f);


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


	// glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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

	// SDL_GL_SwapWindow(MainWindow);
}

void LoadTexture()
{
	glGenTextures(1, &MeteorTextureID);

	glBindTexture(GL_TEXTURE_2D, MeteorTextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
	if (PlayerXTranslation > 0)
		PlayerXTranslation += XSpeed;

	if (PlayerXTranslation < 0)
		PlayerXTranslation -= XSpeed;

	Model = glm::translate(Model, glm::vec3(PlayerXTranslation, 0.f, 0.f));

	if (PlayerXTranslation >= PlayerXTranslationTreshold || PlayerXTranslation <= -PlayerXTranslationTreshold)
		PlayerXTranslation = 0.f;

}

int main()
{
	Init();

	CreateShader();

	LoadTexture();

	CreateVBOAndVAO();

	CreateMVP();


	std::vector<float> BackgroundVertices = {
		// Positions        // UVs
	   -0.5f, -0.5f, -11.f,  0.f, 0.f,   // Bottom-left vertex
		0.5f, -0.5f, -11.f,  1.f, 0.f,  // Bottom-right vertex
	   -0.5f,  0.5f, -11.f,  0.f, 1.f, // Top-Left vertex
		0.5f,  0.5f, -11.f,  1.f, 1.f // Top-Right vertex
	};

	std::vector<int> BackgroundVertexIndices = {
		0, 1, 2, 1, 3, 2
	};


	Background MainBackground = Background(Program);

	MainBackground.CreateModel(glm::vec3(12.f, 12.f, 1.f), glm::vec3(0.f, 0.f, 0.f));

	MainBackground.SetTexture("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/Background.jpg");

	MainBackground.CreateBuffers(BackgroundVertices, BackgroundVertexIndices);


	std::vector<float> MeteorVertices = {
		// Positions        // UVs
	   -0.5f, -0.5f, 0.f,  0.f, 0.f,   // Bottom-left vertex
		0.5f, -0.5f, 0.f,  1.f, 0.f,  // Bottom-right vertex
	   -0.5f,  0.5f, 0.f,  0.f, 1.f, // Top-Left vertex
		0.5f,  0.5f, 0.f,  1.f, 1.f // Top-Right vertex
	};

	
	srand(time(nullptr));

	const int NumberOfMeteors = 7;

	RenderObject MeteorsObj[NumberOfMeteors];

	RenderObject* Meteors[NumberOfMeteors];

	for (int i = 0; i < NumberOfMeteors; i++)
	{
		Meteors[i] = &MeteorsObj[i];
	}



	float ScaleRand;

	float TranslateRandX;

	float TranslateRandY;
	
	float RotationRand;

	for (RenderObject* Meteor : Meteors)
	
	{
		ScaleRand = float(rand() % 100 + 1) / 100.f;

		TranslateRandX = float(rand() % 100 + 1) / 10.f;

		TranslateRandY = float(rand() % 100 + 1) / 10.f;

		RotationRand;

		Meteor->SetProgram(Program);

		Meteor->CreateModel(glm::vec3(ScaleRand, ScaleRand, ScaleRand), glm::vec3(TranslateRandX, TranslateRandY, -10.f));

		Meteor->SetTexture("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/Meteor.png", 1.f);

		Meteor->CreateBuffers(MeteorVertices, BackgroundVertexIndices);

	}

	int testTreshold = 1000.f;
	int i = 0;

	while (!AllowExit)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		Input();

		MoveCharacter();

		PreDraw();

		Draw();

		MainBackground.Draw();

		for (RenderObject* Meteor : Meteors)
		{
			// Meteor.UpdateScale(glm::vec3(1.f, 1.f, 1.f));

			// Meteor.UpdateRotation(glm::vec3(0.f, 0.f, 1.f), 15.f);

			Meteor->UpdateTranslate(glm::vec3(-0.0, -0.01f, 0.f));

			if (i < testTreshold)
				Meteor->Draw(glm::vec3(1.f, 1.f, 1.f));	

		}

		i++;


		// matrix = glm::translate(glm::mat4(1), glm::vec3(x, y, z)) * glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));

		// Meteor1.Model = glm::mat4(1.f);

		//Meteor1.RotationMat = glm::rotate(glm::mat4(1.f), 0.f, glm::vec3(1.f, 0.f, 0.f));

		//Meteor1.ScaleMat = glm::scale(glm::mat4(1), glm::vec3(1.f, 1.f, 1.f));

		//Meteor1.TranslateMat = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f));

		SDL_GL_SwapWindow(MainWindow);

	}

	CleanUp();

	return 0;
}