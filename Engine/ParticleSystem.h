#pragma once
#include "Component/GameComponent.h"
#include "Component/Render/RenderComponent.h"
#include <map>

class Camera;

class ParticleSystem : public GameComponent, public RenderComponent
{
public:

	ID3D11Buffer* bufFirst, * bufSecond, * countBuf, * injectionBuf, * constBuf;

	ID3D11ShaderResourceView *srvFirst, *srvSecond, *srvSrc, *srvDst;
	ID3D11UnorderedAccessView *uavFirst, *uavSecond, *uavSrc, *uavDst, *injUav;

	const unsigned int MaxParticlesCount = 100000;
	const unsigned int MaxParticlesInjectionCount = 100;
	static const int ThreadInGroupTotal = 256;

	UINT injectionCount = 0;

	int ParticlesCount = 1;

#pragma pack(push, 4)
	struct Particle
	{
		DirectX::SimpleMath::Vector4 Position;
		DirectX::SimpleMath::Vector4 Velocity;
		DirectX::SimpleMath::Vector4 Color0;
		DirectX::SimpleMath::Vector4 Color1;
		DirectX::SimpleMath::Vector4 Color;
		DirectX::SimpleMath::Vector2 Size0Size1;
		float Size;
		float LifeTime;
	};
#pragma pack(pop)

#pragma pack(push, 4)
	struct ConstData
	{
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Matrix View;
		DirectX::SimpleMath::Matrix Projection;
		DirectX::SimpleMath::Vector4 DeltatimeMaxparticlesGroupdim;
	};
#pragma pack(pop)

	enum class ComputeFlags
	{
		INJECTION = 1 << 0,
		SIMULATION = 1 << 1,
		ADD_GRAVITY = 1 << 2,
	};

	std::map<ComputeFlags, ID3D11ComputeShader*> ComputeShaders;

	ConstData constData;

	Particle* injectionParticles = new Particle[MaxParticlesInjectionCount];

	ID3D11VertexShader* vertShader;
	ID3D11GeometryShader* geomShader;
	ID3D11PixelShader* pixShader;

	ID3D11RasterizerState* rastState;
	ID3D11BlendState* blendState;
	ID3D11DepthStencilState* depthState;

	Camera* camera;

	GAMEFRAMEWORK_API ParticleSystem(GameObject* gameObject, Camera* camera);

	GAMEFRAMEWORK_API static void GetGroupSize(int particlesCount, int& groupSizeX, int& groupSizeY);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	GAMEFRAMEWORK_API void LoadShaders();
	GAMEFRAMEWORK_API void CreateBuffers();
	GAMEFRAMEWORK_API void AddParticle(const Particle& p);
	GAMEFRAMEWORK_API void SwapBuffers();

	// Inherited via RenderComponent
	GAMEFRAMEWORK_API virtual void DrawShadowMap() override;
	GAMEFRAMEWORK_API virtual void GeometryPass() override;
};

