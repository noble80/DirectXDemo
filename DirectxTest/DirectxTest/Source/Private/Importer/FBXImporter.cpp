#include "stdafx.h"

#include "Importer/FBXImporter.h"
#include <fbxsdk.h>

#include "Engine\Log.h"

using namespace std;
using namespace FBXImporter;
using namespace DirectX;

FbxAMatrix CalculateTransform(FbxNode* node);
void ProcessFbxMesh(FbxNode* Node, std::vector<Mesh>& meshes, float scale, FBX_IMPORT_FLAG flags);
void Compactify(Mesh& mesh);
void LoadUVInformation(FbxMesh* pMesh, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
void ReadTangentBinormalNormal(FbxMesh* inMesh, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const FbxAMatrix& transform);

bool FBXImporter::ImportModelFromFile(std::string path, std::vector<Mesh>& meshes, float scale, FBX_IMPORT_FLAG flags)
{
	std::string fixedPath = "../Models/" + path + ".fbx";
	// Initialize the SDK manager. This object handles all our memory management.
	FbxManager* lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	fbxsdk::FbxImporter* lImporter = fbxsdk::FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if(!lImporter->Initialize(fixedPath.c_str(), -1, lSdkManager->GetIOSettings()))
	{
		return false;
	}

	// Create a new scene so that it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported; so get rid of the importer.
	lImporter->Destroy();

	/*FbxAxisSystem directXAxisSys(FbxAxisSystem::EUpVector::eYAxis,
		FbxAxisSystem::EFrontVector::eParityEven,
		FbxAxisSystem::eRightHanded);
	directXAxisSys.ConvertScene(lScene);*/
	//FbxAxisSystem::DirectX.ConvertScene(lScene);
	// Process the scene and build DirectX Arrays

	ProcessFbxMesh(lScene->GetRootNode(), meshes, scale, flags);

	lSdkManager->Destroy();

	return true;
}


void ProcessFbxMesh(FbxNode* Node, std::vector<Mesh>& meshes, float scale, FBX_IMPORT_FLAG flags)
{

	//FBX Mesh stuff
	int childrenCount = Node->GetChildCount();

	Log::DebugConsole::DeferredMessage << "\nName:" << Node->GetName();

	for(int i = 0; i < childrenCount; i++)
	{
		FbxNode *childNode = Node->GetChild(i);
		FbxMesh *fbxMesh = childNode->GetMesh();


		if(fbxMesh != NULL)
		{
			Mesh newMesh;

			FbxAMatrix transform = CalculateTransform(childNode);

			Log::DebugConsole::DeferredMessage << "\nMesh:" << childNode->GetName();

			// Get index count from mesh
			newMesh.indices.resize(uint32_t(fbxMesh->GetPolygonVertexCount()));
			Log::DebugConsole::DeferredMessage << "\nIndice Count:" << newMesh.indices.size();

			// No need to allocate int array, FBX does for us
			int* ind = fbxMesh->GetPolygonVertices();
			std::copy(ind, ind + newMesh.indices.size(), newMesh.indices.begin());

			// Get vertex count from mesh
			newMesh.vertices.resize(fbxMesh->GetControlPointsCount());
			Log::DebugConsole::DeferredMessage << "\nVertex Count:" << newMesh.vertices.size();
			FbxVector4* vertexArray = fbxMesh->GetControlPoints();
			//================= Process Vertices ===================
			for(int j = 0; j < newMesh.vertices.size(); j++)
			{
				FbxVector4 vert = transform.MultT(fbxMesh->GetControlPointAt(j));
				newMesh.vertices[j].Pos.x = static_cast<float>(vert.mData[0]);
				newMesh.vertices[j].Pos.y = static_cast<float>(vert.mData[1]);
				newMesh.vertices[j].Pos.z = static_cast<float>(vert.mData[2]);
			}

			std::vector<Vertex> vertices2;
			vertices2.resize(newMesh.indices.size());
			LoadUVInformation(fbxMesh, vertices2, newMesh.indices);
			ReadTangentBinormalNormal(fbxMesh, vertices2, newMesh.indices, transform);
			// align (expand) vertex array and set the normals
			for(int j = 0; j < newMesh.indices.size(); j++)
			{
				vertices2[j].UV.y = 1.f - vertices2[j].UV.y;
				vertices2[j].Pos = newMesh.vertices[newMesh.indices[j]].Pos;
				newMesh.indices[j] = j;
			}
			newMesh.vertices = vertices2;

			// print out some stats
			int originalNumVertices = (int)newMesh.vertices.size();

			Compactify(newMesh);

			for(int j = 0; j < newMesh.vertices.size(); j++)
			{
				newMesh.vertices[j].Pos.x /= scale;
				newMesh.vertices[j].Pos.y /= scale;
				newMesh.vertices[j].Pos.z /= scale;
			}

			Log::DebugConsole::DeferredMessage << "\nindex count BEFORE/AFTER compaction " << newMesh.indices.size();
			Log::DebugConsole::DeferredMessage << "\nvertex count ORIGINAL (FBX source): " << originalNumVertices;
			Log::DebugConsole::DeferredMessage << "\nvertex count AFTER expansion: " << newMesh.indices.size();
			Log::DebugConsole::DeferredMessage << "\nvertex count AFTER compaction: " << newMesh.vertices.size();
			Log::DebugConsole::DeferredMessage << "\nSize reduction: " << ((newMesh.indices.size() - newMesh.vertices.size()) / (float)newMesh.indices.size())*100.00f << "%";
			Log::DebugConsole::DeferredMessage << "\nor " << (newMesh.vertices.size() / (float)newMesh.indices.size()) << " of the expanded size";

			newMesh.name = childNode->GetName();

			meshes.push_back(newMesh);
		}
		ProcessFbxMesh(childNode, meshes, scale, flags);
	}
}

void Compactify(Mesh& mesh)
{
	vector<Vertex> newV;
	for(int i = 0; i < mesh.indices.size(); i++)
	{
		auto it = find(newV.begin(), newV.end(), mesh.vertices[mesh.indices[i]]);
		if(it == newV.end())
		{
			newV.push_back(mesh.vertices[mesh.indices[i]]);
			mesh.indices[i] = static_cast<uint32_t>(newV.size() - 1);
		}
		else
		{
			mesh.indices[i] = static_cast<uint32_t>(std::distance(newV.begin(), it));
		}
	}
	mesh.vertices = newV;
}

FbxAMatrix CalculateTransform(FbxNode* node)
{

	FbxAMatrix transform;
	transform.SetIdentity();
	if(node->GetNodeAttribute())
	{
		FbxVector4 translation = node->GetGeometricTranslation(FbxNode::eSourcePivot);
		//translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
		FbxVector4 rotation = node->GetGeometricRotation(FbxNode::eSourcePivot);
		//rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
		const FbxVector4 scale = node->GetGeometricScaling(FbxNode::eSourcePivot);
		transform.SetT(translation);
		transform.SetR(rotation);
		transform.SetS(scale);
	}
	FbxAMatrix globalMatrix = node->EvaluateGlobalTransform();

	FbxAMatrix matrix = globalMatrix * transform;
	return matrix;
}

void ReadTangentBinormalNormal(FbxMesh* inMesh, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const FbxAMatrix& transform)
{
	if(inMesh->GetElementTangentCount() < 1)
	{
		throw std::exception("Invalid Tangent Number");
	}

	if(inMesh->GetElementBinormalCount() < 1)
	{
		throw std::exception("Invalid Binormal Number");
	}

	if(inMesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");
	}

	FbxGeometryElementNormal* vN = inMesh->GetElementNormal(0);
	FbxGeometryElementBinormal* vB = inMesh->GetElementBinormal(0);
	FbxGeometryElementTangent* vT = inMesh->GetElementTangent(0);

	FbxAMatrix fixedT = transform;
	fixedT.SetT(FbxVector4(0.f, 0.f, 0.f));

	FbxVector4 nor, tan, bin;

	for(int i = 0; i < vertices.size(); i++)
	{
		int j = indices[i];
		switch(vN->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
				switch(vN->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{

						nor = vN->GetDirectArray().GetAt(j).mData;
						bin = vB->GetDirectArray().GetAt(j).mData;
						tan = vT->GetDirectArray().GetAt(j).mData;
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{

						int index = vN->GetIndexArray().GetAt(j);
						nor = vN->GetDirectArray().GetAt(index).mData;
						bin = vB->GetDirectArray().GetAt(index).mData;
						tan = vT->GetDirectArray().GetAt(index).mData;
					}
					break;

					default:
						throw std::exception("Invalid Reference");
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				switch(vN->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{

						nor = vN->GetDirectArray().GetAt(i).mData;
						bin = vB->GetDirectArray().GetAt(i).mData;
						tan = vT->GetDirectArray().GetAt(i).mData;
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{

						int index = vN->GetIndexArray().GetAt(i);
						nor = vN->GetDirectArray().GetAt(index).mData;
						bin = vB->GetDirectArray().GetAt(index).mData;
						tan = vT->GetDirectArray().GetAt(index).mData;
					}
					break;

					default:
						throw std::exception("Invalid Reference");
				}
				break;
		}
		nor = fixedT.MultT(nor);
		bin = fixedT.MultT(bin);
		tan = fixedT.MultT(tan);

		vertices[i].Normal.x = static_cast<float>(nor.mData[0]);
		vertices[i].Normal.y = static_cast<float>(nor.mData[1]);
		vertices[i].Normal.z = static_cast<float>(nor.mData[2]);

		vertices[i].Binormal.x = static_cast<float>(bin.mData[0]);
		vertices[i].Binormal.y = static_cast<float>(bin.mData[1]);
		vertices[i].Binormal.z = static_cast<float>(bin.mData[2]);

		vertices[i].Tangent.x = static_cast<float>(tan.mData[0]);
		vertices[i].Tangent.y = static_cast<float>(tan.mData[1]);
		vertices[i].Tangent.z = static_cast<float>(tan.mData[2]);
	}
}

void LoadUVInformation(FbxMesh* pMesh, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	//get all UV set names
	FbxStringList lUVSetNameList;
	pMesh->GetUVSetNames(lUVSetNameList);

	//iterating over all uv sets
	for(int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
	{
		//get lUVSetIndex-th uv set
		const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

		if(!lUVElement)
			continue;

		// only support mapping mode eByPolygonVertex and eByControlPoint
		if(lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return;

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int lPolyCount = pMesh->GetPolygonCount();

		if(lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for(int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for(int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					FbxVector2 lUVValue;

					//get the index of the current vertex in control points array
					int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					//User TODO:
					//Print out the value of UV(lUVValue) or log it to a file
					vertices[lVertIndex].UV.x = static_cast<float>(lUVValue[0]);
					vertices[lVertIndex].UV.y = static_cast<float>(lUVValue[1]);
				}
			}
		}
		else if(lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for(int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for(int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					if(lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						//User TODO:
						//Print out the value of UV(lUVValue) or log it to a file
						vertices[lPolyIndexCounter].UV.x = static_cast<float>(lUVValue[0]);
						vertices[lPolyIndexCounter].UV.y = static_cast<float>(lUVValue[1]);

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}