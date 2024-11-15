#include <iostream>
#include <fstream>
#include <SDL.h>
#include <chrono>
#include <thread>
#include <functional>
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



struct Transformation
{
	float CircleCollisionRadius;

	glm::vec3 Position;

	glm::vec3 RotationAmount;

	glm::vec3 Scale;

	glm::mat4 TranslationMatrix;

	glm::mat4 RotationMatrix;

	glm::mat4 ScaleMatrix;

	glm::mat4 TransformationMatrix;
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
	}

	void SetView()
	{
		glm::vec3 Eye = glm::vec3(0.f, 0.f, 3.f);
		glm::vec3 Center = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 UpVector = glm::vec3(0.f, 1.f, 0.f);
		
		this->View = glm::lookAt(Eye, Center, UpVector);
	}

	void SetProjection(float Width, float Height)
	{
		this->Projection = glm::perspective((float)glm::radians(45.f), float(Width / Height), 0.1f, 100.f);
	}

	void SetModel(glm::vec3 Scale, glm::vec3 RotationValue, glm::vec3 Translate)
	{
		ObjectTransformation.TransformationMatrix = glm::mat4(1.f);

		ObjectTransformation.TransformationMatrix = glm::translate(ObjectTransformation.TransformationMatrix, Translate);

		ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.r), glm::vec3(1, 0, 0));

		ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.g), glm::vec3(0, 1, 0));

		ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.b), glm::vec3(0, 0, 1));


		ObjectTransformation.TransformationMatrix = glm::scale(ObjectTransformation.TransformationMatrix, Scale);
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
		GLuint u_Color = glGetUniformLocation(this->Program, "u_Color");

		glUniform3f(u_Color, Color.r, Color.g, Color.b);


		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		GLuint TextureLocation = glGetUniformLocation(this->Program, "MeteorTexture");

		if (TextureLocation == -1)
			std::cout << "Texture Location was not found!";

		if (TextureLocation != -1)
			glUniform1i(TextureLocation, 0);


		// set Matrices
		GLuint ModelLocation = glGetUniformLocation(this->Program, "u_Model");

		if (ModelLocation != -1)
			glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(ObjectTransformation.TransformationMatrix));

		else
			std::cout << "Can't find Model Location!";


		GLuint ProjectionLocation = glGetUniformLocation(this->Program, "u_Projection");

		if (ProjectionLocation != -1)
			glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(this->Projection));

		else
			std::cout << "Can't find Projection Location!";

		GLuint ViewLocation = glGetUniformLocation(this->Program, "u_View");

		if (ViewLocation)
			glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, glm::value_ptr(this->View));
		else
			std::cout << "Can't find View Location!";

		glUseProgram(this->Program);
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

	}

	bool CollisionDetection(RenderObject* OtherObject)
	{
		DistanceBetweenCircles = std::pow((OtherObject->ObjectTransformation.Position.r - this->ObjectTransformation.Position.r), 2) + std::pow((OtherObject->ObjectTransformation.Position.g - this->ObjectTransformation.Position.g), 2);

		CollisionClosestState = std::pow((OtherObject->ObjectTransformation.CircleCollisionRadius + OtherObject->ObjectTransformation.CircleCollisionRadius), 2);

		return DistanceBetweenCircles < CollisionClosestState;
	}

	__forceinline GLuint GetTextureID() { return TextureID; }

	__forceinline GLuint GetVAO() { return VAO; }

	__forceinline GLuint GetVBO() { return VBO; }

	__forceinline GLuint GetIBO() { return IBO; }

	Transformation ObjectTransformation;

	glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f);

private:

	float DistanceBetweenCircles;
	float CollisionClosestState;
	
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	GLuint TextureID;
	GLuint Program;


	glm::mat4 Projection;

	glm::mat4 View;
};

class EnemyObject : public RenderObject
{
public:
	float YMovementSpeed;
	float ZRotationSpeed;
};

class PlayerObject : public RenderObject
{
public:

	PlayerObject()
	{
		this->PlayerXMovementStep = this->XSpeed / this->StepsCount;
	}

	void SetupMovement()
	{

	}

	void MoveCharacterRight(float& PlayerXTranslation)
	{
		this->ObjectTransformation.Position.r = PlayerXTranslation;

		if (CurrentStep <= StepsCount)
		{
			this->ObjectTransformation.Position.r += PlayerXMovementStep;

			if (PlayerXMovementStep < 0)
				ObjectTransformation.RotationAmount.g = 180.f;
			
			if (PlayerXMovementStep > 0)
				ObjectTransformation.RotationAmount.g = 0.f;
			// std::cout << PlayerXTranslation << std::endl;

			this->CurrentStep++;
		}
	}

	// float PlayerXTranslation = 0.f;
	
	float PlayerXMovementStep = 0.f;
	
	unsigned int CurrentStep = 10; // Player won't move at first
	
	float XSpeed = 1.f;

	unsigned int StepsCount = 10;

};


void Init(SDL_Window*& MainWindow, SDL_GLContext& MainGLContext, float WindowXPos, float WindowYPos, float Width, float Height)
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

	if (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0)
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



void CleanUp(GLuint FragmentShader, GLuint VertexShader, SDL_Window*& MainWindow) // correct it
{
	glDeleteShader(FragmentShader);

	glDeleteShader(VertexShader);

	SDL_Quit();

	SDL_DestroyWindow(MainWindow);
}

GLuint CreateProgram(GLuint FragmentShader, GLuint VertexShader)
{
	GLuint Program = glCreateProgram();

	glAttachShader(Program, VertexShader);

	glAttachShader(Program, FragmentShader);

	glLinkProgram(Program);

	return Program;
}

GLuint CreateShader(std::string ShaderPath, GLenum ShaderType)
{
	std::string ShaderSource = ReadShaderFile(ShaderPath);

	GLuint Shader = glCreateShader(ShaderType);

	const GLchar* ShaderCode = ShaderSource.c_str();

	glShaderSource(Shader, 1, &ShaderCode, nullptr);

	glCompileShader(Shader);

	return Shader;

}

void Input(float& PlayerXMovementStep, unsigned int& CurrentStep, bool& AllowExit, float DeltaTime)
{
	SDL_Event Event;

	while (SDL_PollEvent(&Event))
	{
		const Uint8* InputEventState = SDL_GetKeyboardState(nullptr);
		if (InputEventState[SDL_SCANCODE_LEFT])
		{
			std::cout << "Left Button Is Pressed!" << std::endl;


			PlayerXMovementStep = -abs(PlayerXMovementStep);
			CurrentStep = 0;
			// std::cout << PlayerXMovementStep << std::endl;
			//PlayerXTranslation -= XSpeed * DeltaTime;
		}

		if (Event.type == SDL_QUIT)
		{
			std::cout << "Quit Button Is Pressed!" << std::endl;

			AllowExit = true;
		}

		if (InputEventState[SDL_SCANCODE_RIGHT])
		{
			std::cout << "Right Button Is Pressed!" << std::endl;
			PlayerXMovementStep = abs(PlayerXMovementStep);
			CurrentStep = 0;

		}
	}
}

float RandBetween(float Min, float Max)
{
	double scale = static_cast<double>(std::rand()) / RAND_MAX;
	return Min + scale * (Max - Min);
}

float RandBetween(int Min, int Max)
{
	return float(Min + std::rand() % (Max - Min + 1));
}

int main()
{
	std::vector<float> MeteorVertices = {
		// Positions        // UVs
	   -0.5f, -0.5f, 0.f,  0.f, 0.f,   // Bottom-left vertex
		0.5f, -0.5f, 0.f,  1.f, 0.f,  // Bottom-right vertex
	   -0.5f,  0.5f, 0.f,  0.f, 1.f, // Top-Left vertex
		0.5f,  0.5f, 0.f,  1.f, 1.f // Top-Right vertex
	};

	std::vector<float> BackgroundVertices = {
		// Positions        // UVs
	   -0.5f, -0.5f, -11.f,  0.f, 0.f,   // Bottom-left vertex
		0.5f, -0.5f, -11.f,  1.f, 0.f,  // Bottom-right vertex
	   -0.5f,  0.5f, -11.f,  0.f, 1.f, // Top-Left vertex
		0.5f,  0.5f, -11.f,  1.f, 1.f // Top-Right vertex
	};

	std::vector<int> RectangleVertexIndices = {
		0, 1, 2, 1, 3, 2
	};

	SDL_GLContext MainGLContext(nullptr);

	SDL_Window* MainWindow(nullptr);

	int WindowXPos = 10;

	int WindowYPos = 10;

	int Width = 600;

	int Height = 600;

	bool AllowExit = false;



	Init(MainWindow, MainGLContext, WindowXPos, WindowYPos, Width, Height);


	GLuint VertexShader = CreateShader("Vertex_Shader.glsl", GL_VERTEX_SHADER);

	GLuint FragmentShader = CreateShader("Fragment_Shader.glsl", GL_FRAGMENT_SHADER);

	GLuint Program = CreateProgram(FragmentShader, VertexShader);



	PlayerObject Player;

	Player.ObjectTransformation.Position = glm::vec3(0.f, -3.f, -6.f);

	Player.ObjectTransformation.RotationAmount = glm::vec3(0.f, 0.f, 180.f);
	
	Player.ObjectTransformation.Scale = glm::vec3(1.f, 0.7f, 1.f);

	Player.ObjectTransformation.CircleCollisionRadius = 0.3f;



	Player.SetProgram(Program);

	Player.SetView();

	Player.SetProjection(Width, Height);

	Player.SetModel(Player.ObjectTransformation.Scale, Player.ObjectTransformation.RotationAmount, Player.ObjectTransformation.Position);

	Player.SetTexture("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/UFO.png", 1.f);

	Player.CreateBuffers(MeteorVertices, RectangleVertexIndices);



	RenderObject MainBackground = RenderObject();

	MainBackground.SetProgram(Program);

	MainBackground.SetModel(glm::vec3(12.f, 12.f, 1.f), glm::vec3(0.f, 0.f, 180.f), glm::vec3(0.f, 0.f, 0.f));

	MainBackground.SetView();

	MainBackground.SetProjection(Width, Height);

	MainBackground.SetTexture("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/Background.jpg", 0.3f);

	MainBackground.CreateBuffers(BackgroundVertices, RectangleVertexIndices);



	const int NumberOfMeteors = 50;

	EnemyObject MeteorsObj[NumberOfMeteors];

	EnemyObject* Meteors[NumberOfMeteors];


	for (int i = 0; i < NumberOfMeteors; i++)
	{
		Meteors[i] = &MeteorsObj[i];
	}

	
	float ScaleRand;

	float RandomXPosition;

	float TranslateRandY;

	float RotationRand;

	float CurentMeteorScale = 1.f;

	float CurrentMeteorTranslationX = 0.f;

	float TranslationAdditiveVal = 0.01;

	float CurrentMeteorRotation = 0.0f;

	float RotationAdditiveVal = 1.f;

	float CurrentTime = 0.f;

	float LastTime = 0.f;

	float DeltaTime = RandBetween(3, 7);

	int RightSide = 1.f;
	
	for (EnemyObject* Meteor : Meteors)
	{
		ScaleRand = RandBetween(0.4, 1.f);

		Meteor->ZRotationSpeed = RandBetween(0.3f, 1.f);

		Meteor->YMovementSpeed = RandBetween(0.013f, 0.022f);

		if (RandBetween(1, 2) == 1)
			RightSide = 1.f;
		else
			RightSide = -1.f;


		RandomXPosition = RightSide * (float(rand() % 100 + 1) / 25.f);


		Meteor->SetProgram(Program);

		Meteor->ObjectTransformation.Position = glm::vec3(RandomXPosition, 4.5f, -7.f);

		Meteor->ObjectTransformation.Scale = glm::vec3(ScaleRand, ScaleRand, ScaleRand);

		Meteor->ObjectTransformation.RotationAmount = glm::vec3(0.f);

		Meteor->ObjectTransformation.CircleCollisionRadius = 0.25f * ScaleRand;


		
		glm::vec3 RandomColor = glm::vec3(RandBetween(0.6f, 1.f), RandBetween(0.4f, 1.f), RandBetween(0.6f, 1.f));
		Meteor->Color = (RandomColor);


		Meteor->SetModel(Meteor->ObjectTransformation.Scale, glm::vec3(0.f, 0.f, 0.f), Meteor->ObjectTransformation.Position);

		Meteor->SetView();

		Meteor->SetProjection(Width, Height);

		Meteor->SetTexture("D:/Desktop/OpenGL Project/SDL Project/SDL Project/PNGs/Meteor.png", 1.f);

		Meteor->CreateBuffers(MeteorVertices, RectangleVertexIndices);

	}

	int testTreshold = 1000.f;
	int i = 0;

	std::vector<EnemyObject*> UnspawnedEnemies;

	std::vector<EnemyObject*> SpawnedEnemies;

	for (int i = 0; i < NumberOfMeteors; i++)
	{
		UnspawnedEnemies.push_back(Meteors[i]);
	}





	while (!AllowExit)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		CurrentTime = SDL_GetTicks() / 1000;


		Input(Player.PlayerXMovementStep, Player.CurrentStep, AllowExit, DeltaTime);

		Player.MoveCharacterRight(Player.ObjectTransformation.Position.r);

		Player.SetModel(Player.ObjectTransformation.Scale, Player.ObjectTransformation.RotationAmount, Player.ObjectTransformation.Position);

		Player.Draw(glm::vec3(1.f, 1.f, 1.f));


		MainBackground.Draw(glm::vec3(1.f, 1.f, 1.f));



		if (CurrentTime - LastTime > DeltaTime)
		{
			if (UnspawnedEnemies.size() >= 1)
			{

				DeltaTime = RandBetween(0.01f, 0.07f);

				LastTime = CurrentTime;
				const int SelectedMeteor = RandBetween(0, UnspawnedEnemies.size() - 1);
				SpawnedEnemies.push_back(UnspawnedEnemies[SelectedMeteor]);
				UnspawnedEnemies.erase(UnspawnedEnemies.begin() + SelectedMeteor);
			}
		}


		for (EnemyObject* SpawnedEnemy : SpawnedEnemies)
		{
			SpawnedEnemy->ObjectTransformation.Position.g -= SpawnedEnemy->YMovementSpeed;

			SpawnedEnemy->ObjectTransformation.RotationAmount.b += SpawnedEnemy->ZRotationSpeed;

			SpawnedEnemy->SetModel(SpawnedEnemy->ObjectTransformation.Scale, SpawnedEnemy->ObjectTransformation.RotationAmount, SpawnedEnemy->ObjectTransformation.Position);

			SpawnedEnemy->Draw(SpawnedEnemy->Color);

			if (SpawnedEnemy->CollisionDetection(&Player))
				std::cout << "COLLIDED!";

			if (SpawnedEnemy->ObjectTransformation.Position.g <= -5.f) // out of the screen
			{
				SpawnedEnemy->ObjectTransformation.Position.g = 4.5;
				UnspawnedEnemies.push_back(SpawnedEnemy);
				auto i = std::find(SpawnedEnemies.begin(), SpawnedEnemies.end(), SpawnedEnemy);
				std::cout << i - SpawnedEnemies.begin() << std::endl;
				SpawnedEnemies.erase(std::find(SpawnedEnemies.begin(), SpawnedEnemies.end(), SpawnedEnemy));
			}
			
		}

		SDL_GL_SwapWindow(MainWindow);

	}

	CleanUp(FragmentShader, VertexShader, MainWindow);

	return 0;
}