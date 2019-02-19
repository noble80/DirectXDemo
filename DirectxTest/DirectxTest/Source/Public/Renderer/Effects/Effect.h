#pragma once

struct IDXGISwapChain1;
struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct D3D11_VIEWPORT;
struct ID3D11InputLayout;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11ShaderResourceView;

struct RenderTexture2D;
struct RenderTexture2DAllMips;
class Renderer;

struct VertexShader;
struct PixelShader;

class Effect
{
public:
	virtual RenderTexture2D* RenderEffect(Renderer* renderer, RenderTexture2D* prev) = 0;
	virtual void Initialize(Renderer* renderer) = 0;
	virtual void Release(Renderer* renderer) = 0;

protected:
	RenderTexture2D* output;
};