#include <iostream>
#include <fstream>
#include <SDL.h>
#include <GLAD/glad.h>
#include <vector>

#undef main

#pragma region Definitions

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
	// Positions        // Colors
   -0.5f, -0.5f, 0.0f,  0.5f, 0.0f, 0.0f,  // Bottom-left vertex
	0.5f, -0.5f, 0.0f,  0.0f, 0.5f, 0.0f,  // Bottom-right vertex
   -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 0.5f,   // Top vertex
    0.5f,  0.5f, 0.0f,  0.5f, 0.0f, 0.0f  // Bottom-left vertex
};

std::vector<int> VertexIndices = {
	0, 1, 2, 1, 3, 2
};

GLuint VertexBuffer;

GLuint VertexArray;

GLuint IndexBufferObject;
#pragma endregion


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

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // ?

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // ?

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // ?


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
	glDeleteShader;
	glDetachShader;

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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), (int*)0);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), (int*)(3*sizeof(float)));

	glEnableVertexAttribArray(1);

	// VIO
	glGenBuffers(1, &IndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexIndices.size() * sizeof(int), VertexIndices.data(), GL_STATIC_DRAW);


}

void PreDraw()
{
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

	// glDrawArrays(GL_TRIANGLES, 0, 6);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SDL_GL_SwapWindow(MainWindow);
}

int main()
{
	Init(WindowXPos, WindowYPos, Width, Height);

	CreateShader();

	CreateVBOAndVAO();

	while (!AllowExit)
	{
		Input();

		PreDraw();

		Draw();
	}

	CleanUp();
	
	return 0;
}