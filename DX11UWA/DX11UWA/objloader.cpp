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
			//string vertex1, vertex2, vertex3;
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
			indexVerts.push_back((unsigned int)vertices.size());
			vertices.push_back(vertex);
		}
	}
	return true;
}

HRESULT Model::loadTexture(const wchar_t * path, ID3D11Device* d3dDevice)
{
	return CreateDDSTextureFromFile(d3dDevice, path, NULL, srv.GetAddressOf());
}

void Model::Render()
{
}
