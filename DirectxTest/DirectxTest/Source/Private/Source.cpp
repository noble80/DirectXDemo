#include "stdafx.h"
#include "Renderer\Window.h"

#include "Input\CoreInput.h"
#include "Renderer\Renderer.h"
#include "Engine\SceneManager.h"
#include "Renderer\ResourceManager.h"

#include <DirectXColors.h>

#include <windowsx.h>
#include "XTime.h"
#include <string>

#include "Renderer\GraphicsStructures.h"
#include "Renderer\Material.h"
#include "Renderer\Texture2D.h"
#include "Renderer\Mesh.h"
#include "Renderer\ShaderBuffers.h"
#include "Importer\MeshImporter.h"

#include "Engine\TransformComponent.h"
#include "Engine\DirectionalLightComponent.h"
#include "Engine\SpotLightComponent.h"
#include "Engine\PointLightComponent.h"
#include "Engine\MeshComponent.h"
#include "Engine\CameraComponent.h"

#include "DebugMemory.h"
#include "Engine\Log.h"

#include "Renderer\DebugHelpers.h"
#include <iostream>
#include <d3d11_1.h>
#include <d3d11.h>

using namespace DirectX;

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
	//_CrtSetBreakAlloc(227251);

	Log::DebugConsole::Initialize();

	Window* window = new Window;
	SceneManager* sceneManager = new SceneManager;
	Renderer* renderer = new Renderer;

	window->Initialize(Vector2(WIDTH, HEIGHT), 0, L"DirectXTest");
	renderer->Initialize(window);

	//Load up meshes
	{
		std::string meshNames[] = {"Sphere01", "CornellBox01", "Rock01", "Flag01_1", "Flag01_2"};
		int n = ARRAYSIZE(meshNames);
		for(int i = 0; i < n; ++i)
		{
			MeshImporter::Mesh mesh;
			MeshImporter::ImportModelFromFile(meshNames[i], mesh); // FBX_IMPORT_FLAG_SWAP_YZ);
			int j = 1;
			renderer->CreateGeometryBuffer(mesh.name, &mesh.vertices, mesh.indices);
		}
		//Create materials
		Material* basicMat = renderer->CreateMaterialFromFile("ColorLit");
		Material* reflectiveMat = renderer->CreateMaterialFromFile("ColorLitReflective");
		reflectiveMat->textures.push_back(renderer->CreateTextureFromFile("OvercastCubemap"));
		reflectiveMat->surfaceParameters.specularIntensity = 1.f;
		reflectiveMat->surfaceParameters.specularExponent = 1250.f;

		Material* rockMat = renderer->CreateMaterialFromFile("Trivial");
		rockMat->textures.push_back(renderer->CreateTextureFromFile("Rock01_LP_albedo"));
		rockMat->textures.push_back(renderer->CreateTextureFromFile("Rock01_LP_normal"));

		Material* cornellBoxMat = renderer->CreateMaterialFromFile("ColorLitAO");
		cornellBoxMat->textures.push_back(renderer->CreateTextureFromFile("CornellBoxAO"));

		// Create meshes
		Mesh* rock = renderer->GetResourceManager()->CreateResource<Mesh>("Rock01");
		rock->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Rock01");
		rock->material = rockMat;

		Mesh* box = renderer->GetResourceManager()->CreateResource<Mesh>("CornellBox01");
		box->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("CornellBox01");
		box->material = cornellBoxMat;

		Mesh* sphere = renderer->GetResourceManager()->CreateResource<Mesh>("Sphere01");
		sphere->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Sphere01");
		sphere->material = reflectiveMat;

		Mesh* waveSphere = renderer->GetResourceManager()->CreateResource<Mesh>("WaveSphere01");
		waveSphere->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Sphere01");
		waveSphere->material = renderer->CreateMaterialFromFile("Wave");

		Mesh* flagPole = renderer->GetResourceManager()->CreateResource<Mesh>("FlagPole");
		flagPole->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Flag01_2");
		flagPole->material = basicMat;

		Mesh* flagTop = renderer->GetResourceManager()->CreateResource<Mesh>("FlagTop");
		flagTop->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("Flag01_1");
		flagTop->material = renderer->CreateMaterialFromFile("Flag");

		{
			MeshImporter::Mesh imesh = MeshImporter::WeakImportModelFromOBJ();
			Mesh* obj = renderer->GetResourceManager()->CreateResource<Mesh>("ComplexMeshOBJ");
			obj->geometry = renderer->CreateGeometryBuffer("complexOBJ", &imesh.vertices, imesh.indices);
			obj->material = cornellBoxMat;
		}
	}

	// Entity creation
	Entity* cameraEntity = sceneManager->CreateEntity("Camera01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(cameraEntity);
		transform->SetRotation(Quaternion::FromAngles(5.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 5.f, -25.f, 1.f));

		CameraComponent* camera = sceneManager->CreateComponent<CameraComponent>(cameraEntity);
		camera->SetProjectionMatrix(90.f, WIDTH / (FLOAT)HEIGHT, 0.01f, 100.0f);
	}

	{
		Entity* entity = sceneManager->CreateEntity("Rock01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 0.f, 0.f, 1.f));
		transform->SetScale(XMVectorSet(0.025f, 0.025f, 0.025f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("Rock01"));

	}
	{
		Entity* entity = sceneManager->CreateEntity("Box01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 0.f, 0.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("CornellBox01"));
	}

	{
		Entity* entity = sceneManager->CreateEntity("Flag01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 10.f, 5.f, 1.f));
		transform->SetScale(XMVectorSet(2.f, 2.f, 2.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("FlagPole"));
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("FlagTop"));
	}

	{
		Entity* entity = sceneManager->CreateEntity("Sphere01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(4.f, 10.f, 3.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("Sphere01"));
	}


	{
		Entity* entity = sceneManager->CreateEntity("ComplexOBJ");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, -60.f, 0.f));
		transform->SetPosition(XMVectorSet(-5.f, 0.2f, 3.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("ComplexMeshOBJ"));
	}

	{
		Entity* entity = sceneManager->CreateEntity("Sphere03");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetRotation(Quaternion::FromAngles(0.f, 180.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 0.f, 4.f, 1.f));

		MeshComponent* model = sceneManager->CreateComponent<MeshComponent>(entity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("WaveSphere01"));
	}

	Entity* directionalLightEntity = sceneManager->CreateEntity("DirectionalLight01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(directionalLightEntity);
		DirectionalLightComponent* light = sceneManager->CreateComponent<DirectionalLightComponent>(directionalLightEntity);
		transform->SetRotation(Quaternion::FromAngles(90.f, 0.f, 0.f));
		light->SetLightColor(XMVectorSet(0.9f, 0.85f, 0.8f, 0.f));
		light->SetLightIntensity(1.f);
	}

	{
		Entity* entity = sceneManager->CreateEntity("PointLight01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetPosition(XMVectorSet(-4.f, 2.f, 2.f, 1.f));
		PointLightComponent* light = sceneManager->CreateComponent<PointLightComponent>(entity);
		light->SetLightColor(XMVectorSet(1.f, 0.f, 0.f, 0.f));
		light->SetLightIntensity(3.f);
		light->SetRadius(10.f);
	}

	{
		Entity* entity = sceneManager->CreateEntity("PointLight02");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetPosition(XMVectorSet(4.f, 2.f, 2.f, 1.f));
		PointLightComponent* light = sceneManager->CreateComponent<PointLightComponent>(entity);
		light->SetLightColor(XMVectorSet(0.f, 1.f, 0.f, 0.f));
		light->SetLightIntensity(3.f);
		light->SetRadius(10.f);
	}

	{
		Entity* entity = sceneManager->CreateEntity("SpotLight01");
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(entity);
		transform->SetPosition(XMVectorSet(-8.00833035f, 1.99542284f, -4.99794912f, 1.00000000f));
		transform->SetRotation(XMVectorSet(0.00148191745f, 0.528432846f, -0.000922378327f, 0.848963261f));
		SpotLightComponent* light = sceneManager->CreateComponent<SpotLightComponent>(entity);
		light->SetLightColor(XMVectorSet(0.f, 0.f, 1.f, 0.f));
		light->SetLightIntensity(15.f);
		light->SetRadius(15.f);
		light->SetInnerAngle(10.f);
		light->SetOuterAngle(25.f);
	}

	// time variables
	XTime timer;

	renderer->SetActiveCamera(cameraEntity->GetComponent<CameraComponent>());
	renderer->SetDirectionalLight(directionalLightEntity->GetComponent<DirectionalLightComponent>());

	// Debug stuff
#ifdef _DEBUG
	DebugHelpers::CreateDebugSphere(renderer, 20);
	DebugHelpers::DebugMat = renderer->CreateMaterialFromFile("Debug");
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

			x -= (CoreInput::GetKeyState(KeyCode::A) == KeyState::Down) * 10.f * deltaTime;
			x += (CoreInput::GetKeyState(KeyCode::D) == KeyState::Down) * 10.f * deltaTime;

			y += (CoreInput::GetKeyState(KeyCode::Space) == KeyState::Down) * 10.f * deltaTime;
			y -= (CoreInput::GetKeyState(KeyCode::Control) == KeyState::Down) * 10.f * deltaTime;

			z += (CoreInput::GetKeyState(KeyCode::W) == KeyState::Down) * 10.f * deltaTime;
			z -= (CoreInput::GetKeyState(KeyCode::S) == KeyState::Down) * 10.f * deltaTime;

			float val = 0.f;
			val += (CoreInput::GetKeyState(KeyCode::O) == KeyState::Down) * 1.f * deltaTime;
			val -= (CoreInput::GetKeyState(KeyCode::L) == KeyState::Down) * 1.f * deltaTime;

			Vector4 offset = XMVectorSet(x, y, z, 0.f);
			offset = offset * rot;
			//offset = XMVectorSetY(offset, XMVectorGetY(offset) + y);

			transform->SetPosition(transform->GetPosition() + offset);
			transform->SetRotation(rot*verticalRot*horizontalRot);

		} CoreInput::ResetAxes();

		directionalLightEntity->GetComponent<TransformComponent>()->SetRotation(Quaternion::FromAngles(40.f, totalTime*15.f, 0.f));

		for(auto& light : *sceneManager->GetComponents<PointLightComponent>())
		{
			TransformComponent* transform = light.GetOwner()->GetComponent<TransformComponent>();
			Vector4 pos = transform->GetPosition();
			transform->SetPosition(pos + XMVectorSet(sin(totalTime)*0.05f, cos(totalTime)*0.04f, 0.f, 0.f));
		}

		for(auto& light : *sceneManager->GetComponents<SpotLightComponent>())
		{
			TransformComponent* transform = light.GetOwner()->GetComponent<TransformComponent>();
			Vector4 pos = transform->GetPosition();
			Quaternion rot = transform->GetRotation();
			rot = rot * Quaternion::FromAxisAngle(rot.GetUpVector(), -deltaTime * 45.f);
			transform->SetRotation(rot);
		}

		renderer->SetActiveModels(sceneManager->GetComponents<MeshComponent>());
		renderer->UpdateLightBuffers(XMFLOAT3(0.4f, 0.4f, 0.4f), sceneManager->GetComponents<PointLightComponent>(), sceneManager->GetComponents<SpotLightComponent>());
		renderer->UpdateSceneBuffer(totalTime);
		renderer->RenderFrame();

	#ifdef _DEBUG
		renderer->DrawDebugShape(DebugHelpers::DebugSphere, sceneManager->GetEntity("Sphere01")->GetComponent<TransformComponent>()->GetTransformMatrix());
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