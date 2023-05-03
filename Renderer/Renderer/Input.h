#pragma once

#include <Windows.h>
#include <stdint.h>
#include <bitset>


namespace end
{
	class Input
	{

	public:
		enum mouse_input_buttons {
			KEY_LEFT_MOUSE_BUTTON = 0, KEY_RIGHT_MOUSE_BUTTON,
			COUNT
		};
		struct MousePos
		{
			int x, y;
		};
		static bool isKeyboardKeyPressed(int keycode);
		static void ProccessKeyboardInput(int VKCode, bool isDown);

		static bool isMouseKeyPressed(int keycode);
		static void ProccessMouseInput(int VKCode, bool isDown);

		static MousePos RetrieveMousePos(bool isCurrent);
		static void UpdateMousePos(bool isCurrent, int x, int y);

	};
}