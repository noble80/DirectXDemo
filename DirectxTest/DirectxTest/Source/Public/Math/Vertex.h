#pragma once
#include <DirectXMath.h>
#include "MathConstants.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Binormal;

	inline bool operator==(const Vertex& rhs) const
	{
		const Vertex& lhs = *this;
		
		bool out = 
				(fabsf(lhs.Pos.x	- rhs.Pos.x)	<	MathConstants::kEpsilon) &&
				(fabsf(lhs.Pos.y	- rhs.Pos.y)	<	MathConstants::kEpsilon) &&
				(fabsf(lhs.Pos.z	- rhs.Pos.z)	<	MathConstants::kEpsilon) &&
				(fabsf(lhs.Normal.x	- rhs.Normal.x)	<	MathConstants::kEpsilon) &&
				(fabsf(lhs.Normal.y - rhs.Normal.y) <	MathConstants::kEpsilon) &&
				(fabsf(lhs.Normal.z - rhs.Normal.z) <	MathConstants::kEpsilon) &&
				(fabsf(lhs.UV.x		- rhs.UV.x)		<	MathConstants::kEpsilon) &&
				(fabsf(lhs.UV.y		- rhs.UV.y)		<	MathConstants::kEpsilon) ;
		return out;
	};
};