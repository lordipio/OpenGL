#include "RenderObject.h"
#include "GLM/glm/vec4.hpp"
#include "GLM/glm/ext/matrix_transform.hpp"
#include "GLM/glm/ext/matrix_clip_space.hpp"
#include "GLM/glm/ext/scalar_constants.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"
#include <cstdlib>
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/glm/gtx/string_cast.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"



void RenderObject::SetCollisionRadius(float Radius)
{
	ObjectTransformation.CircleCollisionRadius = Radius;
	StoredCollisionRadius = Radius;
}

void RenderObject::ActivateCollision()
{
	std::cout << "Circle Collision Radius: " << this->ObjectTransformation.CircleCollisionRadius << "      Stored Collision Radius " << StoredCollisionRadius << std::endl;

	ObjectTransformation.CircleCollisionRadius = StoredCollisionRadius;

}

void RenderObject::DisableCollision()
{
	ObjectTransformation.CircleCollisionRadius = 0.f;
}

void RenderObject::SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, glm::vec3 Color)
{
	this->ObjectTransformation.Position = Position;

	this->ObjectTransformation.RotationAmount = RotationAmount;

	this->ObjectTransformation.Scale = Scale;

	this->SetProgram(Program);

	this->SetView();

	this->SetProjection(Width, Height);

	this->SetModel(Scale, RotationAmount, Position);

	this->SetTexture(TexturePath, TextureAlphaReduction);

	this->CreateBuffers(Vertices, VertexIndices);

	this->Color = Color;
}
void RenderObject::SetProgram(GLuint Program)
{
	this->Program = Program;
}

void RenderObject::SetView()
{
	glm::vec3 Eye = glm::vec3(0.f, 0.f, 3.f);

	glm::vec3 Center = glm::vec3(0.f, 0.f, 0.f);

	glm::vec3 UpVector = glm::vec3(0.f, 1.f, 0.f);

	this->View = glm::lookAt(Eye, Center, UpVector);
}

void RenderObject::SetProjection(float Width, float Height)
{
	this->Projection = glm::perspective((float)glm::radians(45.f), float(Width / Height), 0.1f, 100.f);
}

void RenderObject::SetModel(glm::vec3 Scale, glm::vec3 RotationValue, glm::vec3 Translate)
{
	ObjectTransformation.TransformationMatrix = glm::mat4(1.f);

	ObjectTransformation.TransformationMatrix = glm::translate(ObjectTransformation.TransformationMatrix, Translate);

	ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.r), glm::vec3(1, 0, 0));

	ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.g), glm::vec3(0, 1, 0));

	ObjectTransformation.TransformationMatrix = glm::rotate(ObjectTransformation.TransformationMatrix, glm::radians(RotationValue.b), glm::vec3(0, 0, 1));

	ObjectTransformation.TransformationMatrix = glm::scale(ObjectTransformation.TransformationMatrix, Scale);
}


GLuint RenderObject::SetTexture(std::string TexturePath, float AlphaReductionScale)
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

	if (nrChannels == 4)
	{
		for (int i = 0; i < width * height * 4; i += 4)
			data[i + 3] = data[i + 3] * AlphaReductionScale; // reduce alpha
	}

	GLuint ColorFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, width, height, 0, ColorFormat, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	return TextureID;
}

void RenderObject::CreateBuffers(std::vector<float> Vertices, std::vector<int> VerticesIndex)
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

void RenderObject::PreDraw(glm::vec3 Color)
	{
		// Uniform
		GLuint u_Color = glGetUniformLocation(this->Program, "u_Color");

		glUniform3f(u_Color, Color.r, Color.g, Color.b);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, TextureID);

		GLuint TextureLocation = glGetUniformLocation(this->Program, "InTexture");

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

void RenderObject::Draw(glm::vec3 Color)
{
	PreDraw(Color);

	// DRAW
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glEnableVertexAttribArray(0);

	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, TextureID);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}

bool RenderObject::CollisionDetection(RenderObject* OtherObject)
{
	if (this->ObjectTransformation.CircleCollisionRadius == 0 || OtherObject->ObjectTransformation.CircleCollisionRadius == 0)
		return false;

	DistanceBetweenCircles = std::pow((OtherObject->ObjectTransformation.Position.r - this->ObjectTransformation.Position.r), 2) + std::pow((OtherObject->ObjectTransformation.Position.g - this->ObjectTransformation.Position.g), 2);

	CollisionClosestState = std::pow((OtherObject->ObjectTransformation.CircleCollisionRadius + OtherObject->ObjectTransformation.CircleCollisionRadius), 2);

	return DistanceBetweenCircles <= CollisionClosestState + 0.01f && DistanceBetweenCircles + 0.01f >= CollisionClosestState;

}
