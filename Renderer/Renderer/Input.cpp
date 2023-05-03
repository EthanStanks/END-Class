#include "Input.h"

namespace end
{
	std::bitset<256> keyboard_bitset;
	std::bitset<15> mouse_bitset;

	Input::MousePos mouseCurrentPosition;
	Input::MousePos mousePrevPosition;

	bool Input::isKeyboardKeyPressed(int keycode)
	{
		return keyboard_bitset[keycode];
	}

	bool Input::isMouseKeyPressed(int keycode)
	{
		return mouse_bitset[keycode];
	}
	void Input::ProccessMouseInput(int VKCode, bool isDown)
	{
		mouse_bitset[VKCode] = isDown;
	}

	void Input::ProccessKeyboardInput(int VKCode, bool isDown)
	{
		keyboard_bitset[VKCode] = isDown;
	}

	Input::MousePos Input::RetrieveMousePos(bool isCurrent)
	{
		if (isCurrent)
			return mouseCurrentPosition;
		else return mousePrevPosition;
	}
	void Input::UpdateMousePos(bool isCurrent, int x, int y)
	{
		if (isCurrent)
		{
			mouseCurrentPosition.x = x;
			mouseCurrentPosition.y = y;
		}
		else
		{
			mousePrevPosition.x = x;
			mousePrevPosition.y = y;
		}
	}
}