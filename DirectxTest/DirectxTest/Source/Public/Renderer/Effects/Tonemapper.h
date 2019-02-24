#pragma once

#include "Effect.h"

struct ConstantBuffer;

struct TonemapperBuffer
{
	float exposure;
	float BWStrength;
	float time;
	int warp;
};

class Tonemapper : public Effect
{
public:
	Tonemapper();
	virtual RenderTexture2D* RenderEffect(Renderer* renderer, RenderTexture2D* prev) override;
	virtual void Initialize(Renderer* renderer) override;
	virtual void Release(Renderer* renderer) override;

	void SetBWFilterStrength(float bw);
	void ToggleWarp();
	void AddBWFilterStrength(float bw);
	void SetExposure(float e);
	void AddExposure(float e);
private:
	ConstantBuffer* constantBuffer;
	PixelShader* ps;
};