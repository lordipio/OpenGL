#include "Animation.h"
#include <iostream>
#include <string>



void Animation::SetupObject(glm::vec3 Scale, glm::vec3 RotationAmount, glm::vec3 Position, std::string TexturePath, float TextureAlphaReduction, std::vector<float> Vertices, std::vector<int> VertexIndices, GLuint Program, float Width, float Height, const int NumberOfAnimationFrame, float AnimationFrameTimeInterval, glm::vec3 Color)
{
	RenderObject::SetupObject(Scale, RotationAmount, Position, TexturePath, TextureAlphaReduction, Vertices, VertexIndices, Program, Width, Height, Color);
	
	this->NumberOfAnimationFrame = NumberOfAnimationFrame;

	this->AnimationFrameTimeInterval = AnimationFrameTimeInterval;
	
}

void Animation::PlayAnimation(float CurrentTime, std::string FilePath)
{
	if (AnimationFrame > NumberOfAnimationFrame)
	{
		AnimationState = AnimationPlayingState::APS_Stopped;

		AnimationFrame = 1;
	}

	else
	{
		AnimationState = AnimationPlayingState::APS_Playing;

		if (CurrentTime - AnimationLastTime >= AnimationFrameTimeInterval)
		{
			SetTexture(FilePath + std::to_string(AnimationFrame) + ".png", 1.f);
			AnimationFrame++;
			AnimationLastTime = CurrentTime;
		}
	}
}
