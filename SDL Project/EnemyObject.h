#pragma once
#include <RenderObject.h>

class EnemyObject : public RenderObject
{
public:

	void SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, float ZRotationSpeed, float YMovementSpeed, glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f));

	float YMovementSpeed;

	float ZRotationSpeed;
};