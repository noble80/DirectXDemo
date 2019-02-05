#include "stdafx.h"
#include "Renderer\Window.h"

#include "Input\CoreInput.h"
#include "Renderer\Renderer.h"
#include "Engine\SceneManager.h"
#include "Renderer\ResourceManager.h"

#include <windowsx.h>
#include "XTime.h"
#include <string>

#include "Renderer\GraphicsStructures.h"
#include "Renderer\Material.h"
#include "Renderer\Texture2D.h"
#include "Renderer\Mesh.h"
#include "Renderer\ShaderBuffers.h"
#include "Importer\FBXImporter.h"

#include "Engine\TransformComponent.h"
#include "Engine\LightComponent.h"
#include "Engine\ModelComponent.h"
#include "Engine\CameraComponent.h"

#include "DebugMemory.h"
#include "Engine\Log.h"

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
	//_CrtSetBreakAlloc(27237);

	Log::DebugConsole::Initialize();

	Window* window = new Window;
	SceneManager* sceneManager = new SceneManager;
	Renderer* renderer = new Renderer;

	window->Initialize(Vector2(WIDTH, HEIGHT), 0, L"DirectXTest");
	renderer->Initialize(window);

	//Load up meshes
	{
		std::string meshNames[] = {"rock01", "plane01"};
		float scales[] = {40.f, 5.f};
		int n = ARRAYSIZE(scales);
		for(int i = 0; i < n; ++i)
		{
			std::vector<FBXImporter::Mesh> fbxmeshes;
			FBXImporter::ImportModelFromFile(meshNames[i], fbxmeshes, scales[i], 0); // FBX_IMPORT_FLAG_SWAP_YZ);
			int j = 1;
			for(auto& fbxmesh : fbxmeshes)
			{
				renderer->CreateGeometryBuffer(meshNames[i] + "_" + std::to_string(j), &fbxmesh.vertices, fbxmesh.indices);
				j++;
			}
		}
		//Create materials
		Material* mat = renderer->CreateMaterialFromFile("Trivial");
		mat->textures.push_back(renderer->CreateTextureFromFile("Rock01_LP_albedo"));
		mat->textures.push_back(renderer->CreateTextureFromFile("Rock01_LP_normal"));
		Material* mat2 = renderer->CreateMaterialFromFile("ColorLit");

		Mesh* rock = renderer->GetResourceManager()->CreateResource<Mesh>("Rock01");
		rock->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("rock01_1");
		rock->material = renderer->GetResourceManager()->GetResource<Material>("Trivial");

		Mesh* plane = renderer->GetResourceManager()->CreateResource<Mesh>("Plane01");
		plane->geometry = renderer->GetResourceManager()->GetResource<GeometryBuffer>("plane01_1");
		plane->material = renderer->GetResourceManager()->GetResource<Material>("ColorLit");
	}



	// Entity creation
	Entity* cameraEntity = sceneManager->CreateEntity("Camera01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(cameraEntity);
		transform->SetRotation(XMQuaternionRotationRollPitchYaw(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 2.f, -10.0f, 1.f));

		CameraComponent* camera = sceneManager->CreateComponent<CameraComponent>(cameraEntity);
		camera->SetProjectionMatrix(90.f, WIDTH / (FLOAT)HEIGHT, 0.01f, 100.0f);
	}

	Entity* rockEntity = sceneManager->CreateEntity("Rock01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(rockEntity);
		transform->SetRotation(XMQuaternionRotationRollPitchYaw(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 1.f, 0.f, 1.f));

		ModelComponent* model = sceneManager->CreateComponent<ModelComponent>(rockEntity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("Rock01"));
	}

	Entity* planeEntity = sceneManager->CreateEntity("Plane01");
	{
		TransformComponent* transform = sceneManager->CreateComponent<TransformComponent>(planeEntity);
		transform->SetRotation(XMQuaternionRotationRollPitchYaw(0.f, 0.f, 0.f));
		transform->SetPosition(XMVectorSet(0.f, 0.f, 0.f, 1.f));

		ModelComponent* model = sceneManager->CreateComponent<ModelComponent>(planeEntity);
		model->AddMesh(renderer->GetResourceManager()->GetResource<Mesh>("Plane01"));
	}

	Entity* directionalLightEntity = sceneManager->CreateEntity("DirectionalLight01");
	{
		sceneManager->CreateComponent<TransformComponent>(directionalLightEntity);
		LightComponent* light = sceneManager->CreateComponent<LightComponent>(directionalLightEntity);
		light->SetLightColor(XMFLOAT4(0.9f, 0.85f, 0.8f, 0.f));
		light->SetLightIntensity(2.f);
	}

	// time variables
	XTime timer;

	renderer->SetActiveCamera(cameraEntity->GetComponent<CameraComponent>());
	renderer->SetDirectionalLight(directionalLightEntity->GetComponent<LightComponent>());
	// message loop
	while(window->Update())
	{
		timer.Signal();

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
			float angleX = XMConvertToRadians(x*0.2f);
			float angleY = XMConvertToRadians(y*0.2f);;

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

			x -= (CoreInput::GetKeyState(KeyCode::A) == KeyState::Down) * 10.f * timer.Delta();
			x += (CoreInput::GetKeyState(KeyCode::D) == KeyState::Down) * 10.f * timer.Delta();

			y += (CoreInput::GetKeyState(KeyCode::Space) == KeyState::Down) * 10.f * timer.Delta();
			y -= (CoreInput::GetKeyState(KeyCode::Control) == KeyState::Down) * 10.f * timer.Delta();

			z += (CoreInput::GetKeyState(KeyCode::W) == KeyState::Down) * 10.f * timer.Delta();
			z -= (CoreInput::GetKeyState(KeyCode::S) == KeyState::Down) * 10.f * timer.Delta();

			float val = 0.f;
			val += (CoreInput::GetKeyState(KeyCode::O) == KeyState::Down) * 1.f * timer.Delta();
			val -= (CoreInput::GetKeyState(KeyCode::L) == KeyState::Down) * 1.f * timer.Delta();
			TransformComponent* rockT = rockEntity->GetComponent<TransformComponent>();
			rockT->SetPosition(rockT->GetPosition() + XMVectorSet(0.f, val, 0.f, 0.f));


			Vector4 offset = XMVectorSet(x, y, z, 0.f);
			offset = offset * rot;
			//offset = XMVectorSetY(offset, XMVectorGetY(offset) + y);

			transform->SetPosition(transform->GetPosition() + offset);
			transform->SetRotation(rot*verticalRot*horizontalRot);

		} CoreInput::ResetAxes();

		static float yaw = 0.f;
		float pitch = XMConvertToRadians(22.5f);
		float roll = XMConvertToRadians(0.f);

		directionalLightEntity->GetComponent<TransformComponent>()->SetRotation(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));

		yaw += static_cast<float>(timer.Delta());


		renderer->SetActiveModels(sceneManager->GetComponents<ModelComponent>());
		renderer->UpdateLightBuffers(sceneManager->GetComponents<LightComponent>());
		renderer->RenderFrame();

		Log::DebugConsole::PrintDeferred();
	}

	// return the right message parameter as exit code
	window->Shutdown();
	renderer->Shutdown();

	delete window;
	delete renderer;
	delete sceneManager;

	return 0;
}