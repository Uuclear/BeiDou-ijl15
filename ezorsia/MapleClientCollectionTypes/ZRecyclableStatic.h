#pragma once
// ZRecyclableStatic：可回收缓冲区的静态回调注册表。
// 通过 CallBack 链表维护各 ZRecyclableAvBuffer 实例的生命周期回调。
//credits to the creators of https://github.com/MapleStory-Archive/MapleClientEditTemplate
class ZFakeStatAvBuff { };

class ZRecyclableStatic
{
public:
	struct CallBack
	{
	public:
		ZRecyclableStatic::CallBack* m_pNext;

	protected:
		CallBack()
		{
			m_pNext = nullptr;
		}

		virtual ~CallBack() = default;
	};

protected:
	ZRecyclableStatic::CallBack* m_pHead;
};

//assert_size(sizeof(ZFakeStatAvBuff), 0x1)
//assert_size(sizeof(ZRecyclableStatic::CallBack), 0x8)
//assert_size(sizeof(ZRecyclableStatic), 0x4)