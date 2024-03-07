#include "SDL.h"
#include "GLAD/include/glad/glad.h"
#include <iostream>
#include "vector"
#undef main

int gScreenHeight = 480;

int gScreenWidth = 640;

bool gQuit = false;

SDL_Window* gSDLWindow = nullptr;

SDL_GLContext gGLContext = nullptr;

std::string gVertexShader =
"#version 410 core\n"
"in vec4 position;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
"}\n";


std::string gFragmentShader =
"#version 410 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"	color = vec4(1.0f, 0.0f, 0.2f, 1.0f);\n"
"}\n";

GLuint gVertexArrayObject;

GLuint gVertexBufferObject;

GLuint gShaderProgram;

GLuint test = 0;

void InitProgram()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Can't Init SDL!" << std::endl;
		exit(1);
	}

	gSDLWindow = SDL_CreateWindow("The Great Window", 50, 50, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

	if (!gSDLWindow)
	{
		std::cout << "Can't Create gWindow!" << std::endl;
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gGLContext = SDL_GL_CreateContext(gSDLWindow);

	if (!gGLContext)
	{
		std::cout << "Can't Create gGLContext!";
		exit(1);
	}

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		std::cout << "Can't load glad function pointers!";
		exit(1);
	}

}

GLuint CompileShader(GLuint type, std::string source)
{
	GLuint shader = glCreateShader(type);

	const char* src = source.c_str();

	glShaderSource(shader, 1, &src, nullptr);

	glCompileShader(shader);

	return shader;
}

GLuint CreateShaderProgram(std::string vertexShader, std::string fragmentShader)
{
	GLuint shaderProgram = glCreateProgram();

	GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShader);

	GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(shaderProgram, myVertexShader);

	glAttachShader(shaderProgram, myFragmentShader);

	glLinkProgram(shaderProgram);

	glValidateProgram(shaderProgram);

	return shaderProgram;
}

void CreateGraphicsPipeline()
{
	gShaderProgram = CreateShaderProgram(gVertexShader, gFragmentShader);
}

void VertexSpecification()
{
	// Lives on CPU
	std::vector<GLfloat> vertexPosition {
		// X	 Y		Z
		-0.8f, -0.8f, 0.0f, // vertex 1
			0.8f, -0.8f, 0.0f, // vertex 2
			0.0f, 0.8f, 0.0f  // vertex 3
	};

	// Start setting things up on GPU

	// Generate VAO
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// Generate VBO
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, vertexPosition.size() * sizeof(GLfloat), vertexPosition.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, GLint(3), GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);

}

void Input()
{
	SDL_Event Event;

	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_QUIT)
		{
			std::cout << "Quit!";
			gQuit = true;
		}

		if (Event.type == SDL_KEYDOWN) // exit if tap on key down
		{
			std::cout << "Quit!";
			gQuit = true;
		}
	}
}

void PreDraw()
{
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glViewport(GLint(0), GLint(0), gScreenWidth, gScreenHeight);

	glClearColor(GLfloat(1.f), GLfloat(1.f), GLfloat(0.1f), GLfloat(1.f));

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glUseProgram(gShaderProgram);
}

void Draw()
{
	glBindVertexArray(gVertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GetOpenGLContent()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;

	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;

	std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void MainLoop()
{
	while (!gQuit)
	{
		Input();

		PreDraw();

		Draw();

		SDL_GL_SwapWindow(gSDLWindow);
	}
}

void CleanUp()
{
	SDL_Quit();

	SDL_DestroyWindow(gSDLWindow);
}

int main()
{
	InitProgram();

	VertexSpecification();

	CreateGraphicsPipeline();

	GetOpenGLContent();

	MainLoop();

	CleanUp();

	return 0;
}
