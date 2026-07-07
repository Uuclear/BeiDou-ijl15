#pragma once
// ZRefCounted：引用计数基类，提供 m_nRef 及双向链表指针 m_pPrev/m_pNext。
// 供 ZRef、ZList 等容器与智能指针系统使用。
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
class ZRefCounted
{
public:
	ZRefCounted()
	{
		this->m_nRef = 0;
		this->m_pPrev = nullptr;
		this->m_pNext = nullptr;
	}

	virtual ~ZRefCounted() = default;

	union
	{
		volatile long m_nRef;
		ZRefCounted* m_pNext;
	};

	ZRefCounted* m_pPrev;
};

/* not really sure why nexon has this class but this is all it does o__o */
template <class T>
T* ZRefCounted_Alloc()
{
	T* pAlloc = new T();

	return pAlloc;
}

//assert_size(sizeof(ZRefCounted), 0x0C);