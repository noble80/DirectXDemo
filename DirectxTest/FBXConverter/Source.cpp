#include "Public\Importer\FBXImporter.h"
#include "Public\Math\MathLibrary.h"
#include "Public\StringUtility.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <conio.h>

namespace fs = std::filesystem;

using namespace std;

std::string getExtension(string& path)
{

	string::size_type i = path.rfind('.', path.length());

	return path.substr(i + 1, path.length() - i);
}

void AppendAndReplaceExtension(string& path, string app)
{

	string::size_type s = path.rfind('\\', path.length());
	string::size_type i = path.rfind('.', path.length());

	string name = path.substr(s + 1, i - s - 1) + app;

	if(i != string::npos)
	{
		path.replace(s + 1, name.length(), name);
	}
}


void ReplaceExtension(string& path, const string& ext)
{

	string::size_type i = path.rfind('.', path.length());

	if(i != string::npos)
	{
		path.replace(i + 1, ext.length(), ext);
	}
}

std::string GetFileNameFromPath(string& path)
{
	string::size_type s = path.rfind('\\', path.length());
	string::size_type i = path.rfind('.', path.length());

	return path.substr(s + 1, i - s - 1);
}

int main(int argc, const char * argv[])
{

	std::string path = "..\\Models";
	for(const auto & entry : fs::directory_iterator(path))
	{
		string filePath = entry.path().u8string();
		string ext = getExtension(filePath);

		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if(ext != "fbx")
			continue;

		std::cout << filePath << std::endl;
		std::vector<FBXImporter::Mesh> fbxmeshes;
		FBXImporter::ImportModelFromFile(entry.path().u8string(), fbxmeshes, 1.f, 0); // FBX_IMPORT_FLAG_SWAP_YZ);
		int j = 1;
		for(auto& fbxmesh : fbxmeshes)
		{
			std::string newFileName = filePath;

			if(fbxmeshes.size() > 1)
			{
				AppendAndReplaceExtension(newFileName, "_" + to_string(j) + ".mesh");
			}
			else
				ReplaceExtension(newFileName, "mesh");

			ofstream myfile;
			myfile.open(newFileName, ios::out | ios::binary | ios::trunc);

			if(myfile.is_open())
			{
				uint32_t vertCount = static_cast<uint32_t>(fbxmesh.vertices.size());
				uint32_t indexCount = static_cast<uint32_t>(fbxmesh.indices.size());
				myfile.write((char*)&vertCount, sizeof(uint32_t));
				myfile.write((char*)&indexCount, sizeof(uint32_t));
				myfile.write((char*)fbxmesh.vertices.data(), sizeof(Vertex)*vertCount);
				myfile.write((char*)fbxmesh.indices.data(), sizeof(uint32_t)*indexCount);


				myfile.close();
			}

			j++;
		}
	}

	for(const auto & entry : fs::directory_iterator(path))
	{
		string filePath = entry.path().u8string();
		string ext = getExtension(filePath);

		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if(ext != "mesh")
			continue;

		std::cout << filePath << std::endl;

		ifstream myfile;
		myfile.open(filePath, ios::in | ios::binary);

		if(myfile.is_open())
		{
			uint32_t vertCount;
			uint32_t indCount;
			string name = GetFileNameFromPath(filePath);
			FBXImporter::Mesh mesh;
			myfile.seekg(0, ios::beg);
			myfile.read((char*)&vertCount, sizeof(uint32_t));
			myfile.read((char*)&indCount, sizeof(uint32_t));
			mesh.vertices.resize(vertCount);
			mesh.indices.resize(indCount);
			myfile.read((char*)mesh.vertices.data(), sizeof(Vertex)*vertCount);
			myfile.read((char*)mesh.indices.data(), sizeof(uint32_t)*indCount);
			mesh.name = name;

			myfile.close();
		}

	}



	_getch();

	return 0;
}