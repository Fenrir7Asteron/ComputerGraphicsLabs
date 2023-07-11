#pragma once
#include <vector>
#include "d3d11.h"
#include "magic_enum.hpp"

using namespace magic_enum::bitwise_operators;

class MacrosParser
{
public:
	template <typename T>
	static std::vector<D3D_SHADER_MACRO> GetMacros(T flags)
	{
		std::vector<D3D_SHADER_MACRO> macros;

		constexpr auto& entries = magic_enum::enum_entries<T>();

		for (const std::pair<T, std::string_view>& p : entries)
		{
			if (static_cast<uint32_t>(flags & p.first) > 0)
			{
				D3D_SHADER_MACRO macro = { p.second.data() , "1" };
				macros.push_back(macro);
			}
		}

		macros.push_back({ NULL, NULL });

		return macros;
	}
};

