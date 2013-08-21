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
		if (Alloc(outRect, width + margin * 2, height + margin * 2))
		{
			outRect = outRect.Inset(margin);
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
				outRect = Rect(r.m_Left, r.m_Top, r.m_Left + width, r.m_Top + height);
				r.m_Left += width;
				return true;
			}
		}
		return false;
	}

}