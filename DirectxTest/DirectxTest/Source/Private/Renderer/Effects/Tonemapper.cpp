#include "stdafx.h"

#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Renderer.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\ShaderContainers.h"
#include "Renderer\ResourceManager.h"

#include <d3d11_1.h>

Tonemapper::Tonemapper()
{
	ps = nullptr;
}

RenderTexture2D* Tonemapper::RenderEffect(Renderer * renderer, RenderTexture2D* prev)
{
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	renderer->SetPixelShaderConstantBuffer(0, &constantBuffer->gpu.data);
	renderer->UpdateConstantBuffer(constantBuffer);

	renderer->SetFullscreenViewport(1.0f);
	renderer->SetRenderTargets(1, &output->renderTargetView, nullptr);

	renderer->SetPixelShader(ps->d3dShader);
	renderer->SetPixelShaderResource(0, &prev->resourceView);
	renderer->SetPixelShaderResource(1, &prev->depthView);
	renderer->DrawScreenQuad();

	return output;
}

void Tonemapper::Initialize(Renderer * renderer)
{
	output = renderer->m_FinalOutputTexture;
	if(!ps)
		ps = renderer->CreatePixelShader("Tonemapper");

	if(!constantBuffer)
		constantBuffer = renderer->CreateConstantBuffer(sizeof(TonemapperBuffer), "Tonemapper");
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	buff->exposure = 1.0f;
	buff->BWStrength = 0.0f;
}

void Tonemapper::Release(Renderer* renderer)
{
	renderer->GetResourceManager()->RemoveResource(ps);
	renderer->GetResourceManager()->RemoveResource(constantBuffer);
}

void Tonemapper::SetBWFilterStrength(float bw)
{
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	buff->BWStrength =  std::clamp(bw, 0.f, 1.0f);
}

void Tonemapper::AddBWFilterStrength(float bw)
{
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	buff->BWStrength = std::clamp(buff->BWStrength  + bw, 0.f, 1.0f);
}

void Tonemapper::SetExposure(float e)
{
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	buff->exposure = e;
}

void Tonemapper::AddExposure(float e)
{
	TonemapperBuffer* buff = static_cast<TonemapperBuffer*>(constantBuffer->cpu);
	buff->exposure += e;
}
