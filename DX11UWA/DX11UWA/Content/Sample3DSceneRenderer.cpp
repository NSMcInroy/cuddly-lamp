#include "pch.h"
#include "Sample3DSceneRenderer.h"


#include "..\Common\DirectXHelper.h"


//#define DBOUT( s )            \
//{                             \
//   std::ostringstream os_;    \
//   os_ << s;                   \
//   OutputDebugStringA( os_.str().c_str() );  \
//}
//#include <Windows.h>
//#include <iostream>
//#include <sstream>


using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;
// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));
	memset(&m_camera1, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / (outputSize.Height *0.5f);
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	static const XMVECTORF32 eye1 = { 0.0f, 6.5f, -10.0f, 0.0f };
	static const XMVECTORF32 at1 = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up1 = { 0.0f, 10.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_camera1, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye1, at1, up1)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}



	// Update or move camera here
	UpdateCamera(timer, 4.0f, 1.75f);
	XMFLOAT4X4 tmp;
	XMStoreFloat4x4(&tmp, m_scene.models[BulbID].loationMatrix);
	m_pointConstantBufferData.pos.x = tmp.m[0][3];
	m_pointConstantBufferData.pos.y = tmp.m[1][3];
	m_pointConstantBufferData.pos.z = tmp.m[2][3];


	XMStoreFloat4x4(&tmp, m_scene.models[PlanetID].loationMatrix);

	m_dirConstantBufferData.dir.x = -tmp.m[0][3];
	m_dirConstantBufferData.dir.y = -tmp.m[1][3];
	m_dirConstantBufferData.dir.z = -tmp.m[2][3];

	XMStoreFloat4x4(&tmp, m_scene.models[LaternID].loationMatrix);
	m_spotConstantBufferData.pos.x = tmp.m[0][3];
	m_spotConstantBufferData.pos.y = tmp.m[1][3];
	m_spotConstantBufferData.pos.z = tmp.m[2][3];
	m_spotConstantBufferData.dir.x = -tmp.m[0][3];
	m_spotConstantBufferData.dir.y = -tmp.m[1][3];
	m_spotConstantBufferData.dir.z = -tmp.m[2][3];
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0, 3, 5), XMMatrixRotationY(radians))));
	m_scene.models[BulbID].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0, 1, -5), XMMatrixRotationY(radians)));
	m_scene.models[PlanetID].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0, 15, 0), XMMatrixRotationZ(radians)));
	m_scene.models[LaternID].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0, 2, 7), XMMatrixRotationY(-radians)));


}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	//top camera
	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}




	//second camera
	if (m_kbuttons[VK_UP])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera1);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera1, result);
	}
	if (m_kbuttons[VK_DOWN])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera1);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera1, result);


	}
	if (m_kbuttons[VK_RIGHT])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera1);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera1, result);


	}
	if (m_kbuttons[VK_LEFT])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera1);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera1, result);

	}



	if (m_currMousePos)
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;

		}
		m_prevMousePos = m_currMousePos;
	}

	m_constantBufferData.camerapos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);
	m_scene.models[SkyBoxID].loationMatrix = XMMatrixTranspose(XMMatrixSet(
		1.0, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m_camera._41, m_camera._42, m_camera._43, 1.0f));
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}
void Sample3DSceneRenderer::Draw(void)
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);


	//draw all models in scene
	for (unsigned int i = 0; i < m_scene.models.size(); ++i)
	{
		m_scene.models[i].m_constantBufferData = m_constantBufferData;

		XMStoreFloat4x4(&m_scene.models[i].m_constantBufferData.model, m_scene.models[i].loationMatrix);
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_scene.models[i].m_constantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		stride = sizeof(VERTEX);
		offset = 0;
		context->IASetVertexBuffers(0, 1, m_scene.models[i].m_vertexBuffer.GetAddressOf(), &stride, &offset);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(m_scene.models[i].m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(m_scene.m_inputLayout.Get());
		if (m_scene.models[i].vertices[0].skybox.x == 1.0f)//is a skybox
		{
			context->PSSetShaderResources(2, 1, m_scene.skyboxsrv.GetAddressOf());

		}
		else
		{
			context->PSSetShaderResources(0, 1, m_scene.models[i].srv.GetAddressOf());
			context->PSSetShaderResources(1, 1, m_scene.models[i].normalsrv.GetAddressOf());
		}
		// Attach our vertex shader.
		context->VSSetShader(m_scene.m_vertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(m_scene.m_pixelShader.Get(), nullptr, 0);


		//Lighting
		//Directional
		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource1(m_dirConstantBuffer.Get(), 0, NULL, &m_dirConstantBufferData, 0, 0, 0);
		// Each vertex is one instance of the VertexPositionColor struct.
		context->PSSetConstantBuffers1(0, 1, m_dirConstantBuffer.GetAddressOf(), nullptr, nullptr);

		//point
		context->UpdateSubresource1(m_pointConstantBuffer.Get(), 0, NULL, &m_pointConstantBufferData, 0, 0, 0);
		context->PSSetConstantBuffers1(1, 1, m_pointConstantBuffer.GetAddressOf(), nullptr, nullptr);

		//spot light
		context->UpdateSubresource1(m_spotConstantBuffer.Get(), 0, NULL, &m_spotConstantBufferData, 0, 0, 0);
		context->PSSetConstantBuffers1(2, 1, m_spotConstantBuffer.GetAddressOf(), nullptr, nullptr);

		// Draw the objects.
		context->DrawIndexed(m_scene.models[i].m_indexCount, 0, 0);
	}
}
// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}
	auto context = m_deviceResources->GetD3DDeviceContext();


	context->OMSetRenderTargets(1, m_scene.renderTargetViewMap.GetAddressOf(), m_scene.depthStencil.Get());
	context->ClearDepthStencilView(m_scene.depthStencil.Get(), D3D11_CLEAR_DEPTH, 1, NULL);
	//Draw first screen on texture
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	context->RSSetViewports(1, &m_deviceResources->GetScreenViewport1());
	Draw();


	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	//Draw First Screen
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	context->RSSetViewports(1, &m_deviceResources->GetScreenViewport1());
	Draw();

	//move second screen skybox
	m_scene.models[SkyBoxID].loationMatrix = XMMatrixTranspose(XMMatrixSet(
		1.0, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m_camera1._41, m_camera1._42, m_camera1._43, 1.0f));

	//Draw second screen
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera1))));
	context->RSSetViewports(1, &m_deviceResources->GetScreenViewport());
	Draw();


}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});





	//Models
	// Load shaders asynchronously.
	auto loadModelVSTask = DX::ReadDataAsync(L"ModelVertexShader.cso");
	auto loadModelPSTask = DX::ReadDataAsync(L"ModelPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createSceneVSTask = loadModelVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_scene.m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SKYBOX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMALMAP", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_scene.m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createScenePSTask = loadModelPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_scene.m_pixelShader));
	});

	auto createLightsTask = createScenePSTask.then([this]()
	{
		//dir
		CD3D11_BUFFER_DESC constantDirectionLightBufferDesc(sizeof(DirectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantDirectionLightBufferDesc, nullptr, &m_dirConstantBuffer));


		//m_dirConstantBufferData.color = XMFLOAT4(1, 1, 1, 1);
		m_dirConstantBufferData.color = XMFLOAT4(0.3f, 0.3f, 0.3f, 1);
		m_dirConstantBufferData.dir = XMFLOAT4(1, -1, 0, 0);

		//point
		CD3D11_BUFFER_DESC constantPointLightBufferDesc(sizeof(PointConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantPointLightBufferDesc, nullptr, &m_pointConstantBuffer));


		m_pointConstantBufferData.color = XMFLOAT4(1, 1, 0.005, 1);
		m_pointConstantBufferData.pos = XMFLOAT4(0, 1, -1, 0);

		//spot
		CD3D11_BUFFER_DESC constantSpotLightBufferDesc(sizeof(SpotConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantSpotLightBufferDesc, nullptr, &m_spotConstantBuffer));


		m_spotConstantBufferData.color = XMFLOAT4(1, 0.05, 0.05, 1);
		m_spotConstantBufferData.pos = XMFLOAT4(0, 3, 0, 1);
		m_spotConstantBufferData.dir = XMFLOAT4(0, -1, 0, 0);
		m_spotConstantBufferData.coneratio = XMFLOAT4(0.8, 0.7, 10, 1);
	});

	// Once both shaders are loaded, create the mesh.
	auto createTreeTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/WillowTree.obj"))
		{
			HRESULT hs = model.loadTexture(L"assets/WillowTexture.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.srv = nullptr;
			hs = model.loadNormal(L"assets/WillowNormal.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.normalsrv = nullptr;
			else
				for (unsigned int i = 0; i < model.vertices.size(); ++i)
					model.vertices[i].normalmap.x = 1.0f;



			m_scene.models.push_back(model);
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(3.14159f), XMMatrixTranslation(0.0f, 0.05f, 0.0f)));

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});

	// Once the cube is loaded, the object is ready to be rendered.
	auto createFloorTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/floor plane.obj"))
		{
			HRESULT hs = model.loadTexture(L"assets/TileTexture.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.srv = nullptr;
			hs = model.loadNormal(L"assets/TileNormal.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.normalsrv = nullptr;
			else
				for (unsigned int i = 0; i < model.vertices.size(); ++i)
					model.vertices[i].normalmap.x = 1.0f;


			m_scene.models.push_back(model);
			RTTID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0.0f, 0.0f, 0.0f), XMMatrixRotationY(3.14159f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});
	auto createWallTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/floor plane.obj"))
		{

			model.srv = m_scene.shaderResourceViewMap;

			model.normalsrv = nullptr;

			for (unsigned int i = 0; i < model.vertices.size(); ++i)
			{
				model.vertices[i].skybox.y = 1.0f;
			}

			m_scene.models.push_back(model);
			RTTID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationX(-1.5708f),  XMMatrixTranslation(0.0f, 10.0f, 13.0f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});
	auto createBulbTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/bulb.obj"))
		{
			HRESULT hs = model.loadTexture(L"assets/bulbon.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.srv = nullptr;


			model.normalsrv = nullptr;


			m_scene.models.push_back(model);
			BulbID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(3.14159f), XMMatrixTranslation(0.0f, 1.0f, 1.0f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});
	auto createPlanetTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/planet.obj"))
		{
			HRESULT hs = model.loadTexture(L"assets/PlanetTexture.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.srv = nullptr;


			hs = model.loadNormal(L"assets/PlanetNormal.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.normalsrv = nullptr;
			else
				for (unsigned int i = 0; i < model.vertices.size(); ++i)
					model.vertices[i].normalmap.x = 1.0f;



			m_scene.models.push_back(model);
			PlanetID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(3.14159f), XMMatrixTranslation(0.0f, 1.0f, 1.0f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});

	auto createLaternTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/latern.obj"))
		{
			HRESULT hs = model.loadTexture(L"assets/LaternTexture.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				model.srv = nullptr;

			model.normalsrv = nullptr;

			m_scene.models.push_back(model);
			LaternID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(3.14159f), XMMatrixTranslation(5.0f, 1.0f, 1.0f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});

	auto createRTTTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		///////////////////////// Map's Texture
		// Initialize the  texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the texture description.
		// We will have our map be a square
		// We will need to have this texture bound as a render target AND a shader resource
		textureDesc.Width = m_deviceResources->GetOutputSize().Width;
		textureDesc.Height = m_deviceResources->GetOutputSize().Height / 2;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the texture
		m_deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc, NULL, &m_scene.renderTargetTextureMap);


		/////////////////////// Map's Render Target
		// Setup the description of the render target view.
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		m_deviceResources->GetD3DDevice()->CreateRenderTargetView(m_scene.renderTargetTextureMap.Get(), &renderTargetViewDesc, &m_scene.renderTargetViewMap);


		/////////////////////// Map's Shader Resource View
		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView(m_scene.renderTargetTextureMap.Get(), &shaderResourceViewDesc, &m_scene.shaderResourceViewMap);


		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			lround(m_deviceResources->GetOutputSize().Width),
			lround(m_deviceResources->GetOutputSize().Height / 2),
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				&m_scene.depthTargetTextureMap
			)
		);

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateDepthStencilView(
				m_scene.depthTargetTextureMap.Get(),
				&depthStencilViewDesc,
				&m_scene.depthStencil
			)
		);
	});
	auto createSkyBoxTask = (createScenePSTask && createSceneVSTask).then([this]()
	{
		Model model;
		if (model.loadOBJ("assets/SkyBox.obj"))
		{
			HRESULT hs = m_scene.loadSkyBox(L"assets/SkyboxOcean.dds", m_deviceResources->GetD3DDevice());
			if (hs != S_OK)
				m_scene.skyboxsrv = nullptr;

			model.srv = nullptr;
			model.normalsrv = nullptr;
			for (unsigned int i = 0; i < model.vertices.size(); ++i)
			{
				model.vertices[i].skybox.x = 1.0f;
				model.vertices[i].normalmap.x = 0.0f;
			}

			m_scene.models.push_back(model);
			SkyBoxID = m_scene.models.size() - 1;
			m_scene.models[m_scene.models.size() - 1].loationMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(3.14159f), XMMatrixTranslation(5.0f, 1.0f, 0.0f)));


			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].vertices.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * m_scene.models[m_scene.models.size() - 1].vertices.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_scene.models[m_scene.models.size() - 1].m_vertexBuffer));


			m_scene.models[m_scene.models.size() - 1].m_indexCount = m_scene.models[m_scene.models.size() - 1].indexVerts.size();

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = m_scene.models[m_scene.models.size() - 1].indexVerts.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * m_scene.models[m_scene.models.size() - 1].indexVerts.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_scene.models[m_scene.models.size() - 1].m_indexBuffer));
		}
	});


	// Once the cube is loaded, the object is ready to be rendered.
	(createCubeTask && createFloorTask && createRTTTask && createLaternTask && createBulbTask &&  createPlanetTask && createSkyBoxTask && createTreeTask && createLightsTask).then([this]()
	{
		m_loadingComplete = true;
	});


}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}