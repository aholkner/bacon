#include "MaxRectsAllocator.h"

namespace Bacon
{
	
	void MaxRectsAllocator::Init(int width, int height)
	{
		m_FreeRects.clear();
		m_FreeRects.push_back(Rect(0, 0, width, height));
	}
	
	bool MaxRectsAllocator::Alloc(Rect& outRect, int width, int height, int margin)
	{
		if (Alloc(outRect, width, height))
		{
			outRect.Inset(margin);
			return true;
		}
		return false;
	}

	bool MaxRectsAllocator::Alloc(Rect& outRect, int width, int height)
	{
		for (Rect& r : m_FreeRects)
		{
			if (r.GetWidth() >= width && r.GetHeight() >= height)
			{
				outRect = Rect(r.m_Left, r.m_Top, width, height);
				m_FreeRects.clear();
				return true;
			}
		}
		return false;
	}

}