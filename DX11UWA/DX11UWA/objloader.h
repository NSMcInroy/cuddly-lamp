#pragma once
#include "pch.h"
#include "Content\ShaderStructures.h"
#include "DDSTextureLoader.h"



class Model
{
public:
	vector<DX11UWA::VERTEX> vertices;
	vector<unsigned int> indexVerts;
	uint32	m_indexCount;
	XMMATRIX loationMatrix;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srv;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_indexBuffer;
	DX11UWA::ModelViewProjectionConstantBuffer			m_constantBufferData;

	//render function
	bool loadOBJ(const char * path);
	HRESULT loadTexture(const wchar_t * path, ID3D11Device* d3dDevice);
	void Render();

};

