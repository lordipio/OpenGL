
#include <RenderObject.h>
#include <PlayerObject.h>
#include <EnemyObject.h>
#include <Animation.h>
#include <SDL.h>
#include <fstream>


#pragma once

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

void Input(float& PlayerXMovementStep, unsigned int& CurrentStep, bool& AllowExit)
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
	std::vector<float> RectangleVertices = {
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

	const int NumberOfMeteors = 15;

	float ScaleRand;

	float RandomXPosition;

	float CurrentTime = 0.f;

	float EnemySpawnLastTime = 0.f;
	
	float CollisionHitLastTime = 0.f;

	float CollisionDisableTime = 2.f;

	float SpawnTime = RandBetween(3, 7);

	int SpawnPositionRightSide = 1.f;

	float DeltaTime = 0.f;

	float LastFrameTime = 0.f;

	float MeteorZRotationSpeed = 0.f;

	float MeteorYMovementSpeed = 0.f;


	Init(MainWindow, MainGLContext, WindowXPos, WindowYPos, Width, Height);


	GLuint VertexShader = CreateShader("./Vertex_Shader.glsl", GL_VERTEX_SHADER);

	GLuint FragmentShader = CreateShader("./Fragment_Shader.glsl", GL_FRAGMENT_SHADER);

	GLuint Program = CreateProgram(FragmentShader, VertexShader);


	// Create Player

	PlayerObject Player;

	Player.SetupObject(glm::vec3(1.f, 0.6f, 1.f), glm::vec3(0.f, 0.f, 180.f), glm::vec3(0.f, -3.f, -6.f), "./PNGs/UFO.png", 1.f, RectangleVertices, RectangleVertexIndices, Program, Width, Height, 3, 50.f, 10);

	Player.SetCollisionRadius(0.3f);
	

	// Create Background

	RenderObject MainBackground;

	MainBackground.SetupObject(glm::vec3(12.f, 12.f, 1.f), glm::vec3(0.f, 0.f, 180.f), glm::vec3(0.f, 0.f, 0.f), "./PNGs/Background.jpg", 0.3f, BackgroundVertices, RectangleVertexIndices, Program, Width, Height);



	// Create Meteors

	EnemyObject MeteorsObj[NumberOfMeteors];

	EnemyObject* Meteors[NumberOfMeteors];


	for (int i = 0; i < NumberOfMeteors; i++)
	{
		Meteors[i] = &MeteorsObj[i];
	}

	for (EnemyObject* Meteor : Meteors)
	{
		ScaleRand = RandBetween(0.4, 1.f);

		MeteorZRotationSpeed = RandBetween(30.f, 60.f);

		MeteorYMovementSpeed = RandBetween(2.f, 6.f);

		if (RandBetween(1, 2) == 1)
			SpawnPositionRightSide = 1.f;
		else
			SpawnPositionRightSide = -1.f;

		RandomXPosition = SpawnPositionRightSide * (float(rand() % 100 + 1) / 25.f);

		Meteor->SetupObject(glm::vec3(ScaleRand, ScaleRand, ScaleRand), glm::vec3(0.f, 0.f, 0.f), glm::vec3(RandomXPosition, 4.5f, -7.f), "./PNGs/Meteor.png", 1.f, RectangleVertices, RectangleVertexIndices, Program, Width, Height, MeteorZRotationSpeed, MeteorYMovementSpeed, glm::vec3(RandBetween(0.6f, 1.f), RandBetween(0.4f, 1.f), RandBetween(0.6f, 1.f)));
		Meteor->SetCollisionRadius(0.25f * ScaleRand);

	}


	std::vector<EnemyObject*> UnspawnedEnemies;

	std::vector<EnemyObject*> SpawnedEnemies;

	for (int i = 0; i < NumberOfMeteors; i++)
	{
		UnspawnedEnemies.push_back(Meteors[i]);
	}



	// Create Explosion Animation
	float AnimationLastTime = 0.f;

	Animation HitAnimation;

	HitAnimation.SetupObject(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.f, 0.f, 180.f), glm::vec3(0.f, -3.f, -6.f), "./PNGs/Background.jpg", 1.f, RectangleVertices, RectangleVertexIndices, Program, Width, Height, 16, 0.05f);


	Animation DeadAnimation;

	DeadAnimation.SetupObject(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.f, 0.f, 180.f), glm::vec3(0.f, -3.f, -6.f), "./PNGs/ExplosionAnimation/Explosion1.png", 1.f, RectangleVertices, RectangleVertexIndices, Program, Width, Height, 10, 0.1f);


	while (!AllowExit)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		CurrentTime = SDL_GetTicks() / 1000.f;

		DeltaTime = CurrentTime - LastFrameTime;


		Input(Player.PlayerXMovementStep, Player.CurrentStep, AllowExit);

		Player.MoveCharacterRight(Player.ObjectTransformation.Position.r, DeltaTime);

		Player.SetModel(Player.ObjectTransformation.Scale, Player.ObjectTransformation.RotationAmount, Player.ObjectTransformation.Position);

		if (Player.IsAlive())
			Player.Draw(Player.Color);

		else
		{
			DeadAnimation.PlayAnimation(CurrentTime, "./PNGs/ExplosionAnimation/Explosion");
			DeadAnimation.SetModel(DeadAnimation.ObjectTransformation.Scale, DeadAnimation.ObjectTransformation.RotationAmount, glm::vec3(Player.ObjectTransformation.Position.r, Player.ObjectTransformation.Position.g, Player.ObjectTransformation.Position.b + 0.1f));
			DeadAnimation.Draw(glm::vec3(1.f, 1.f, 1.f));

			if (DeadAnimation.AnimationState == AnimationPlayingState::APS_Stopped)
			{
				AllowExit = true;
			}
		}

		 if (CurrentTime - CollisionHitLastTime >= CollisionDisableTime)
			 Player.ActivateCollision();


		MainBackground.Draw(glm::vec3(1.f, 1.f, 1.f));



		if (CurrentTime - EnemySpawnLastTime > SpawnTime)
		{
			if (UnspawnedEnemies.size() >= 1)
			{

				SpawnTime = RandBetween(0.1f, 1.f);

				EnemySpawnLastTime = CurrentTime;

				const int SelectedMeteor = RandBetween(0, UnspawnedEnemies.size() - 1);
				SpawnedEnemies.push_back(UnspawnedEnemies[SelectedMeteor]);
				UnspawnedEnemies.erase(UnspawnedEnemies.begin() + SelectedMeteor);
			}
		}

		
		if (HitAnimation.AnimationState == AnimationPlayingState::APS_Playing && Player.IsAlive())
		{
			HitAnimation.PlayAnimation(CurrentTime, "./PNGs/HitAnimation/Hit");
			HitAnimation.SetModel(HitAnimation.ObjectTransformation.Scale, HitAnimation.ObjectTransformation.RotationAmount, glm::vec3(Player.ObjectTransformation.Position.r, Player.ObjectTransformation.Position.g, Player.ObjectTransformation.Position.b + 0.1f));
			HitAnimation.Draw(glm::vec3(1.f, 1.f, 1.f));
		}

		for (EnemyObject* SpawnedEnemy : SpawnedEnemies)
		{
			SpawnedEnemy->ObjectTransformation.Position.g -= SpawnedEnemy->YMovementSpeed * DeltaTime;

			SpawnedEnemy->ObjectTransformation.RotationAmount.b += SpawnedEnemy->ZRotationSpeed * DeltaTime;

			SpawnedEnemy->SetModel(SpawnedEnemy->ObjectTransformation.Scale, SpawnedEnemy->ObjectTransformation.RotationAmount, SpawnedEnemy->ObjectTransformation.Position);

			SpawnedEnemy->Draw(SpawnedEnemy->Color);

			if (SpawnedEnemy->CollisionDetection(&Player)) // collision test
			{
				Player.DisableCollision();


				Player.ReduceLife();


				CollisionHitLastTime = CurrentTime;
				HitAnimation.AnimationState = AnimationPlayingState::APS_Playing;
			}

			if (SpawnedEnemy->ObjectTransformation.Position.g <= -5.f) // out of the screen
			{
				SpawnedEnemy->ObjectTransformation.Position.g = 4.5;
				UnspawnedEnemies.push_back(SpawnedEnemy);
				auto i = std::find(SpawnedEnemies.begin(), SpawnedEnemies.end(), SpawnedEnemy);
				SpawnedEnemies.erase(std::find(SpawnedEnemies.begin(), SpawnedEnemies.end(), SpawnedEnemy));
			}
			
		}

		LastFrameTime = CurrentTime;

		SDL_GL_SwapWindow(MainWindow);
	}

	CleanUp(FragmentShader, VertexShader, MainWindow);

	return 0;
}