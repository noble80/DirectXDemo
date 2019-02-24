#pragma once

#include "Effect.h"

struct ConstantBuffer;

struct FogBuffer
{
	DirectX::XMFLOAT3 fogColor;
	float fogAlpha;
	float nearZ;
	float farZ;
	float fogStart;
	float fogFallOff;
};

class Fog : public Effect
{
public:
	// Inherited via Effect
	virtual RenderTexture2D * RenderEffect(Renderer * renderer, RenderTexture2D * prev) override;
	virtual void Initialize(Renderer * renderer) override;
	virtual void Release(Renderer * renderer) override;

	float GetFogAlpha() const;
	void AddFogAlpha(float s);
private:
	ConstantBuffer* constantBuffer;
	PixelShader* ps;
};