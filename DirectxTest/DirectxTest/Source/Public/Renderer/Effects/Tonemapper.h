#pragma once

#include "Effect.h"

class Tonemapper : public Effect
{
public:
	Tonemapper();
	virtual RenderTexture2D* RenderEffect(Renderer* renderer, RenderTexture2D* prev) override;
	virtual void Initialize(Renderer* renderer) override;
	virtual void Release(Renderer* renderer) override;

private:
	PixelShader* ps;
};