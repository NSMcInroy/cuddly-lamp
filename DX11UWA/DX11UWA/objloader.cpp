#include "pch.h"
#include "objloader.h"

bool Model::loadOBJ(const char * path)
{
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<XMFLOAT3> temp_positions;
	vector<XMFLOAT2> temp_uvs;
	vector<XMFLOAT3> temp_normals;


	FILE * file;
	fopen_s(&file, path, "r");
	if (file == NULL)
		return false;
	//until we are at End Of File(EOF)
	while (true)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0)
		{
			XMFLOAT3 positon;
			fscanf_s(file, "%f %f %f\n", &positon.x, &positon.y, &positon.z);
			positon.x = -positon.x;
			temp_positions.push_back(positon);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			XMFLOAT2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = 1.0f - uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.x = -normal.x;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			//string vertices[i], vertices[i+1], vertices[i+2];
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int numMatches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (numMatches != 9)
				return false;

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[2]);
			vertexIndices.push_back(vertexIndex[1]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[2]);
			uvIndices.push_back(uvIndex[1]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[2]);
			normalIndices.push_back(normalIndex[1]);
		}

	}


	bool isUnique;
	unsigned index;
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		isUnique = true;
		DX11UWA::VERTEX vertex;
		vertex.position = temp_positions[vertexIndices[i] - 1];
		vertex.uv = temp_uvs[uvIndices[i] - 1];
		vertex.normals = temp_normals[normalIndices[i] - 1];

		for (index = 0; index < vertices.size(); ++index)
		{
			if (vertex.normals.x == vertices[index].normals.x && vertex.normals.y == vertices[index].normals.y && vertex.normals.z == vertices[index].normals.z)// if the normals are ==
				if (vertex.uv.x == vertices[index].uv.x && vertex.uv.y == vertices[index].uv.y)// if uv are equal
					if (vertex.position.x == vertices[index].position.x && vertex.position.y == vertices[index].position.y && vertex.position.z == vertices[index].position.z)
					{
						isUnique = false;
						indexVerts.push_back(index);
						break;
					}
		}
		if (isUnique)
		{
			vertex.normalmap.x = 0.0f;
			indexVerts.push_back((unsigned int)vertices.size());
			vertices.push_back(vertex);
		}
	}

	for (unsigned int i = 0; i < indexVerts .size(); i += 3)
	{
		float vector1[3], vector2[3];
		float uVector[2], vVector[2];
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;

		// Calculate the two vectors for this face.
		vector1[0] = vertices[indexVerts[i+ 1]].position.x - vertices[indexVerts[i]].position.x;
		vector1[1] = vertices[indexVerts[i+ 1]].position.y - vertices[indexVerts[i]].position.y;
		vector1[2] = vertices[indexVerts[i+ 1]].position.z - vertices[indexVerts[i]].position.z;

		vector2[0] = vertices[indexVerts[i+ 2]].position.x - vertices[indexVerts[i]].position.x;
		vector2[1] = vertices[indexVerts[i+ 2]].position.y - vertices[indexVerts[i]].position.y;
		vector2[2] = vertices[indexVerts[i+ 2]].position.z - vertices[indexVerts[i]].position.z;

		// Calculate the tu and tv texture space vectors.
		uVector[0] = vertices[indexVerts[i + 1]].uv.x - vertices[indexVerts[i]].uv.x;
		vVector[0] = vertices[indexVerts[i + 1]].uv.y - vertices[indexVerts[i]].uv.y;

		uVector[1] = vertices[indexVerts[i + 2]].uv.x - vertices[indexVerts[i]].uv.x;
		vVector[1] = vertices[indexVerts[i + 2]].uv.y - vertices[indexVerts[i]].uv.y;

		// Calculate the denominator of the tangent/binormal equation.
		float invDenominator = 1.0f / (uVector[0] * vVector[1] - uVector[1] * vVector[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
		tangent.x = (vVector[1] * vector1[0] - vVector[0] * vector2[0]) * invDenominator;
		tangent.y = (vVector[1] * vector1[1] - vVector[0] * vector2[1]) * invDenominator;
		tangent.z = (vVector[1] * vector1[2] - vVector[0] * vector2[2]) * invDenominator;

		binormal.x = (uVector[0] * vector2[0] - uVector[1] * vector1[0]) * invDenominator;
		binormal.y = (uVector[0] * vector2[1] - uVector[1] * vector1[1]) * invDenominator;
		binormal.z = (uVector[0] * vector2[2] - uVector[1] * vector1[2]) * invDenominator;

		// Calculate the length of this normal.
		float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

		// Normalize the normal and then store it
		tangent.x = tangent.x / length;
		tangent.y = tangent.y / length;
		tangent.z = tangent.z / length;

		// Calculate the length of this normal.
		length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

		// Normalize the normal and then store it
		binormal.x = binormal.x / length;
		binormal.y = binormal.y / length;
		binormal.z = binormal.z / length;

		//set the 3 vertices to the tangent and the binormals

		vertices[indexVerts[i]].tangent = tangent;
		vertices[indexVerts[i]].binormals = binormal;
		vertices[indexVerts[i+ 1]].tangent = tangent;
		vertices[indexVerts[i+ 1]].binormals = binormal;
		vertices[indexVerts[i+ 2]].tangent = tangent;
		vertices[indexVerts[i+ 2]].binormals = binormal;


	}
	return true;
}

HRESULT Model::loadTexture(const wchar_t * path, ID3D11Device* d3dDevice)
{
	return CreateDDSTextureFromFile(d3dDevice, path, NULL, srv.GetAddressOf());
}
HRESULT Model::loadNormal(const wchar_t * path, ID3D11Device* d3dDevice)
{
	return CreateDDSTextureFromFile(d3dDevice, path, NULL, normalsrv.GetAddressOf());
}

void Model::Render()
{
}
