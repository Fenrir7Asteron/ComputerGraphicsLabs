#pragma once
#include "GameFrameworkExports.h"
#include <vector>

class Camera;
class DirectionalLight;
class PointLight;
class RenderComponent;
class ParticleSystemSorted;

namespace RenderAPI
{
	class RenderAPI
	{
	public:
		GAMEFRAMEWORK_API virtual void Render(float deltaTime) = 0;
		GAMEFRAMEWORK_API virtual void Release() = 0;

		GAMEFRAMEWORK_API virtual void SetCamera(Camera* camera);

		Camera* camera;
		DirectionalLight* dirLight;
		std::vector<PointLight*> pointLights;
		std::vector<RenderComponent*> renderComponents;
		std::vector<ParticleSystemSorted*> particleSystems;
	};
}
