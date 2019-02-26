#include "stdafx.h"
#include "Renderer\Window.h"

#include "Input\CoreInput.h"
#include "Renderer\Renderer.h"
#include "Engine\SceneManager.h"
#include "Renderer\ResourceManager.h"

#include "Renderer\Effects\Tonemapper.h"
#include "Renderer\Effects\Bloom.h"
#include "Renderer\Effects\DOF.h"
#include "Renderer\Effects\Fog.h"
#include <DirectXColors.h>

#include <windowsx.h>
#include "XTime.h"
#include <string>

#include <time.h>

#include "Renderer\GraphicsStructures.h"
#include "Renderer\Material.h"
#include "Renderer\Texture2D.h"
#include "Renderer\RenderTexture2D.h"
#include "Renderer\Mesh.h"
#include "Renderer\ShaderBuffers.h"
#include "Importer\MeshImporter.h"
#include "Renderer\ShaderContainers.h"

#include "Engine\TransformComponent.h"
#include "Engine\DirectionalLightComponent.h"
#include "Engine\SpotLightComponent.h"
#include "Engine\PointLightComponent.h"
#include "Engine\MeshComponent.h"
#include "Engine\InstancedMeshComponent.h"
#include "Engine\CameraComponent.h"

#include "DebugMemory.h"
#include "Engine\Log.h"

#include "Renderer\DebugHelpers.h"
#include <iostream>
#include <d3d11_1.h>
#include <d3d11.h>

using namespace DirectX;
using namespace MathLibrary;

#define WIDTH 1280
#define HEIGHT 720

//Windows version of main. WINAPI reverses order of parameters
int WINAPI WinMain(
	HINSTANCE hInstance, //ptr to current instance of app
	HINSTANCE hPrevInstance, //prev instance of app. Is always null nowadays.
	LPSTR lpCmdLine, // command line of app excluding program name
	int nCmdShow // how the windows is shown. Legacy. Can ignore
)
{
	//Check for memory leaks
	ENABLE_LEAK_DETECTION();

	// this function call will set a breakpoint at the location of a leaked block
	// set the parameter to the identifier for a leaked block
	//_CrtSetBreakAlloc(50844);

	Log::DebugConsole::Initialize();

	Window* window = new Window;
	SceneManager* sceneManager = new SceneManager;
	Renderer* renderer = new Renderer;

	window->Initialize(Vector2(WIDTH, HEIGHT), 0, L"DirectXTest");
	renderer->Initialize(window);

	//Load up meshes
	{
		std::string meshNames[] = {"Tree02", "Tree01","GodTree", "Sphere01", "Rock01", "RectangularBillboard", "Flag01_1", "Flag01_2"};
		int n = ARRAYSIZE(meshNames);
		for(int i = 0; i < n; ++i)
		{
			MeshImporter::Mesh mesh;
			MeshImporter::ImportModelFromFile(meshNames[i], mesh); // FBX_IMPORT_FLAG_SWAP_YZ);
			int j = 1;
			renderer->CreateGeometryBuffer(mesh.name, &mesh.vertices, mesh.indices);
		}
		//Create materials
		D3D11_INPUT_ELEMENT_DESC vLayout[] =
		{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

				//{ "INSTANCERAND", 0, DXGI_FORMAT_R32_FLOAT,   1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		VertexShader* instancedVS = renderer->LoadVertexShaderCustomLayout("DefaultInstanced", vLayout, 5);


		VertexShader* defaultVS = renderer->LoadVertexShader("Default");
		PixelShader* defaultPS = renderer->LoadPixelShader("BlinnPhong");
		PixelShader* pbrPS = renderer->LoadPixelShader("PBR");

		Material* simpleColorMat = renderer->LoadMaterial("SimpleColor");
		simpleColorMat->vertexShader = defaultVS;
		simpleColorMat->pixelShader = defaultPS;
		simpleColorMat->surfaceParameters.diffuseColor = XMFLOAT3(0.5, 0.5f, 0.5f);

		Material* rockMat = renderer->LoadMaterial("Rock01");
		rockMat->vertexShader = instancedVS;
		rockMat->pixelShader = pbrPS;
		rockMat->diffuseMap = renderer->LoadTexture("Rock01_LP_albedo");
		rockMat->normalMap = renderer->LoadTexture("Rock01_LP_normal");
		rockMat->detailsMap = renderer->LoadTexture("Rock01_LP_details");
		rockMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		rockMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		rockMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		rockMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_HAS_DETAILS_MAP | SURFACE_FLAG_HAS_NORMAL_MAP;
		rockMat->surfaceParameters.roughness = 1.0f;
		rockMat->surfaceParameters.metallic = 1.0f;
		rockMat->surfaceParameters.diffuseColor = XMFLOAT3(2.0f, 2.0f, 2.0f);

		Material* poleMat = renderer->LoadMaterial("Pole01");
		poleMat->vertexShader = defaultVS;
		poleMat->pixelShader = pbrPS;
		poleMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		poleMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		poleMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		poleMat->surfaceParameters.roughness = 0.0f;
		poleMat->surfaceParameters.metallic = 1.0f;
		poleMat->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);

		Material* godTreeMatGlow = renderer->LoadMaterial("GodTreeGlow");
		godTreeMatGlow->vertexShader = defaultVS;
		godTreeMatGlow->pixelShader = renderer->LoadPixelShader("Bamboozled");
		godTreeMatGlow->diffuseMap = renderer->LoadTexture("GodTree_Diffuse");
		godTreeMatGlow->normalMap = renderer->LoadTexture("GodTree_Normal");
		godTreeMatGlow->glowMap = renderer->LoadTexture("GodTree_Glow");
		godTreeMatGlow->detailsMap = renderer->LoadTexture("GodTree_Details");
		godTreeMatGlow->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		godTreeMatGlow->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		godTreeMatGlow->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		godTreeMatGlow->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_HAS_NORMAL_MAP | SURFACE_FLAG_HAS_DETAILS_MAP;
		godTreeMatGlow->surfaceParameters.roughness = 1.0f;
		godTreeMatGlow->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		godTreeMatGlow->surfaceParameters.metallic = 1.0f;

		Material* godTreeMat = renderer->LoadMaterial("GodTree");
		godTreeMat->vertexShader = defaultVS;
		godTreeMat->pixelShader = pbrPS;
		godTreeMat->diffuseMap = renderer->LoadTexture("GodTree_Diffuse");
		godTreeMat->normalMap = renderer->LoadTexture("GodTree_Normal");
		godTreeMat->glowMap = renderer->LoadTexture("GodTree_Glow");
		godTreeMat->detailsMap = renderer->LoadTexture("GodTree_Details");
		godTreeMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		godTreeMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		godTreeMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		godTreeMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_HAS_NORMAL_MAP | SURFACE_FLAG_HAS_DETAILS_MAP;
		godTreeMat->surfaceParameters.roughness = 1.0f;
		godTreeMat->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		godTreeMat->surfaceParameters.metallic = 1.0f;

		Material* treeMat = renderer->LoadMaterial("Tree01");
		treeMat->vertexShader = instancedVS;
		treeMat->pixelShader = pbrPS;
		treeMat->diffuseMap = renderer->LoadTexture("TreeCM");
		treeMat->normalMap = renderer->LoadTexture("TreeNM");
		treeMat->detailsMap = renderer->LoadTexture("TreeDM");
		treeMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		treeMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		treeMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		treeMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_HAS_NORMAL_MAP | SURFACE_FLAG_HAS_DETAILS_MAP | SURFACE_FLAG_IS_MASKED;
		treeMat->surfaceParameters.roughness = 1.0f;
		treeMat->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		treeMat->surfaceParameters.metallic = 1.0f;

		Material* treeMat2 = renderer->LoadMaterial("Tree02");
		treeMat2->vertexShader = instancedVS;
		treeMat2->pixelShader = pbrPS;
		treeMat2->diffuseMap = renderer->LoadTexture("Tree2CM");
		treeMat2->normalMap = renderer->LoadTexture("Tree2NM");
		treeMat2->detailsMap = renderer->LoadTexture("Tree2DM");
		treeMat2->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		treeMat2->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		treeMat2->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		treeMat2->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_HAS_NORMAL_MAP | SURFACE_FLAG_HAS_DETAILS_MAP;
		treeMat2->surfaceParameters.roughness = 1.0f;
		treeMat2->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		treeMat2->surfaceParameters.metallic = 1.0f;

		Material* flagMat = renderer->LoadMaterial("Flag");
		flagMat->vertexShader = renderer->LoadVertexShader("Flag");
		flagMat->pixelShader = defaultPS;
		flagMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP;
		{
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = 1024;
			desc.Height = 1024;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			flagMat->diffuseMap = renderer->LoadRenderTexture2D(&desc, "FlagCapture");
		}

		Material* skyMat = renderer->LoadMaterial("Sky");
		skyMat->vertexShader = renderer->LoadVertexShader("Sky");
		skyMat->pixelShader = renderer->LoadPixelShader("Sky");
		skyMat->diffuseMap = renderer->LoadTexture("IBLTestEnvHDR");

		Material* cloudTransparentMat = renderer->LoadMaterial("cloudTest");
		cloudTransparentMat->vertexShader = renderer->LoadVertexShader("Billboard");
		cloudTransparentMat->pixelShader = pbrPS;
		cloudTransparentMat->diffuseMap = renderer->LoadTexture("Cloud");
		cloudTransparentMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		cloudTransparentMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		cloudTransparentMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		cloudTransparentMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_IS_MASKED;
		cloudTransparentMat->surfaceParameters.roughness = 0.7f;
		cloudTransparentMat->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		cloudTransparentMat->surfaceParameters.metallic = 0.0f;

		Material* cloudCutoutMat = renderer->LoadMaterial("cloudTest");
		cloudCutoutMat->vertexShader = renderer->LoadVertexShader("Billboard");
		cloudCutoutMat->pixelShader = pbrPS;
		cloudCutoutMat->diffuseMap = renderer->LoadTexture("Cloud");
		cloudCutoutMat->IBLDiffuse = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestDiffuseHDR");
		cloudCutoutMat->IBLSpecular = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestSpecularHDR");
		cloudCutoutMat->IBLIntegration = renderer->GetResourceManager()->GetResource<Texture2D>("IBLTestBrdf");
		cloudCutoutMat->surfaceParameters.textureFlags = SURFACE_FLAG_HAS_DIFFUSE_MAP | SURFACE_FLAG_IS_TRANSLUSCENT | SURFACE_FLAG_IS_UNLIT;
		cloudCutoutMat->surfaceParameters.roughness = 0.7f;
		cloudCutoutMat->surfaceParameters.diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		cloudCutoutMat->surfaceParameters.metallic = 0.0f;

		// Create meshes
		Mesh* skySphere = renderer->GetResourceManager()->CreateResource<Mesh>("SkySphere");
		skySphere->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Sphere01");
		skySphere->material = skyMat;

		Mesh* cloud1 = renderer->GetResourceManager()->CreateResource<Mesh>("cloudTransparent");
		cloud1->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("RectangularBillboard");
		cloud1->material = cloudTransparentMat;

		Mesh* cloud2 = renderer->GetResourceManager()->CreateResource<Mesh>("cloudCutout");
		cloud2->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("RectangularBillboard");
		cloud2->material = cloudCutoutMat;

		Mesh* Tree01 = renderer->GetResourceManager()->CreateResource<Mesh>("Tree01");
		Tree01->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Tree01");
		Tree01->material = treeMat;

		Mesh* Tree02 = renderer->GetResourceManager()->CreateResource<Mesh>("Tree02");
		Tree02->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Tree02");
		Tree02->material = treeMat2;

		Mesh* rock = renderer->GetResourceManager()->CreateResource<Mesh>("Rock01");
		rock->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Rock01");
		rock->material = rockMat;

		Mesh* godTree = renderer->GetResourceManager()->CreateResource<Mesh>("GodTree");
		godTree->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("GodTree");
		godTree->material = godTreeMat;

		Mesh* flagPole = renderer->GetResourceManager()->CreateResource<Mesh>("FlagPole");
		flagPole->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Flag01_2");
		flagPole->material = poleMat;

		Mesh* flagTop = renderer->GetResourceManager()->CreateResource<Mesh>("FlagTop");
		flagTop->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Flag01_1");
		flagTop->material = flagMat;
	}

	// Entity creation
	Entity* cameraEntity = sceneManager->CreateEntity("Camera01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(cameraEntity);
		transform->SetRotation(Quaternion::FromAngles(15.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(-870.f, 608.86f, -1035.f, 1.f));

		CameraComponent* camera = sceneManager->CreateComponent<CameraComponent>(cameraEntity);
		Vector2 dimensions = window->GetDimensions();
		camera->SetProjectionMatrix(90.f, dimensions, 0.1f, 8000.0f);
	}

	Entity* secondaryCamera = sceneManager->CreateEntity("Camera02");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(secondaryCamera);
		transform->SetRotation(Quaternion::FromAngles(90.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(-870.f, 1000.f, -1035.f, 1.f));

		CameraComponent* camera = sceneManager->CreateComponent<CameraComponent>(secondaryCamera);
		Vector2 dimensions = window->GetDimensions();
		dimensions.x /= 4.f;
		dimensions.y /= 4.f;
		camera->SetProjectionMatrix(90.f, dimensions, 0.1f, 8000.0f);
	}

	{
		TransformComponent* ct = cameraEntity->GetComponent<TransformComponent>();
		Entity* entity = sceneManager->CreateEntity("Rock01");
		InstancedMeshComponent* instanced = sceneManager->CreateComponent<InstancedMeshComponent>(entity);
		instanced->CreateInstanceBuffer(renderer);
		Mesh* mesh = renderer->GetResourceManager()->GetResource<Mesh>("Rock01");
		instanced->SetMesh(mesh);
		Transform t;
		t.pos = ct->GetPosition();
		t.scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		t.rot = Quaternion();
		instanced->SetTransform(t);
		instanced->SetInstanceCount(20);
		instanced->RandomizeInstanceTransforms(200.f, 180.f, 180.f, 90.f, XMFLOAT2(0.08f, 0.15f));
	}

	Entity* godTree = sceneManager->CreateEntity("GodTree");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(godTree);
		transform->SetRotation(Quaternion::FromAngles(0.f, 130.f, 0.f));
		transform->SetPosition(XMVectorSet(-870.f, 561.86f, -950.f, 1.f));
		transform->SetScale(XMVectorSet(12.f, 12.f, 12.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(godTree);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("GodTree"));

	}

	{
		TransformComponent* ct = cameraEntity->GetComponent<TransformComponent>();
		Entity* entity = sceneManager->CreateEntity("TreeInstanced");
		InstancedMeshComponent* instanced = sceneManager->CreateComponent<InstancedMeshComponent>(entity);
		instanced->CreateInstanceBuffer(renderer);
		Mesh* mesh = renderer->GetResourceManager()->GetResource<Mesh>("Tree01");
		instanced->SetMesh(mesh);
		Transform t;
		t.pos = ct->GetPosition();
		t.scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		t.rot = Quaternion();
		instanced->SetTransform(t);
		instanced->SetInstanceCount(15);
		instanced->RandomizeInstanceTransforms(200.f, 0.f, 180.f, 0.f, XMFLOAT2(0.7f, 1.5f));
	}

	{
		TransformComponent* ct = cameraEntity->GetComponent<TransformComponent>();
		Entity* entity = sceneManager->CreateEntity("TreeInstanced2");
		InstancedMeshComponent* instanced = sceneManager->CreateComponent<InstancedMeshComponent>(entity);
		instanced->CreateInstanceBuffer(renderer);
		Mesh* mesh = renderer->GetResourceManager()->GetResource<Mesh>("Tree02");
		instanced->SetMesh(mesh);
		Transform t;
		t.pos = ct->GetPosition();
		t.scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		t.rot = Quaternion();
		instanced->SetTransform(t);
		instanced->SetInstanceCount(100);
		instanced->RandomizeInstanceTransforms(500.f, 0.f, 180.f, 0.f, XMFLOAT2(1.7f, 3.f));
	}

	std::vector<TransformComponent*> clouds;
	srand((unsigned int)time(NULL));
	for(int i = 0; i < 40; ++i)
	{
		Entity* entity = sceneManager->CreateEntity("Cloud" + std::to_string(i));
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 0.f, 0.f));

		Vector4 center = XMVectorSet(-870.f, 608.86f, -1035.f, 1.f);
		float distance = rand() / (float)RAND_MAX * 50.f + 8000.f;
		float scale = distance / 100.f + rand() / (float)RAND_MAX * 10.f;
		float pitch = MathLibrary::lerp(0.f, -10.f, rand() / (float)RAND_MAX);
		float yaw = MathLibrary::lerp(0.f, 360.f, rand() / (float)RAND_MAX);

		Vector4 dir = XMVector3Normalize(Quaternion::FromAngles(pitch, yaw, 0.f).GetForwardVector());
		Vector4 pos = center + dir * distance;

		transform->SetPosition(pos);
		transform->SetScale(XMVectorSet(scale, scale, scale, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("cloudTransparent"));

		clouds.push_back(transform);
	}

	{
		Entity* entity = sceneManager->CreateEntity("Flag01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 90.f, 0.f));
		transform->SetPosition(XMVectorSet(-800.f, 560.86f, -920.f, 1.f));
		transform->SetScale(XMVectorSet(16.f, 16.f, 16.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("FlagPole"));
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("FlagTop"));
	}

	Entity* directionalLightEntity = sceneManager->CreateEntity("DirectionalLight01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(directionalLightEntity);
		DirectionalLightComponent* light = sceneManager->CreateComponent<DirectionalLightComponent>(directionalLightEntity);
		transform->SetRotation(Quaternion::FromAngles(25.f, 90.f, 0.f));
		light->SetLightColor(XMVectorSet(0.9f, 0.85f, 0.8f, 1.f));
		light->SetLightIntensity(10.f);
	}

	Entity* spotLightEntity = sceneManager->CreateEntity("SpotLight01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(spotLightEntity);
		SpotLightComponent* light = sceneManager->CreateComponent<SpotLightComponent>(spotLightEntity);
		light->SetInnerAngle(20.f);
		light->SetOuterAngle(30.f);
		light->SetRadius(400.f);
		light->SetLightColor(XMVectorSet(5.f, 2.f, 0.f, 0.f));
		light->SetLightIntensity(0.f);
	}

	// time variables
	XTime timer;

	renderer->SetActiveCamera(cameraEntity->GetComponent<CameraComponent>());
	renderer->SetSecondaryCamera(secondaryCamera->GetComponent<CameraComponent>());
	renderer->SetDirectionalLight(directionalLightEntity->GetComponent<DirectionalLightComponent>());

	// Debug stuff
#ifdef _DEBUG
	DebugHelpers::CreateDebugSphere(renderer, 20);
	DebugHelpers::DebugMat = renderer->LoadMaterial("Debug");
	DebugHelpers::DebugMat->vertexShader = renderer->GetResourceManager()->GetResource<VertexShader>("Default");
	DebugHelpers::DebugMat->pixelShader = renderer->LoadPixelShader("Debug");
#endif

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// message loop
	while(window->Update())
	{
		timer.Signal();
		float deltaTime = static_cast<float>(timer.Delta());
		float totalTime = static_cast<float>(timer.TotalTimeExact());

		static DWORD frameCount = 0; ++frameCount;
		static DWORD framesPast = frameCount;
		static DWORD prevCount = (DWORD)timer.TotalTime();
		if(GetTickCount() - prevCount > 1000) // only update every second
		{
			char buffer[256];
			sprintf_s(buffer, "DirectX Test. FPS: %d", frameCount - framesPast);
			SetWindowTextA(static_cast<HWND>(window->GetHandle()), buffer);
			framesPast = frameCount;
			prevCount = GetTickCount();
		}

		// Handle input
		{
			CameraComponent* camera = cameraEntity->GetComponent<CameraComponent>();
			static bool Track = false;
			float x, y, z;
			TransformComponent* transform = cameraEntity->GetComponent<TransformComponent>();
			x = static_cast<float>(CoreInput::GetMouseX());
			y = static_cast<float>(CoreInput::GetMouseY());

			static float accumAngleY = 0.f;
			float angleX = XMConvertToRadians(x*10.f);
			float angleY = XMConvertToRadians(y*10.f);;

			Quaternion rot = transform->GetRotation();
			Quaternion horizontalRot = Quaternion::FromAxisAngle(VectorConstants::Up, angleX);
			Quaternion verticalRot = Quaternion::FromAxisAngle(rot.GetRightVector(), angleY);

			XMVECTOR prevFw = XMVectorSetY(rot.GetForwardVector(), 0.f);
			prevFw = XMVector3Normalize(prevFw);
			XMVECTOR nextFw = rot.GetForwardVector()*verticalRot;
			XMVECTOR angleVec = XMVector3AngleBetweenVectors(prevFw, nextFw);
			float angle = XMConvertToDegrees(XMVectorGetX(angleVec));
			if(angle > 90.f)
				verticalRot = XMQuaternionIdentity();

			x = 0;
			y = 0;
			z = 0;

			x -= (CoreInput::GetKeyState(KeyCode::A) == KeyState::Down) * 100.f * deltaTime;
			x += (CoreInput::GetKeyState(KeyCode::D) == KeyState::Down) * 100.f * deltaTime;

			y += (CoreInput::GetKeyState(KeyCode::Space) == KeyState::Down) * 100.f * deltaTime;
			y -= (CoreInput::GetKeyState(KeyCode::Control) == KeyState::Down) * 100.f * deltaTime;

			z += (CoreInput::GetKeyState(KeyCode::W) == KeyState::Down) * 100.f * deltaTime;
			z -= (CoreInput::GetKeyState(KeyCode::S) == KeyState::Down) * 100.f * deltaTime;

			Vector4 offset = XMVectorSet(x, y, z, 0.f);
			offset = offset * rot;
			//offset = XMVectorSetY(offset, XMVectorGetY(offset) + y);

			transform->SetPosition(transform->GetPosition() + offset);
			if(!Track)
				transform->SetRotation(rot*verticalRot*horizontalRot);
			else
			{
				TransformComponent* tree = godTree->GetComponent<TransformComponent>();
				Vector4 fw = tree->GetPosition() - camera->GetCameraPosition();
				fw = XMVector3Normalize(fw);				
				Quaternion initial = cameraEntity->GetComponent<TransformComponent>()->GetRotation();
				Quaternion target = Quaternion::LookAt(fw, VectorConstants::Up);
				target.__xvec = XMQuaternionSlerp(initial.__xvec, target.__xvec, deltaTime*20.f);				
				cameraEntity->GetComponent<TransformComponent>()->SetRotation(target);

				static Vector4 pos = tree->GetPosition();
				Vector4 move = XMVectorSet(20.f, 0.f, 20.f, 0.f);
				tree->SetPosition(XMVectorLerp(pos - move, pos + move, sin(totalTime)));
			}

			if(CoreInput::GetKeyState(KeyCode::N) == KeyState::DownFirst)
			{
				Track = !Track;
			}

			if(CoreInput::GetKeyState(KeyCode::L) == KeyState::Down)
			{
				Material* flagMat = renderer->GetResourceManager()->GetResource<Material>("Flag");
				if(flagMat)
				{
					RenderTexture2D* rt = dynamic_cast<RenderTexture2D*>(flagMat->diffuseMap);
					if(rt)
					{
						renderer->RenderSceneToTexture(rt, cameraEntity->GetComponent<CameraComponent>());
					}
				}
			}

			{
				static int currSetting = 1;

				if(CoreInput::GetKeyState(KeyCode::Num1) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::One) == KeyState::DownFirst)
					currSetting = 1;
				if(CoreInput::GetKeyState(KeyCode::Num2) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Two) == KeyState::DownFirst)
					currSetting = 2;
				if(CoreInput::GetKeyState(KeyCode::Num3) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Three) == KeyState::DownFirst)
					currSetting = 3;
				if(CoreInput::GetKeyState(KeyCode::Num4) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Four) == KeyState::DownFirst)
					currSetting = 4;
				if(CoreInput::GetKeyState(KeyCode::Num5) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Five) == KeyState::DownFirst)
					currSetting = 5;
				if(CoreInput::GetKeyState(KeyCode::Num6) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Six) == KeyState::DownFirst)
					currSetting = 6;
				if(CoreInput::GetKeyState(KeyCode::Num7) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Seven) == KeyState::DownFirst)
					currSetting = 7;
				if(CoreInput::GetKeyState(KeyCode::Num8) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Eight) == KeyState::DownFirst)
					currSetting = 8;
				if(CoreInput::GetKeyState(KeyCode::Num9) == KeyState::DownFirst || CoreInput::GetKeyState(KeyCode::Nine) == KeyState::DownFirst)
					currSetting = 9;

				float val = 0;
				val += ((CoreInput::GetKeyState(KeyCode::Plus) == KeyState::Down) || (CoreInput::GetKeyState(KeyCode::Up) == KeyState::Down)) * deltaTime;
				val -= ((CoreInput::GetKeyState(KeyCode::Minus) == KeyState::Down) || (CoreInput::GetKeyState(KeyCode::Down) == KeyState::Down)) * deltaTime;

				if(CoreInput::GetKeyState(KeyCode::Q) == KeyState::DownFirst)
				{
					renderer->GetPostProcessingEffect<Fog>()->Toggle();
				}

				if(CoreInput::GetKeyState(KeyCode::Z) == KeyState::DownFirst)
				{
					renderer->GetPostProcessingEffect<DOF>()->Toggle();
				}

				if(CoreInput::GetKeyState(KeyCode::X) == KeyState::DownFirst)
				{
					renderer->GetPostProcessingEffect<Tonemapper>()->ToggleWarp();
				}

				if(CoreInput::GetKeyState(KeyCode::F) == KeyState::DownFirst)
				{
					SpotLightComponent* light = spotLightEntity->GetComponent<SpotLightComponent>();
					float intensity = light->GetLightIntensity();
					light->SetLightIntensity(intensity == 0.f ? 20.f : 0.f);
				}

				static Entity* lastLight;
				if(CoreInput::GetKeyState(KeyCode::G) == KeyState::DownFirst)
				{
					static int i = 0;

					static std::vector<Entity*> lights;
					if(lights.size() < 5)
					{
						lastLight = sceneManager->CreateEntity("CreatedLight" + std::to_string(i));
						sceneManager->CreateComponent<TransformComponent>(lastLight);
						sceneManager->CreateComponent<PointLightComponent>(lastLight);
						lights.push_back(lastLight);
					}
					else
					{
						lastLight = lights[i];
					}

					TransformComponent* transform = lastLight->GetComponent<TransformComponent>();
					PointLightComponent* light = lastLight->GetComponent<PointLightComponent>();

					transform->SetPosition(cameraEntity->GetComponent<TransformComponent>()->GetPosition());
					light->SetRadius(50.f * rand() / (float)RAND_MAX + 20.f);
					float r = rand() / (float)RAND_MAX;
					float g = rand() / (float)RAND_MAX;
					float b = rand() / (float)RAND_MAX;
					Vector4 color = XMVector3Normalize(XMVectorSet(r, g, b, 0.f));
					light->SetLightColor(color);
					light->SetLightIntensity(20.f);
					i = (i + 1) % 5;
				}

				if(CoreInput::GetKeyState(KeyCode::G) == KeyState::Down)
				{
					if(lastLight)
					{
						TransformComponent* transform = lastLight->GetComponent<TransformComponent>();
						transform->SetPosition(cameraEntity->GetComponent<TransformComponent>()->GetPosition());
					}
				}

				if(CoreInput::GetKeyState(KeyCode::C) == KeyState::Down)
				{
					for(auto& comp : *sceneManager->GetComponents<InstancedMeshComponent>())
					{
						comp.SetTransform(cameraEntity->GetComponent<TransformComponent>()->GetTransform());
					}
				}

				if(CoreInput::GetKeyState(KeyCode::V) == KeyState::DownFirst)
				{
					for(auto& comp : *sceneManager->GetComponents<InstancedMeshComponent>())
					{
						comp.RandomizeInstanceTransforms();
					}
				}

				if(CoreInput::GetKeyState(KeyCode::B) == KeyState::DownFirst)
				{
					static bool sw = false;
					if(sw)
						godTree->GetComponent<MeshComponent>()->GetMeshes()[0]->material = renderer->GetResourceManager()->GetResource<Material>("GodTree");
					else
						godTree->GetComponent<MeshComponent>()->GetMeshes()[0]->material = renderer->GetResourceManager()->GetResource<Material>("GodTreeGlow");

					sw = !sw;
				}

				if(CoreInput::GetKeyState(KeyCode::M) == KeyState::DownFirst)
				{
					renderer->bMinimap = !renderer->bMinimap;
				}

				switch(currSetting)
				{
					case 1:
					{
						renderer->GetPostProcessingEffect<Tonemapper>()->AddExposure(val*2.f);
						break;
					}
					case 2:
					{
						renderer->GetPostProcessingEffect<Tonemapper>()->AddBWFilterStrength(val*2.f);
						break;
					}
					case 3:
					{
						renderer->GetPostProcessingEffect<Bloom>()->AddIntensity(val*3.f);
						break;
					}
					case 4:
					{
						renderer->GetPostProcessingEffect<DOF>()->MoveNearPlane(val*1000.f);
						break;
					}
					case 5:
					{
						renderer->GetPostProcessingEffect<Fog>()->AddFogAlpha(val*3.f);
						break;
					}
					case 6:
					{
						float FOV = camera->GetFOV();
						FOV += val * 40.f;
						FOV = std::clamp(FOV, 40.f, 180.f);
						cameraEntity->GetComponent<CameraComponent>()->UpdateFOV(FOV);
						break;
					}
					case 7:
					{
						float nearClip = camera->GetNearZ();
						nearClip += val * 20.f;
						nearClip = std::clamp(nearClip, 0.1f, 500.0f);
						cameraEntity->GetComponent<CameraComponent>()->UpdateNearZ(nearClip);
						break;
					}
					case 8:
					{
						float farClip = camera->GetFarZ();
						farClip += val * farClip;
						farClip = std::clamp(farClip, 100.0f, 8000.f);
						cameraEntity->GetComponent<CameraComponent>()->UpdateFarZ(farClip);
						break;
					}
					case 9:
					{
						TransformComponent* dir = directionalLightEntity->GetComponent<TransformComponent>();
						dir->SetRotation(dir->GetRotation()*Quaternion::FromAngles(0.f, val*40.f, 0.f));
					}
				}
			}
		};

		for(auto& t : clouds)
		{
			Vector4 pos = t->GetPosition();
			pos = pos * Quaternion::FromAngles(0.f, deltaTime*0.5f, 0.f);
			t->SetPosition(pos);
		}

		//directionalLightEntity->GetComponent<TransformComponent>()->SetRotation(Quaternion::FromAngles(40.f, totalTime*15.f, 0.f));

		{
			TransformComponent* cameraTransform = cameraEntity->GetComponent<TransformComponent>();
			TransformComponent* spotTransform = spotLightEntity->GetComponent<TransformComponent>();

			Quaternion cameraRot = cameraTransform->GetRotation();

			Quaternion targetRot = Quaternion::LookAt(cameraRot.GetForwardVector(), cameraRot.GetUpVector());
			Vector4 targetPos = cameraTransform->GetPosition() - cameraRot.GetUpVector()*5.f - cameraRot.GetForwardVector()*5.f;

			secondaryCamera->GetComponent<TransformComponent>()->SetPosition(cameraTransform->GetPosition() + VectorConstants::Up*200.f);

			spotTransform->SetPosition(targetPos);
			spotTransform->SetRotation(targetRot);
		}

		renderer->SetActiveModels(sceneManager->GetComponents<MeshComponent>(), sceneManager->GetComponents<InstancedMeshComponent>());
		renderer->SetActiveLights(XMFLOAT3(0.5f, 0.5f, 0.5f), sceneManager->GetComponents<PointLightComponent>(), sceneManager->GetComponents<SpotLightComponent>());
		renderer->UpdateSceneBuffer(totalTime);
		renderer->RenderFrame();

	#ifdef _DEBUG
		if(CoreInput::GetKeyState(KeyCode::Y) == KeyState::Down)
			renderer->DrawDebugShape(DebugHelpers::DebugSphere, godTree->GetComponent<TransformComponent>()->GetTransformMatrix());
		Log::DebugConsole::PrintDeferred();
	#endif

		renderer->PresentFrame();
	}

	// return the right message parameter as exit code
	window->Shutdown();
	renderer->Shutdown();

	delete window;
	delete renderer;
	delete sceneManager;

	return 0;
}