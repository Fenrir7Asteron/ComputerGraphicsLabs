#define NOMINMAX
#define _USE_MATH_DEFINES

#include "SandboxGame.h"

#include <memory>
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	SandboxGame game("Sandbox");
	game.Init(800, 800);
	game.Run();
	return 0;
}