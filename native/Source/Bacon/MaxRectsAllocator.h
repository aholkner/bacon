#pragma once

#include "Rect.h"

#include <vector>

namespace Bacon
{
	class MaxRectsAllocator
	{
	public:
		void Init(int width, int height);
		bool Alloc(Rect& outRect, int width, int height, int margin);
		
	private:
		std::vector<Rect> m_FreeRects;
		
		bool Alloc(Rect& outRect, int width, int height);
	};
	
}