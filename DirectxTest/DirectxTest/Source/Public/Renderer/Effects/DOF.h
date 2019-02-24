#pragma once

#include "Effect.h"

struct ConstantBuffer;

struct DOFBuffer
{
	float nearZ;
	float farZ;
	float DOFStart;
	float DOFFalloff;
};

class DOF : public Effect
{
public:
	DOF();
	virtual RenderTexture2D* RenderEffect(Renderer* renderer, RenderTexture2D* prev) override;
	virtual void Initialize(Renderer* renderer) override;
	virtual void Release(Renderer* renderer) override;
	void MoveNearPlane(float n);
private:
	bool on = false;
	ConstantBuffer* constantBuffer;
	ConstantBuffer* blurBuffer;
	RenderTexture2D* blurA;
	RenderTexture2D* blurB;
	PixelShader* blurShader;
	PixelShader* combineShader;
};