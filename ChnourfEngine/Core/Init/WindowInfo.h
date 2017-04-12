#pragma once
#include <string>

namespace Core
{
	struct WindowInfo
	{
		std::string name;
		int width;
		int height;
		int positionX;
		int positionY;
		bool isReshapable;

		WindowInfo()
		{
			//name = "Chnourf Engine";
			//width = 800;
			//height = 600;
			//positionX = 200;
			//positionX = 200;
			//isReshapable = true;
		}

		WindowInfo(std::string aName, int aStartPositionX, int aStartPositionY, int aWidth, int aHeight, bool aIsReshapable) :
			name(aName),
			positionX(aStartPositionX),
			positionY(aStartPositionY),
			width(aWidth),
			height(aHeight),
			isReshapable(aIsReshapable)
		{}

		WindowInfo(const WindowInfo& aWindowInfo) :
			name(aWindowInfo.name),
			positionX(aWindowInfo.positionX),
			positionY(aWindowInfo.positionY),
			width(aWindowInfo.width),
			height(aWindowInfo.height),
			isReshapable(aWindowInfo.isReshapable)
		{}

		void operator=(const WindowInfo& windowInfo)
		{
			name = windowInfo.name;
			positionX = windowInfo.positionX;
			positionY = windowInfo.positionY;
			width = windowInfo.width;
			height = windowInfo.height;
			isReshapable = windowInfo.isReshapable;
		}

	};
}