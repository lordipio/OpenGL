#pragma once
#include <RenderObject.h>

class Animation : public RenderObject
{
public:
	void SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, const int NumberOfAnimationFrame, float AnimationFrameTimeInterval, glm::vec3 Color = glm::vec3(1.f, 1.f, 1.f));

	/**
	 * @brief Play textures in order
	 * @param FilePath FilePath should not contain frame number and .png
	 */
	void PlayAnimation(float CurrentTime, std::string FilePath); 

	AnimationPlayingState AnimationState = AnimationPlayingState::APS_Stopped;

private:
	int AnimationFrame = 1;

	int NumberOfAnimationFrame = 10;

	float AnimationFrameTimeInterval = 0.1f;

	float AnimationLastTime = 0.f;
};