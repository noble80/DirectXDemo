#include "stdafx.h"
#include "Renderer\Effects\DOF.h"

#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Renderer.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\ShaderContainers.h"
#include "Renderer\ResourceManager.h"
#include "Renderer\GraphicsStructures.h"
#include <d3d11.h>
#include "Renderer\ResourceManager.h"
#include "Engine\CameraComponent.h"

DOF::DOF()
{
	output = nullptr;
	constantBuffer = nullptr;
	blurShader = nullptr;
	combineShader = nullptr;
	enabled = false;
}

RenderTexture2D* DOF::RenderEffect(Renderer * renderer, RenderTexture2D* prev)
{
	DOFBuffer* buff = static_cast<DOFBuffer*>(constantBuffer->cpu);
	BlurBuffer* blur = static_cast<BlurBuffer*>(blurBuffer->cpu);
	renderer->SetPixelShaderConstantBuffer(0, &blurBuffer->gpu.data);
	renderer->UpdateConstantBuffer(blurBuffer);

	renderer->ClearRenderTarget();
	renderer->SetRenderTargets(1, &blurB->renderTargetView, nullptr);
	renderer->SetPixelShaderResource(0, &prev->resourceView);
	renderer->SetPixelShaderResource(1, &prev->depthView);

	blur->horizontal = 1;
	renderer->UpdateConstantBuffer(blurBuffer);

	renderer->SetFullscreenViewport(0.5f);
	renderer->SetPixelShader(blurShader->d3dShader);
	renderer->DrawScreenQuad();
	for(int i = 0; i < 5; i++)
	{

		blur->horizontal = 0;
		renderer->UpdateConstantBuffer(blurBuffer);

		renderer->ClearRenderTarget();
		renderer->SetPixelShaderResource(0, &blurB->resourceView);
		renderer->SetRenderTargets(1, &blurA->renderTargetView, nullptr);
		renderer->DrawScreenQuad();

		blur->horizontal = 1;
		renderer->UpdateConstantBuffer(blurBuffer);

		renderer->ClearRenderTarget();
		renderer->SetPixelShaderResource(0, &blurA->resourceView);
		renderer->SetRenderTargets(1, &blurB->renderTargetView, nullptr);
		renderer->DrawScreenQuad();
	}
	buff->farZ = renderer->GetActiveCamera()->GetFarZ();
	buff->nearZ = renderer->GetActiveCamera()->GetNearZ();
	renderer->UpdateConstantBuffer(constantBuffer);
	renderer->SetPixelShaderConstantBuffer(0, &constantBuffer->gpu.data);
	renderer->SetFullscreenViewport(1.0f);
	renderer->ClearRenderTarget();
	renderer->SetPixelShaderResource(0, &prev->resourceView);
	renderer->SetPixelShaderResource(1, &prev->depthView);
	renderer->SetPixelShaderResource(2, &blurB->resourceView);
	renderer->SetPixelShader(combineShader->d3dShader);
	renderer->SetRenderTargets(1, &output->renderTargetView, nullptr);
	renderer->DrawScreenQuad();
	return output;
}

void DOF::Initialize(Renderer * renderer)
{
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(renderer->GetSceneTexture()->texture);
	tex->GetDesc(&desc);
	output = renderer->LoadRenderTexture2D(&desc, "DOFOutput");
	desc.Height /= 2;
	desc.Width /= 2;
	blurA = renderer->LoadRenderTexture2D(&desc, "DOFPassA");
	blurB = renderer->LoadRenderTexture2D(&desc, "DOFPassB");
	if(!blurShader)
		blurShader = renderer->LoadPixelShader("Blur");
	if(!combineShader)
		combineShader = renderer->LoadPixelShader("DOFCombine");

	if(!constantBuffer)
		constantBuffer = renderer->CreateConstantBuffer(sizeof(DOFBuffer), "DOF");
	if(!blurBuffer)
		blurBuffer = renderer->CreateConstantBuffer(sizeof(BlurBuffer), "BlurDOF");
	DOFBuffer* buff = static_cast<DOFBuffer*>(constantBuffer->cpu);
	BlurBuffer* blur = static_cast<BlurBuffer*>(blurBuffer->cpu);
	blur->horizontal = 1;
	blur->resolution.x = desc.Width;
	blur->resolution.y = desc.Height;
	buff->DOFStart = 20.f;
	buff->DOFFalloff = 1000.f;
}

void DOF::Release(Renderer* renderer)
{
	renderer->GetResourceManager()->RemoveResource(output);
	renderer->GetResourceManager()->RemoveResource(blurA);
	renderer->GetResourceManager()->RemoveResource(blurB);
	renderer->GetResourceManager()->RemoveResource(constantBuffer);
	renderer->GetResourceManager()->RemoveResource(blurBuffer);
}

void DOF::MoveNearPlane(float n)
{
	DOFBuffer* buff = static_cast<DOFBuffer*>(constantBuffer->cpu);
	buff->DOFStart = buff->DOFStart + n;
}
