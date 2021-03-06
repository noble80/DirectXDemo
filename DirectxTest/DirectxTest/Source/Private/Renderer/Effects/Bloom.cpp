#include "stdafx.h"
#include "Renderer\Effects\Bloom.h"

#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Renderer.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\ShaderContainers.h"
#include "Renderer\ResourceManager.h"
#include "Renderer\GraphicsStructures.h"
#include <d3d11.h>
#include "Renderer\ResourceManager.h"


Bloom::Bloom()
{
	output = nullptr;
	constantBuffer = nullptr;
	maskA = nullptr;
	maskB = nullptr;
	maskShader = nullptr;
	blurShader = nullptr;
	combineShader = nullptr;
}

RenderTexture2D* Bloom::RenderEffect(Renderer * renderer, RenderTexture2D* prev)
{
	BloomBuffer* buff = static_cast<BloomBuffer*>(constantBuffer->cpu);
	BlurBuffer* blur = static_cast<BlurBuffer*>(blurBuffer->cpu);
	renderer->SetPixelShaderConstantBuffer(0, &constantBuffer->gpu.data);
	renderer->UpdateConstantBuffer(constantBuffer);
	renderer->SetRenderTargets(1, &maskA->renderTargetView, nullptr);

	renderer->SetPixelShaderResource(0, &prev->resourceView);
	renderer->SetPixelShaderResource(1, &prev->depthView);

	renderer->SetFullscreenViewport(0.5f);
	renderer->SetPixelShader(maskShader->d3dShader);
	renderer->DrawScreenQuad();

	renderer->ClearRenderTarget();

	renderer->SetPixelShader(blurShader->d3dShader);
	renderer->SetPixelShaderConstantBuffer(0, &blurBuffer->gpu.data);

	for(int i = 0; i < 10; i++)
	{
		blur->horizontal = 1;
		renderer->UpdateConstantBuffer(blurBuffer);

		renderer->ClearRenderTarget();
		renderer->SetPixelShaderResource(0, &maskA->resourceView);
		renderer->SetRenderTargets(1, &maskB->renderTargetView, nullptr);
		renderer->DrawScreenQuad();

		blur->horizontal = 0;
		renderer->UpdateConstantBuffer(blurBuffer);

		renderer->ClearRenderTarget();
		renderer->SetPixelShaderResource(0, &maskB->resourceView);
		renderer->SetRenderTargets(1, &maskA->renderTargetView, nullptr);
		renderer->DrawScreenQuad();
	}
	renderer->SetPixelShaderConstantBuffer(0, &constantBuffer->gpu.data);
	renderer->SetFullscreenViewport(1.0f);
	renderer->ClearRenderTarget();
	renderer->SetPixelShaderResource(0, &prev->resourceView);
	renderer->SetPixelShaderResource(2, &maskA->resourceView);
	renderer->SetPixelShader(combineShader->d3dShader);
	renderer->SetRenderTargets(1, &output->renderTargetView, nullptr);
	renderer->DrawScreenQuad();
	return output;
}

void Bloom::Initialize(Renderer * renderer)
{
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(renderer->GetSceneTexture()->texture);
	tex->GetDesc(&desc);
	output = renderer->LoadRenderTexture2D(&desc, "BloomOutput");
	desc.Height /= 2;
	desc.Width /= 2;
	maskA = renderer->LoadRenderTexture2D(&desc, "BloomMaskA");
	maskB = renderer->LoadRenderTexture2D(&desc, "BloomMaskB");
	if(!maskShader)
		maskShader = renderer->LoadPixelShader("BloomMask");
	if(!blurShader)
		blurShader = renderer->LoadPixelShader("Blur");
	if(!combineShader)
		combineShader = renderer->LoadPixelShader("BloomCombine");

	if(!constantBuffer)
		constantBuffer = renderer->CreateConstantBuffer(sizeof(BloomBuffer), "Bloom");
	if(!blurBuffer)
		blurBuffer = renderer->CreateConstantBuffer(sizeof(BlurBuffer), "BlurBloom");
	BloomBuffer* buff = static_cast<BloomBuffer*>(constantBuffer->cpu);
	BlurBuffer* blur = static_cast<BlurBuffer*>(blurBuffer->cpu);
	buff->threshold = .9f;
	buff->intensity = .6f;
	blur->horizontal = 1;
	blur->resolution.x = desc.Width;
	blur->resolution.y = desc.Height;
}

void Bloom::Release(Renderer* renderer)
{
	renderer->GetResourceManager()->RemoveResource(output);
	renderer->GetResourceManager()->RemoveResource(constantBuffer);
	renderer->GetResourceManager()->RemoveResource(blurBuffer);
	renderer->GetResourceManager()->RemoveResource(maskA);
	renderer->GetResourceManager()->RemoveResource(maskB);

}

void Bloom::SetIntensity(float i)
{
	BloomBuffer* buff = static_cast<BloomBuffer*>(constantBuffer->cpu);
	buff->intensity = std::clamp(i, 0.f, 10.f);
}

void Bloom::AddIntensity(float i)
{
	BloomBuffer* buff = static_cast<BloomBuffer*>(constantBuffer->cpu);
	buff->intensity = std::clamp(buff->intensity + i, 0.f, 10.f);
}
