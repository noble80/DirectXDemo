#include "stdafx.h"

#include "Importer/MeshImporter.h"
#include "Importer\ComplexOBJ.h"

#include <Windows.h>

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>


using namespace std;
using namespace MeshImporter;

bool MeshImporter::ImportModelFromFile(std::string name, Mesh& mesh)
{
	std::string fixedPath = "../Models/" + name + ".mesh";
	
	ifstream myfile;
	myfile.open(fixedPath, ios::in | ios::binary);

	if(myfile.is_open())
	{
		uint32_t vertCount;
		uint32_t indCount;
		myfile.seekg(0, ios::beg);
		myfile.read((char*)&vertCount, sizeof(uint32_t));
		myfile.read((char*)&indCount, sizeof(uint32_t));
		mesh.vertices.resize(vertCount);
		mesh.indices.resize(indCount);
		myfile.read((char*)mesh.vertices.data(), sizeof(Vertex)*vertCount);
		myfile.read((char*)mesh.indices.data(), sizeof(uint32_t)*indCount);
		mesh.name = name;

		myfile.close();

		return true;
	}


	return false;
}

Mesh MeshImporter::WeakImportModelFromOBJ()
{
	Mesh mesh;

	int n = ARRAYSIZE(ComplexFBX_data);

	for(int i = 0; i < n; i++)
	{
		Vertex v;
		v.Pos.x = ComplexFBX_data[i].pos[0]/12.f;
		v.Pos.y = ComplexFBX_data[i].pos[1]/12.f;
		v.Pos.z = ComplexFBX_data[i].pos[2]/12.f;

		v.Normal.x = ComplexFBX_data[i].nrm[0];
		v.Normal.y = ComplexFBX_data[i].nrm[1];
		v.Normal.z = ComplexFBX_data[i].nrm[2];

		mesh.vertices.push_back(v);
	}

	n = ARRAYSIZE(ComplexFBX_indicies);
	for(int i = 0; i < n; i++)
	{
		mesh.indices.push_back(ComplexFBX_indicies[i]);
	}

	return mesh;
}