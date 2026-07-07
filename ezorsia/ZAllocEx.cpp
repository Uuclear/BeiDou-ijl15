#include "stdafx.h"
#include "MapleClientCollectionTypes\ZAllocEx.h"
// 全局 operator new/delete 实现：将进程内 C++ 堆操作委托给 ZAllocEx<ZAllocAnonSelector>。
// 确保本 DLL 与 MapleStory 客户端使用相同的内存分配器，避免跨模块释放崩溃。
// 覆盖单对象与数组形式的 new/delete。
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