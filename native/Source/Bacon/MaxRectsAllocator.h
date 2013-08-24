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
		
		std::vector<Rect> const& GetFreeRects() const { return m_FreeRects; }
		
	private:
		std::vector<Rect> m_FreeRects;
		
		bool Alloc(Rect& outRect, int width, int height);
		bool GetBestFreeRect(Rect& outRect, int width, int height);
		void AddFreeRect(Rect const& f);
	};
	
}