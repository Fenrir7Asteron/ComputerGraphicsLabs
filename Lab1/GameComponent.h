#pragma once
class Game;

class GameComponent
{
public:
	GameComponent(Game* game);
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;

protected:
	void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);

	Game* game_;
};

