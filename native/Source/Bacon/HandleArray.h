#pragma once

#include <vector>

namespace Bacon {
	
	template<typename T>
	class HandleArray
	{
	public:
        HandleArray()
            : m_Free(0xffff)
            , m_NextVersion(1)
			, m_Count(0)
        {
        }

		void Reserve(std::size_t count)
		{
			m_Elements.reserve(count);
		}
		
		int GetCount() const
		{
			return m_Count;
		}
		
		T* Get(int handle)
		{
			int index = GetIndexFromHandle(handle);
			if (index < 0)
				return nullptr;
			
			return &m_Elements[index].m_Value;
		}
		
		int GetHandle(T* value)
		{
			if (value < &m_Elements[0].m_Value || value >= &m_Elements[0].m_Value + m_Elements.size())
				return 0;
			
			unsigned short index = (unsigned short)(value - &m_Elements[0].m_Value);
			return index | (m_Elements[index].m_Version << 16);
		}
		
		int Alloc()
		{
			int index;
			if (m_Free != 0xffff)
			{
				index = m_Free;
				m_Free = m_Elements[index].m_NextFree;
				m_Elements[index].m_NextFree = 0xffff;
				new (&m_Elements[index].m_Value) T;
				++m_Count;
				return CreateHandle(index);
			}
			
			if (m_Elements.size() >= 0xfffe)
				return 0;
			
			index = (int)m_Elements.size();
			m_Elements.push_back(Element(T(), 0, 0xffff));
			new (&m_Elements[index].m_Value) T;
			++m_Count;
			return CreateHandle(index);
		}
		
		bool Free(int handle)
		{
			int index = GetIndexFromHandle(handle);
			if (index < 0)
				return false;
			
			Element& element = m_Elements[index];
			element.m_NextFree = m_Free;
			element.m_Value.~T();
			
			#if DEBUG
			memset(&element.m_Value, 0xdd, sizeof(T));
			#endif
			
			m_Free = index;
			--m_Count;
			return true;
		}
		
		class Iterator
		{
		public:
			Iterator(HandleArray<T>& array, int index)
			: m_Array(array)
			, m_Index(index)
			{ }
			
			T* operator->() { return &m_Array.m_Elements[m_Index].m_Value; }
			T& operator*() { return m_Array.m_Elements[m_Index].m_Value; }
			
			bool operator!=(const Iterator& other) const
			{
				return (&m_Array != &other.m_Array ||
						m_Index != other.m_Index);
			}
			
			Iterator& operator++()
			{
				do {
					++m_Index;
				} while (m_Index < m_Array.m_Elements.size() &&
						 m_Array.m_Elements[m_Index].m_NextFree != 0xffff);
				return *this;
			}
			
		private:
			HandleArray<T>& m_Array;
			int m_Index;
		};
		
		Iterator begin()
		{
			return ++Iterator(*this, -1);
		}
		
		Iterator end()
		{
			return Iterator(*this, (int)m_Elements.size());
		}
		
	private:
		struct Element
		{
            Element(const T& value, unsigned short version, unsigned short nextFree)
                : m_Value(value)
                , m_Version(version)
                , m_NextFree(nextFree)
            {
            }

			T m_Value;
			unsigned short m_Version;
			unsigned short m_NextFree;		// 0xffff if occupied
		};
		
		std::vector<Element> m_Elements;
		unsigned short m_Free;		// 0xffff if none free
		unsigned short m_NextVersion;
		int m_Count;
		
		int CreateHandle(unsigned short index)
		{
			m_Elements[index].m_Version = m_NextVersion;
			return index | (m_NextVersion++ << 16);
		}
		
		int GetIndexFromHandle(int handle)
		{
			int index = handle & 0xffff;
			if (index >= (int)m_Elements.size() ||
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