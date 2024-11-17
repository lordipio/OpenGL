#include "PlayerObject.h"


PlayerObject::PlayerObject()
{
}

bool PlayerObject::IsAlive()
{
	if (CurrentLife <= 0)
		return false;
	else
		return true;
}

void PlayerObject::ReduceLife()
{
	this->CurrentLife--;
	Color = glm::vec3(CurrentLife / float(Life), 0.1f, 0.1f);
}


void PlayerObject::SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, const int Life, float XSpeed, unsigned int StepsCount, glm::vec3 Color)
{
	RenderObject::SetupObject(Scale, RotationAmount, Position, TexturePath, TextureAlphaReduction, Vertices, VertexIndices, Program, Width, Height, Color);

	this->XSpeed = XSpeed;
	this->StepsCount = StepsCount;
	this->CurrentStep = StepsCount + 1;  // Player won't move at first
	this->PlayerXMovementStep = XSpeed / StepsCount;
	this->Life = Life;
	this->CurrentLife = Life;
}



void PlayerObject::MoveCharacterRight(float& PlayerXTranslation, float DeltaTime)
{
	this->ObjectTransformation.Position.r = PlayerXTranslation;

	if (CurrentStep <= StepsCount)
	{
		this->ObjectTransformation.Position.r += PlayerXMovementStep * DeltaTime;

		if (PlayerXMovementStep < 0)
			ObjectTransformation.RotationAmount.g = 180.f;

		if (PlayerXMovementStep > 0)
			ObjectTransformation.RotationAmount.g = 0.f;

		this->CurrentStep++;
	}
}
