#pragma once

#include "Effect.h"

struct ConstantBuffer;

struct BloomBuffer
{
	DirectX::XMFLOAT2 resolution;
	int currMip;
	int horizontal;
	float threshold;
	float intensity;
	DirectX::XMFLOAT2 padding;
};

class Bloom : public Effect
{
public:
	Bloom();
	virtual RenderTexture2D* RenderEffect(Renderer* renderer, RenderTexture2D* prev) override;
	virtual void Initialize(Renderer* renderer) override;
	virtual void Release(Renderer* renderer) override;
private:
	ConstantBuffer* constantBuffer;
	RenderTexture2D* maskA;
	RenderTexture2D* maskB;
	PixelShader* maskShader;
	PixelShader* blurShader;
	PixelShader* combineShader;
};