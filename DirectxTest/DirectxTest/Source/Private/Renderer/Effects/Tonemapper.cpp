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
}

void Tonemapper::Release(Renderer* renderer)
{
	renderer->GetResourceManager()->RemoveResource(ps);
}
