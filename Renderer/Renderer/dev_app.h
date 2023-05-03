#pragma once
#include <cstdint>
#include <chrono>
#include "math_types.h"
#include <bitset>
#include "renderer.h"

namespace end
{
	// Simple app class for development and testing purposes
	struct dev_app_t
	{
		void update(end::renderer_t* renderer);

		dev_app_t();

		double get_delta_time()const;

		void EnableDebugGrid(bool isEnabled);

		float ClampColor(float c);
		float4 CycleColors(float& r, float& g, float& b);
	};
}