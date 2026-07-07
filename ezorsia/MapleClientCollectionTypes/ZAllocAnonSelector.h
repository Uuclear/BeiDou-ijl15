#pragma once
// ZAllocAnonSelector：ZAllocEx 的匿名内存池块大小选择器。
// 根据 BLOCK_SIZE 索引返回对应的块大小与每批分配块数量。
// 用于通用（非字符串）对象的内存池分配策略。
#include "ZAllocBase.h"
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
struct ZAllocAnonSelector
{
protected:
	unsigned int GetBlockSize(ZAllocBase::BLOCK_SIZE nIndex, int* nAllocBlocks)
	{
		switch (nIndex)
		{
		case ZAllocBase::BLOCK_SIZE::BLOCK16:
			*nAllocBlocks = 64;
			return 16;
		case ZAllocBase::BLOCK_SIZE::BLOCK32:
			*nAllocBlocks = 32;
			return 32;
		case ZAllocBase::BLOCK_SIZE::BLOCK64:
			*nAllocBlocks = 16;
			return 64;
		case ZAllocBase::BLOCK_SIZE::BLOCK128:
			*nAllocBlocks = 8;
			return 128;
		default:
			*nAllocBlocks = 0;
			return 0;
		}
	}
};
