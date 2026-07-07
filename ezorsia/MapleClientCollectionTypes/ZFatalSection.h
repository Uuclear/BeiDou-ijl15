#pragma once
// ZFatalSection：致命区/同步区结构，用于可回收缓冲区等的线程互斥保护。
// 包含 ZFatalSectionData 与 ZFatalSection，对应客户端内部同步原语。
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
struct ZFatalSectionData
{
	void* _m_pTIB;
	int _m_nRef;
};

struct ZFatalSection : ZFatalSectionData
{
	/* TODO emulate this class */
};

//assert_size(sizeof(ZFatalSection), 0x8)