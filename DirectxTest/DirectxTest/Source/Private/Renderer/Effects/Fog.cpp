#include "stdafx.h"

#include "..\..\..\Public\Renderer\Effects\Fog.h"

#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Renderer.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\ShaderContainers.h"
#include "Renderer\ResourceManager.h"
#include "Renderer\GraphicsStructures.h"
#include <d3d11.h>
#include "Renderer\ResourceManager.h"

#include "Engine\CameraComponent.h"

RenderTexture2D * Fog::RenderEffect(Renderer * renderer, RenderTexture2D * prev)
{
	FogBuffer* buff = static_cast<FogBuffer*>(constantBuffer->cpu);
	buff->farZ = renderer->GetActiveCamera()->GetFarZ();
	buff->nearZ = renderer->GetActiveCamera()->GetNearZ();

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

void Fog::Initialize(Renderer * renderer)
{
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(renderer->GetSceneTexture()->texture);
	tex->GetDesc(&desc);
	output = renderer->LoadRenderTexture2D(&desc, "FogOutput");

	ps = renderer->LoadPixelShader("Fog");

	if(!constantBuffer)
		constantBuffer = renderer->CreateConstantBuffer(sizeof(FogBuffer), "Fog");
	FogBuffer* buff = static_cast<FogBuffer*>(constantBuffer->cpu);
	buff->fogAlpha = 0.5f;
	buff->fogStart = 50.f;
	buff->fogColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	buff->fogFallOff = 800.f;
}

void Fog::Release(Renderer * renderer)
{
	renderer->GetResourceManager()->RemoveResource(output);
	renderer->GetResourceManager()->RemoveResource(constantBuffer);
}

float Fog::GetFogAlpha() const
{
	FogBuffer* buff = static_cast<FogBuffer*>(constantBuffer->cpu);
	return buff->fogAlpha;
}

void Fog::AddFogAlpha(float s)
{
	FogBuffer* buff = static_cast<FogBuffer*>(constantBuffer->cpu);
	buff->fogAlpha = std::clamp(s + buff->fogAlpha, 0.f, 1.f);
}
