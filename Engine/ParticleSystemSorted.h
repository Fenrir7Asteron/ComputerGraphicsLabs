#pragma once

#include "GameComponent.h"
#include <map>
#include "BitonicSort.h"

class Camera;

class ParticleSystemSorted : public GameComponent
{
public:
	ID3D11Buffer *particlesPool, *sortBuffer, *deadBuf, *injectionBuf, *constBuf, *countBuf, *debugBuf;

	ID3D11ShaderResourceView* srvSorted, * srvPool;
	ID3D11UnorderedAccessView* uavDead, * uavSorted, *uavSortedRW, *injUav, *uavPool;

	DirectX::SimpleMath::Vector3 Position;
	float Width, Height, Length;

	const unsigned int MaxParticlesCount = 256 * 256 * 16;
	const unsigned int MaxParticlesInjectionCount = 10000;
	static const int ThreadInGroupTotal = 256;
	UINT ParticlesCount = 0;
	UINT ParticlesDeadCount = 0;
	UINT injectionCount = 0;

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
		DirectX::SimpleMath::Vector4 DeltaTimeMaxParticlesGroupdim;
		DirectX::SimpleMath::Vector4 CameraPosX;
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

	BitonicSort* sort;

	GAMEFRAMEWORK_API ParticleSystemSorted(GameFramework* game,
		GameComponent* parent,
		DirectX::SimpleMath::Vector3 position,
		DirectX::SimpleMath::Quaternion rotation,
		DirectX::SimpleMath::Vector3 scale,
		Camera* camera);

	GAMEFRAMEWORK_API static void GetGroupSize(int particlesCount, int& groupSizeX, int& groupSizeY);

	GAMEFRAMEWORK_API void Update(float deltaTime) override;
	GAMEFRAMEWORK_API void GeometryPass() override;

	GAMEFRAMEWORK_API void AddParticle(const Particle& p);

private:
	void LoadShaders();
	void CreateBuffers();
};

