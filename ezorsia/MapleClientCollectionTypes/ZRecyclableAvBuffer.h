#pragma once
// ZRecyclableAvBuffer：指定类型的可用对象缓冲池，管理 free 链表与分配统计。
// 单例模式，提供 raw_new/raw_delete 供 ZRecyclable 派生类调用。
#include "ZRecyclableStatic.h"
#include "ZFatalSection.h"
#include "ZAllocEx.h"
#include <mutex>
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
#define ZRECYCLABLE_RAW_BLOCK_SIZE 0x10

template <class T>
class ZRecyclableAvBuffer : ZRecyclableStatic::CallBack
{
private:
	ZFakeStatAvBuff* m_pStat;
	ZFatalSection m_l; // TODO maybe emulate this one day
	void* m_pAv;

	ZRecyclableAvBuffer()
	{
		this->m_pAv = nullptr;
		this->m_pStat = nullptr;
		ZeroMemory(&this->m_l, sizeof(ZFatalSection));
	}

	static std::mutex* GetMutex() // instead of ZFatalSection
	{
		static std::mutex mtx;
		return &mtx;
	}

public:
	static ZRecyclableAvBuffer<T>* GetInstance()
	{
		static ZRecyclableAvBuffer<T> s_pInstance = ZRecyclableAvBuffer<T>();

		return &s_pInstance;
	}

	void** raw_new()
	{
		void** pAlloc;

		this->GetMutex()->lock();

		if (!this->m_pAv)
		{
			this->m_pAv = ZAllocBase::AllocRawBlocks(sizeof(T), ZRECYCLABLE_RAW_BLOCK_SIZE);
		}

		pAlloc = (void**)this->m_pAv;
		this->m_pAv = *pAlloc;

		this->GetMutex()->unlock();

		return pAlloc;
	}

	void raw_delete(void* p)
	{
		this->GetMutex()->lock();

		*(void**)p = this->m_pAv;
		this->m_pAv = p;

		this->GetMutex()->unlock();
	}
};

//assert_size(sizeof(ZRecyclableAvBuffer<long>), 0x18)