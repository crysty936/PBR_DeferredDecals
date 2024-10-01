#include "TestGameMode.h"
#include "Camera/Camera.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Renderer/Drawable/ShapesUtils/BasicShapes.h"
#include "Controller/ControllerBase.h"
#include "glm/common.hpp"
#include <stdlib.h>
#include "Renderer/Model/3D/Assimp/AssimpModel3D.h"
#include "Core/SceneHelper.h"
#include "Renderer/RHI/Resources/MeshDataContainer.h"
#include "Renderer/ForwardRenderer.h"
#include "Renderer/RHI/Resources/VertexInputLayout.h"
#include "Renderer/Drawable/ShapesUtils/BasicShapesData.h"
#include "Renderer/RHI/RHI.h"
#include "Renderer/Material/MaterialsManager.h"
#include "Utils/InlineVector.h"
#include "Renderer/Material/EngineMaterials/RenderMaterial_Parallax.h"
#include "Renderer/Material/EngineMaterials/RenderMaterial_Billboard.h"
#include "Renderer/DrawDebugHelpers.h"
#include "Renderer/Model/3D/Assimp/AssimpModel3DEditorSphere.h"
#include "imgui.h"

TestGameMode GGameMode = {};

TestGameMode::TestGameMode() = default;
TestGameMode::~TestGameMode() = default;

class InstancedAssimpModelTest : public AssimpModel3D
{
public:
	InstancedAssimpModelTest(const eastl::string& inPath, const eastl::string& inName)
		: AssimpModel3D(inPath, inName) {}
	virtual ~InstancedAssimpModelTest() = default;

protected:
	virtual eastl::shared_ptr<RHIShader> CreateShaders(const class VertexInputLayout& inLayout) const override
	{
		eastl::vector<ShaderSourceInput> shaders = {
		{ "VS_Pos-UV-Normal_Instanced", EShaderType::Sh_Vertex },
		{ "PS_BasicTex", EShaderType::Sh_Fragment } };

		return RHI::Instance->CreateShaderFromPath(shaders, inLayout);
	}


	virtual void AddAdditionalBuffers(eastl::shared_ptr<MeshDataContainer>& inDataContainer) const override
	{
		// Instance data
		glm::mat4* instanceOffsets = new glm::mat4[InstancesCount * InstancesCount];

		for (int32_t i = 0; i < InstancesCount; ++i)
		{
			for (int32_t j = 0; j < InstancesCount; ++j)
			{
				glm::mat4& instanceMat = instanceOffsets[i * InstancesCount + j];

				instanceMat = glm::identity<glm::mat4>();
				instanceMat = glm::translate(instanceMat, glm::vec3(4.f * i, 4.f * j, 0.f));
			}
		}

		VertexInputLayout vertexInstanceLayout;
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);

		vertexInstanceLayout.AttribsOffset = 3;

		const eastl::shared_ptr<RHIVertexBuffer> instanceBuffer = RHI::Instance->CreateVertexBuffer(vertexInstanceLayout, &instanceOffsets[0], sizeof(glm::mat4) * InstancesCount * InstancesCount);
		inDataContainer->AdditionalBuffers.push_back(instanceBuffer);
	}


	virtual RenderCommand CreateRenderCommand(eastl::shared_ptr<RenderMaterial>& inMaterial, eastl::shared_ptr<MeshNode>& inParent, eastl::shared_ptr<MeshDataContainer>& inDataContainer) override
	{
		RenderCommand newCommand;
		newCommand.Material = inMaterial;
		newCommand.Parent = inParent;
		newCommand.DataContainer = inDataContainer;
		newCommand.DrawType = EDrawType::DrawInstanced;
		newCommand.InstancesCount = InstancesCount * InstancesCount;
		//newCommand.DrawPasses = static_cast<EDrawMode::Type>(EDrawMode::Default | EDrawMode::NORMAL_VISUALIZE);

		return newCommand;
	}

	const int32_t InstancesCount = 10;
};

// Instanced textured cube
class InstancedCubeTest : public Model3D
{
public:
	InstancedCubeTest(const eastl::string& inName)
		: Model3D(inName)
	{

	}
	virtual ~InstancedCubeTest() = default;

	virtual void CreateProxy() override
	{
 		const eastl::string RenderDataContainerID = "instancedCubeVAO";
 		eastl::shared_ptr<MeshDataContainer> dataContainer{ nullptr };
 
 		const bool existingContainer = ForwardRenderer::Get().GetOrCreateContainer(RenderDataContainerID, dataContainer);
 		VertexInputLayout inputLayout;

 		// Vertex points
 		inputLayout.Push<float>(3, VertexInputType::Position);
 		// Vertex Normal
 		inputLayout.Push<float>(3, VertexInputType::Normal);
 		// Vertex Tex Coords
 		inputLayout.Push<float>(2, VertexInputType::TexCoords);


		if (!existingContainer)
		{
			int32_t indicesCount = BasicShapesData::GetCubeIndicesCount();
			eastl::shared_ptr<RHIIndexBuffer> ib = RHI::Instance->CreateIndexBuffer(BasicShapesData::GetCubeIndices(), indicesCount);


			int32_t verticesCount = BasicShapesData::GetCubeVerticesCount();
			const eastl::shared_ptr<RHIVertexBuffer> vb = RHI::Instance->CreateVertexBuffer(inputLayout, BasicShapesData::GetCubeVertices(), verticesCount, ib);

			dataContainer->VBuffer = vb;
		}

		// Instance data
		constexpr int32_t instancesCount = 10;
		glm::mat4* instanceOffsets = new glm::mat4[instancesCount * instancesCount];

		for (int32_t i = 0; i < instancesCount; ++i)
		{
			for (int32_t j = 0; j < instancesCount; ++j)
			{
				glm::mat4& instanceMat = instanceOffsets[i * instancesCount + j];

				instanceMat = glm::identity<glm::mat4>();
				instanceMat = glm::translate(instanceMat, glm::vec3(glm::sin(0.5 * j) * 40, 10.f, glm::cos(i * 0.5 * j) * 40));
			}
		}

		VertexInputLayout vertexInstanceLayout;
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);
		vertexInstanceLayout.Push<float>(4, VertexInputType::InstanceData, EAttribDivisor::PerInstance);

		vertexInstanceLayout.AttribsOffset = 3;
		
		const eastl::shared_ptr<RHIVertexBuffer> instanceBuffer = RHI::Instance->CreateVertexBuffer(vertexInstanceLayout, &instanceOffsets[0], sizeof(glm::mat4) * instancesCount * instancesCount);
		dataContainer->AdditionalBuffers.push_back(instanceBuffer);

  		MaterialsManager& matManager = MaterialsManager::Get();
  		bool materialExists = false;
  		eastl::shared_ptr<RenderMaterial> material = matManager.GetOrAddMaterial("instanced_cube_material", materialExists);
  
  		if (!materialExists)
  		{
  			eastl::shared_ptr<RHITexture2D> tex = RHI::Instance->CreateAndLoadTexture2D("../Data/Textures/MinecraftGrass.jpg", true);
  			material->OwnedTextures.push_back(tex);
  
  			eastl::vector<ShaderSourceInput> shaders = {
  			{ "Pos-UV-Normal_BasicTex_Instanced/VS_Pos-UV-Normal_Instanced", EShaderType::Sh_Vertex },
  			{ "Pos-UV-Normal_BasicTex_Instanced/PS_BasicTex", EShaderType::Sh_Fragment } };
  
  			material->Shader = RHI::Instance->CreateShaderFromPath(shaders, inputLayout);
  		}
  
  		eastl::shared_ptr<MeshNode> cubeNode = eastl::make_shared<MeshNode>("Cube Mesh");
  		AddChild(cubeNode);
  
  		RenderCommand newCommand;
  		newCommand.Material = material;
  		newCommand.DataContainer = dataContainer;
  		newCommand.Parent = cubeNode;
  		newCommand.DrawType = EDrawType::DrawInstanced;
		newCommand.InstancesCount = instancesCount * instancesCount;
  
  		ForwardRenderer::Get().AddCommand(newCommand);
	}
};

class ParallaxQuad : public Model3D
{
public:
	ParallaxQuad(const eastl::string& inName)
		: Model3D(inName) {}
	virtual ~ParallaxQuad() = default;

	virtual void CreateProxy() override
	{
		const eastl::string RenderDataContainerID = "ParallaxQuadContainer";
		eastl::shared_ptr<MeshDataContainer> dataContainer;

		const bool existingContainer = ForwardRenderer::Get().GetOrCreateContainer(RenderDataContainerID, dataContainer);
		VertexInputLayout inputLayout;

		// Vertex points
		inputLayout.Push<float>(3, VertexInputType::Position);
		// Vertex Normal
		inputLayout.Push<float>(3, VertexInputType::Normal);
		// Vertex Tex Coords
		inputLayout.Push<float>(2, VertexInputType::TexCoords);
 		// Tangent
 		inputLayout.Push<float>(3, VertexInputType::Tangent);
 		// Bitangent
 		inputLayout.Push<float>(3, VertexInputType::Bitangent);

		if (!existingContainer)
		{
			int32_t indicesCount = BasicShapesData::GetTBNQuadIndicesCount();
			eastl::shared_ptr<RHIIndexBuffer> ib = RHI::Instance->CreateIndexBuffer(BasicShapesData::GetTBNQuadIndices(), indicesCount);


			int32_t verticesCount = BasicShapesData::GetTBNQuadVerticesCount();
			const eastl::shared_ptr<RHIVertexBuffer> vb = RHI::Instance->CreateVertexBuffer(inputLayout, BasicShapesData::GetTBNQuadVertices(), verticesCount, ib);

			dataContainer->VBuffer = vb;
		}

		MaterialsManager& matManager = MaterialsManager::Get();
		bool materialExists = false;
		eastl::shared_ptr<RenderMaterial> material = matManager.GetOrAddMaterial<RenderMaterial_Parallax>("parallax_quad_material", materialExists);

		if (!materialExists)
		{
			eastl::shared_ptr<RHITexture2D> diffMap = RHI::Instance->CreateAndLoadTexture2D("../Data/Textures/Parallax/bricks.jpg", true);
			eastl::shared_ptr<RHITexture2D> normalMap = RHI::Instance->CreateAndLoadTexture2D("../Data/Textures/Parallax/bricks_normal.jpg", false);
			eastl::shared_ptr<RHITexture2D> heightMap = RHI::Instance->CreateAndLoadTexture2D("../Data/Textures/Parallax/bricks_disp.jpg", false);

			material->OwnedTextures.push_back(diffMap);
			material->OwnedTextures.push_back(normalMap);
			material->OwnedTextures.push_back(heightMap);

			eastl::vector<ShaderSourceInput> shaders = {
			{ "Parallax/VS_Pos-UV-Normal-Tangent-Bitangent_Model_WorldPosition", EShaderType::Sh_Vertex },
			{ "Parallax/PS_ParallaxTest", EShaderType::Sh_Fragment } };

			material->Shader = RHI::Instance->CreateShaderFromPath(shaders, inputLayout);
		}

		eastl::shared_ptr<MeshNode> cubeNode = eastl::make_shared<MeshNode>("Mesh");
		AddChild(cubeNode);

		RenderCommand newCommand;
		newCommand.Material = material;
		newCommand.DataContainer = dataContainer;
		newCommand.Parent = cubeNode;
		newCommand.DrawType = EDrawType::DrawElements;

		ForwardRenderer::Get().AddCommand(newCommand);
	}
};

class BillboardQuad : public Model3D
{
public:
	BillboardQuad() = default;
	virtual ~BillboardQuad() = default;

	virtual void CreateProxy() override
	{
		const eastl::string RenderDataContainerID = "BillboardQuadContainer";
		eastl::shared_ptr<MeshDataContainer> dataContainer;

		const bool existingContainer = ForwardRenderer::Get().GetOrCreateContainer(RenderDataContainerID, dataContainer);
		VertexInputLayout inputLayout;

		// Vertex points
		inputLayout.Push<float>(3, VertexInputType::Position);
		// Vertex Tex Coords
		inputLayout.Push<float>(2, VertexInputType::TexCoords);

		if (!existingContainer)
		{
			int32_t indicesCount = BasicShapesData::GetQuadIndicesCount();
			eastl::shared_ptr<RHIIndexBuffer> ib = RHI::Instance->CreateIndexBuffer(BasicShapesData::GetQuadIndices(), indicesCount);


			int32_t verticesCount = BasicShapesData::GetQuadVerticesCount();
			const eastl::shared_ptr<RHIVertexBuffer> vb = RHI::Instance->CreateVertexBuffer(inputLayout, BasicShapesData::GetQuadVertices(), verticesCount, ib);

			dataContainer->VBuffer = vb;
		}

		MaterialsManager& matManager = MaterialsManager::Get();
		bool materialExists = false;
		eastl::shared_ptr<RenderMaterial> material = matManager.GetOrAddMaterial<RenderMaterial_Billboard>("billboard_quad_material", materialExists);

		if (!materialExists)
		{
			eastl::shared_ptr<RHITexture2D> diffMap = RHI::Instance->CreateAndLoadTexture2D("../Data/Textures/MinecraftGrass.jpg", true);

			material->OwnedTextures.push_back(diffMap);

			eastl::vector<ShaderSourceInput> shaders = {
			{ "Pos-UV_BasicTex_Billboard/VS_Pos-UV_Billboard", EShaderType::Sh_Vertex },
			{ "Pos-UV_BasicTex_Billboard/PS_BasicTex", EShaderType::Sh_Fragment } };

			material->Shader = RHI::Instance->CreateShaderFromPath(shaders, inputLayout);
		}

		eastl::shared_ptr<MeshNode> billboardNode = eastl::make_shared<MeshNode>("Mesh");
		AddChild(billboardNode);

		RenderCommand newCommand;
		newCommand.Material = material;
		newCommand.DataContainer = dataContainer;
		newCommand.Parent = billboardNode;
		newCommand.DrawType = EDrawType::DrawElements;

		ForwardRenderer::Get().AddCommand(newCommand);
	}
};

static eastl::shared_ptr<DeferredDecal> decal;

void TestGameMode::Init()
{
	// Scene setup 
	SceneManager& sManager = SceneManager::Get();
	Scene& currentScene = sManager.GetCurrentScene();
	GameCamera = currentScene.GetCurrentCamera();

	// Position the camera a bit better
	if (TransformObjPtr parentShared = GameCamera->GetParent().lock())
	{
		// Move the camera parent
		parentShared->Move(glm::vec3(-34.f, 51.f, 0.f));
		parentShared->SetRotationDegrees(glm::vec3(-180.f, -90.f, -180.f));
		GameCamera->SetRotationDegrees(glm::vec3(-34.f, 0.f, 0.f));
	}

	{
	 	DirLight = SceneHelper::CreateVisualEntity<LightSource>("Directional Light");
	 	DirLight->LData.Type = ELightType::Directional;
	 	DirLight->SetRelativeLocation({ -2.0f, 20.0f, -1.0f });
	 	//DirLight->SetRotationDegrees(glm::vec3(90.f, 90.f, 0.f));
	}

	constexpr float linear = 0.0014f;
	constexpr float quadratic = 0.000007f;

	FloorModel = SceneHelper::CreateVisualEntity<AssimpModel3D>("../Data/Models/Sponza/Sponza.gltf", "Sponza");
	FloorModel->Move(glm::vec3(0.f, -2.f, 0.f));
	FloorModel->SetScale(glm::vec3(10.f, 10.f, 10.f));

	SphereModel = SceneHelper::CreateVisualEntity<AssimpModel3DEditorSphere>("../Data/Models/Sphere/scene.gltf", "EditorSphere");
	SphereModel->Move(glm::vec3(0.f, -2.f, 0.f));

	decal = SceneHelper::CreateVisualEntity<DeferredDecal>("Decal");
	decal->Move(glm::vec3(43.f, 2.7f, -3.f));
	decal->SetScale(glm::vec3(15.f, 9.f, 21.f));

}

void TestGameMode::Tick(float inDeltaT)
{
	ImGui::Begin("Game Tick Stuff");

	DirLight->SetRotationDegrees(glm::vec3(90.f, 0.f, 0.f));

	//Controller->ExecuteCallbacks();

	//AssimpModel->Rotate(0.1f, glm::vec3(1.f, 0.f, 0.f));

	//FloorModel->Rotate(0.1f, glm::vec3(1.f, 0.f, 0.f));

	//Quad->Rotate(0.1f, glm::vec3(-1.f, 0.f, 0.f));

	static const glm::vec3 standardForward = glm::vec3(0.f, 0.f, 1.f);
	static const glm::vec3 standardRight = glm::vec3(1.f, 0.f, 0.f);
	static const glm::vec3 standardUp = glm::vec3(0.f, 1.f, 0.f);

	const eastl::shared_ptr<TransformObject>& usedObj = DirLight;

	if (usedObj)
	{
 		glm::quat rot = usedObj->GetAbsoluteTransform().Rotation;
 
 		// order of operations matters, considered like a matrix
 		glm::vec3 forward = glm::normalize(rot * standardForward);
		glm::vec3 right = glm::normalize(rot * standardRight);
		glm::vec3 up = glm::normalize(rot * standardUp);

 		const glm::vec3 start = usedObj->GetAbsoluteTransform().Translation;
 
		// Rotation test stuff
 		{
 			static const glm::vec3 targetVector = glm::vec3(0.f, 1.f, 0.f);
 			DrawDebugHelpers::DrawDebugLine(start, start + targetVector * 4.f, glm::vec3(0.f, 1.f, 0.f));
 
 			static const glm::vec3 dir =  targetVector - forward;
 			static const float angle = glm::degrees(glm::acos(glm::dot(forward, targetVector)));
 
 			static bool doRotation = false;
 			ImGui::Checkbox("Do rotation", &doRotation);
 
			static float degrees = 0.f;
			ImGui::SliderFloat("Rotation Lerp", &degrees, 0.f, 180.f);
 			if (doRotation)
 			{
				//const float usedAngleRadians = lerpAmount * lerpAmount;
				//const float usedAngleDegrees = glm::degrees(usedAngleRadians);
				const float usedAngleDegrees = degrees;

 				DirLight->Rotate(usedAngleDegrees, dir);
 			}
 		}

		rot = usedObj->GetAbsoluteTransform().Rotation;

		// order of operations matters, considered like a matrix
		forward = glm::normalize(rot * standardForward);
		right = glm::normalize(rot * standardRight);
		up = glm::normalize(rot * standardUp);
 
 		DrawDebugHelpers::DrawDebugLine(start, start + forward * 4.f, glm::vec3(0.f, 0.f, 1.f));
 		DrawDebugHelpers::DrawDebugLine(start, start + right * 4.f, glm::vec3(1.f, 0.f, 0.f));
 		DrawDebugHelpers::DrawDebugLine(start, start + up * 4.f, glm::vec3(1.f, 1.f, 0.f));
	}


	ImGui::End();
}
