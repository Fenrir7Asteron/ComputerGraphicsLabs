#pragma once
#include "GameFrameworkExports.h"
#include "SimpleMath.h"
#include <map>
#include <d3d11.h>

class GameFramework;

class BitonicSort
{
	GameFramework* game;

public:
	struct Params
	{
		unsigned int Level;
		unsigned int LevelMask;
		unsigned int Width;
		unsigned int Height;
	};

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

	const int NumberOfElements = 256 * 256 * 16;
	const int BitonicBlockSize = 1024;
	const int TransposeBlockSize = 16;
	const int MatrixWidth = BitonicBlockSize;
	const int MatrixHeight = NumberOfElements / BitonicBlockSize;

	const unsigned int MaxParticlesCount = 256 * 256 * 16;

	ID3D11Buffer* paramsCB;
	ID3D11Buffer* buffer2;
	ID3D11UnorderedAccessView* uavBuf;
	ID3D11ShaderResourceView* srvBuf;

	enum class ComputeFlags
	{
		BITONIC_SORT = 1 << 0,
		TRANSPOSE = 1 << 1,
	};

	std::map<ComputeFlags, ID3D11ComputeShader*> ComputeShaders;

	GAMEFRAMEWORK_API BitonicSort(GameFramework* game);

	GAMEFRAMEWORK_API void Dispose();

	GAMEFRAMEWORK_API void SetConstants(const UINT& level, const UINT& levelMask, const UINT& width, const UINT& height);

	GAMEFRAMEWORK_API void Sort(ID3D11UnorderedAccessView* uav, ID3D11ShaderResourceView* srv);

private:
	void LoadShaders();
	void CreateBuffers();
};

