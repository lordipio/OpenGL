#pragma once
#include <iostream>
#include <GLAD/glad.h>
#include <vector>
#include "GLM/glm/vec3.hpp"
#include "GLM/glm/glm.hpp"
#include "GLM/glm/mat4x4.hpp"

enum class AnimationPlayingState
{
	APS_Playing = 0,
	APS_Stopped = 1
};

struct Transformation
{
	float CircleCollisionRadius = 0.f;

	glm::vec3 Position;

	glm::vec3 RotationAmount;

	glm::vec3 Scale;

	glm::mat4 TranslationMatrix;

	glm::mat4 RotationMatrix;

	glm::mat4 ScaleMatrix;

	glm::mat4 TransformationMatrix;
};


class RenderObject
{
public:
	virtual void SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f));

	void SetCollisionRadius(float Radius);

	void ActivateCollision();

	void DisableCollision();

	void SetProgram(GLuint Program);

	void SetView();

	void SetProjection(float Width, float Height);

	void SetModel(glm::vec3 Scale, glm::vec3 RotationValue, glm::vec3 Translate);

	GLuint SetTexture(std::string TexturePath, float AlphaReductionScale);

	void CreateBuffers(std::vector<float> Vertices, std::vector<int> VerticesIndex);
	
	void PreDraw(glm::vec3 Color);
	
	void Draw(glm::vec3 Color);
	
	bool CollisionDetection(RenderObject* OtherObject);

	__forceinline GLuint GetTextureID() { return TextureID; }

	__forceinline GLuint GetVAO() { return VAO; }

	__forceinline GLuint GetVBO() { return VBO; }

	__forceinline GLuint GetIBO() { return IBO; }

	Transformation ObjectTransformation;

	glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f);

protected:

	float DistanceBetweenCircles;

	float CollisionClosestState;

	bool LastFrameCollided = false;

	bool CurrentFrameCollided = false;

	bool ObjectIsColliding = false;

	float StoredCollisionRadius;

	GLuint VAO;

	GLuint VBO;

	GLuint IBO;

	GLuint TextureID;

	GLuint Program;

	glm::mat4 Projection;

	glm::mat4 View;
};