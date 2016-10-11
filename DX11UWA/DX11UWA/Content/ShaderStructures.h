#pragma once

namespace DX11UWA
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 camerapos;

	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionUVNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 uv;
		DirectX::XMFLOAT3 normal;
	};	

	struct VERTEX
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT2 normalmap;
		DirectX::XMFLOAT3 normals;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormals;
	};

	struct DirectionConstantBuffer
	{
		DirectX::XMFLOAT4 dir;
		DirectX::XMFLOAT4 color;
	};

	struct PointConstantBuffer
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;
	};

	struct SpotConstantBuffer
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 dir;
		DirectX::XMFLOAT4 coneratio;
	};
}