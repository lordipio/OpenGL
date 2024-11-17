#pragma once
#include <RenderObject.h>

class PlayerObject : public RenderObject
{
public:
	PlayerObject();

	void SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, const int Life, float XSpeed = 1.f, unsigned int StepsCount = 10, glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f));

	bool IsAlive();

	void ReduceLife();

	__forceinline int GetLife()
	{
		return this->CurrentLife;
	}

	void MoveCharacterRight(float& PlayerXTranslation, float DeltaTime);

	float PlayerXMovementStep = 0.f;



	int Life = 3;
	unsigned int CurrentStep;


private:
	unsigned int StepsCount = 10;
	float XSpeed = 1.f;
	int CurrentLife = 3;


};