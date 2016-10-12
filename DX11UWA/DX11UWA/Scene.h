#pragma once
#include "objloader.h"

class Scene
{
	
public:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	skyboxsrv;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;

	vector<Model> models;

	HRESULT loadSkyBox(const wchar_t * path, ID3D11Device* d3dDevice)
	{
		return CreateDDSTextureFromFile(d3dDevice, path, NULL, skyboxsrv.GetAddressOf());
	}
private:

};