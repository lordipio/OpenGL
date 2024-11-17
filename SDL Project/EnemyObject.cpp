#include "EnemyObject.h"

void EnemyObject::SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, float ZRotationSpeed, float YMovementSpeed, glm::vec3 Color)
{
	RenderObject::SetupObject(Scale, RotationAmount, Position, TexturePath, TextureAlphaReduction, Vertices, VertexIndices, Program, Width, Height, Color);

	this->YMovementSpeed = YMovementSpeed;
	this->ZRotationSpeed = ZRotationSpeed;
}
