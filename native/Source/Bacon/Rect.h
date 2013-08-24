#pragma once

namespace Bacon {

	struct Rect
	{
		Rect(int left, int top, int right, int bottom)
		: m_Left(left)
		, m_Top(top)
		, m_Right(right)
		, m_Bottom(bottom)
		{ }
		
		Rect() { }
		
		int m_Left;
		int m_Top;
		int m_Right;
		int m_Bottom;

		bool IsValid() const { return m_Right > m_Left && m_Bottom > m_Top; }

		int GetWidth() const { return m_Right - m_Left; }
		int GetHeight() const { return m_Bottom - m_Top; }
		int GetArea() const { return GetWidth() * GetHeight(); }

		Rect Inset(int s) const
		{
			return Rect(m_Left + s, m_Top + s, m_Right - s, m_Bottom - s);
		}
		
		Rect Expand(int s) const
		{
			return Rect(m_Left - s, m_Top - s, m_Right + s, m_Bottom + s);
		}
		
		Rect Offset(int s) const
		{
			return Rect(m_Left + s, m_Top + s, m_Right + s, m_Bottom + s);
		}
		
		bool Intersects(Rect const& r) const
		{
			return	(m_Left < r.m_Right &&
					 m_Right > r.m_Left &&
					 m_Top < r.m_Bottom &&
					 m_Bottom > r.m_Top);
		}

		bool Contains(Rect const& r) const
		{
			return	(m_Left <= r.m_Left &&
					 m_Right >= r.m_Right &&
					 m_Top <= r.m_Top &&
					 m_Bottom >= r.m_Bottom);
		}
		
	};

}