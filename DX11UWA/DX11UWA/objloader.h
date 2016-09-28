#pragma once
#include "pch.h"
#include "Content\ShaderStructures.h"


class Model
{
public:
	vector<DX11UWA::VERTEX> vertices;
	
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	DX11UWA::ModelViewProjectionConstantBuffer	m_constantBufferData;

	//render function
	bool loadOBJ(const char * path);
	void Render();

};

