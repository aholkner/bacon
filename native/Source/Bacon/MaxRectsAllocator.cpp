#include "MaxRectsAllocator.h"

namespace Bacon
{
	static const int MinimimFreeRectSize = 8;
	
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

	bool MaxRectsAllocator::Alloc(Rect& r, int width, int height)
	{
		// Get the best fitting free rectangle
		Rect freeRect;
		if (!GetBestFreeRect(freeRect, width, height))
			return false;
			
		// Fit new box within free rect
		r = Rect(freeRect.m_Left, freeRect.m_Top, freeRect.m_Left + width, freeRect.m_Top + height);
		
		// Remove intersecting free rectangles
		size_t freeCount = m_FreeRects.size();
		for (size_t i = 0; i < freeCount && i < m_FreeRects.size(); ++i)
		{
			Rect& f = m_FreeRects[i];
			if (!f.Intersects(r))
				continue;
				
			Rect f1(f.m_Left,	f.m_Top,	r.m_Left,	f.m_Bottom);	// Left
			Rect f2(r.m_Right,	f.m_Top,	f.m_Right,	f.m_Bottom);	// Right
			Rect f3(f.m_Left,	f.m_Top,	f.m_Right,	r.m_Top);		// Top
			Rect f4(f.m_Left,	r.m_Bottom, f.m_Right,	f.m_Bottom);	// Bottom
			
			// Erase f
			m_FreeRects[i] = m_FreeRects.back();
			m_FreeRects.resize(m_FreeRects.size() - 1);
			--i;
			
			// Insert subdivisions of f
			AddFreeRect(f1);
			AddFreeRect(f2);
			AddFreeRect(f3);
			AddFreeRect(f4);
		}
		
		return true;
	}

	static int GetShortestLeftover(Rect const& r, int width, int height)
	{
		return std::min(r.GetWidth() - width, r.GetHeight() - height);
	}

	bool MaxRectsAllocator::GetBestFreeRect(Rect& outRect, int width, int height)
	{
		const Rect* best = nullptr;
		for (Rect const& r : m_FreeRects)
		{
			if (r.GetWidth() < width || r.GetHeight() < height)
				continue;
			
			if (best == nullptr)
				best = &r;
			else if (GetShortestLeftover(r, width, height) < GetShortestLeftover(*best, width, height))
				best = & r;
		}
		if (!best)
			return false;
		
		outRect = *best;
		return true;
	}
	
	void MaxRectsAllocator::AddFreeRect(Rect const& r)
	{
		// Do not add if degenerate, or below minimum size
		if (r.GetWidth() < MinimimFreeRectSize || r.GetHeight() < MinimimFreeRectSize)
			return;
		
		// Do not add if another free rect entirely contains this one
		for (Rect const& c : m_FreeRects)
		{
			if (c.Contains(r))
				return;
		}
		
		m_FreeRects.push_back(r);
	}
}