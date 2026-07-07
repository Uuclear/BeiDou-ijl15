#pragma once
// ZRefCountedAccessor：引用计数访问器空基类模板，用于 ZRef/ZList 的多重继承布局。
// 配合 ZRefCountedDummy 使非 ZRefCounted 类型也能接入引用计数体系。
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
struct ZRefCountedAccessorBase
{
};


template<class T>
class ZRefCountedAccessor : ZRefCountedAccessorBase
{

};

//assert_size(sizeof(ZRefCountedAccessor<int>), 0x01);
//assert_size(sizeof(ZRefCountedAccessorBase), 0x01);