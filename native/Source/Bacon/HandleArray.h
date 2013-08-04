#pragma once

#include <vector>

namespace Bacon {
	
	template<typename T>
	class HandleArray
	{
	public:
		void Reserve(std::size_t count)
		{
			m_Elements.reserve(count);
		}
		
		T* Get(int handle)
		{
			int index = GetIndexFromHandle(handle);
			if (index < 0)
				return nullptr;
			
			return &m_Elements[index].m_Value;
		}
		
		int Alloc()
		{
			int index;
			if (m_Free != 0xffff)
			{
				index = m_Free;
				m_Free = m_Elements[index].m_NextFree;
				m_Elements[index].m_NextFree = 0xffff;
				return CreateHandle(index);
			}
			
			if (m_Elements.size() >= 0xfffe)
				return 0;
			
			index = (int)m_Elements.size();
			m_Elements.push_back({ T(), 0, 0xffff });
			return CreateHandle(index);
		}
		
		bool Free(int handle)
		{
			int index = GetIndexFromHandle(handle);
			if (index < 0)
				return false;
			
			Element& element = m_Elements[index];
			element.m_NextFree = m_Free;
			m_Free = index;
			return true;
		}
		
		
	private:
		struct Element
		{
			T m_Value;
			unsigned short m_Version;
			unsigned short m_NextFree;		// 0xffff if occupied
		};
		
		std::vector<Element> m_Elements;
		unsigned short m_Free = 0xffff;		// 0xffff if none free
		unsigned short m_NextVersion = 1;
		
		int CreateHandle(unsigned short index)
		{
			m_Elements[index].m_Version = m_NextVersion;
			return index | (m_NextVersion++ << 16);
		}
		
		int GetIndexFromHandle(int handle)
		{
			int index = handle & 0xffff;
			if (index >= m_Elements.size() ||
				m_Elements[index].m_NextFree != 0xffff ||
				m_Elements[index].m_Version != GetVersionFromHandle(handle))
				return -1;
			return index;
		}
		
		int GetVersionFromHandle(int handle)
		{
			return handle >> 16;
		}
	};
	
}