#include "stdafx.h"
#include "ZAllocEx.h"
// 重载全局 operator new/delete，将 C++ 堆分配路由至 ZAllocEx<ZAllocAnonSelector>。
// 使通过 new 创建的对象与 MapleStory 客户端共用同一套内存管理器。
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
void* operator new(size_t uSize)
{
	return ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(uSize);
}

void* operator new[](size_t uSize)
{
	return ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(uSize);
}

void operator delete(void* p)
{
	ZAllocEx<ZAllocAnonSelector>::GetInstance()->Free((void**)p);
}

void operator delete[](void* p)
{
	ZAllocEx<ZAllocAnonSelector>::GetInstance()->Free((void**)p);
}