#pragma once
// ============================================================================
// AutoTypes.h - v83 客户端函数类型定义与 Hook 包装
// ============================================================================
// 定义 MapleStory v83 客户端内部函数的 calling convention 函数指针类型，
// 以及对应的原始地址、Hook lambda 包装器。主要用于 WZ 资源管理器（ResMan）、
// 窗口创建、登录流程及字符串池等功能的函数替换 Hook。
// 类型命名遵循客户端反编译符号；Hook lambda 可在调用原函数前后插入自定义逻辑。
// ============================================================================
#include "MapleClientCollectionTypes/ZXString.h"

// CWnd::CreateWnd - 客户端 UI 窗口创建（__fastcall）
// pThis=窗口对象, nLeft/nTop/nWidth/nHeight=位置尺寸, bScreenCoord=是否屏幕坐标
typedef void(__fastcall* _CWndCreateWnd_t)(void* pThis, void* edx, int nLeft, int nTop, int nWidth, int nHeight, int z, int bScreenCoord, void* esi, int bSetFocus);
static auto _CWndCreateWnd = reinterpret_cast<_CWndCreateWnd_t>(0x009DE4D2); //thanks you teto for helping me on this learning journey
// Hook 包装：先执行原函数，可在 lambda 内修改传入参数以改变窗口布局
static _CWndCreateWnd_t _CWndCreateWnd_Hook = [](void* pThis, void* edx, int nLeft, int nTop, int nWidth, int nHeight, int z, int bScreenCoord, void* esi, int bSetFocus)
-> void {_CWndCreateWnd(pThis, edx, nLeft, nTop, nWidth, nHeight, z, bScreenCoord, esi, bSetFocus); //changing the values here will modify the result of the function
						//usually put changes here //this only runs after the original execution
};

// CLogin::SendSelectCharPacket - 登录选角后发送选角封包
typedef void(__fastcall* _CLoginSendSelectCharPacket_t)(void* pThis, void* edx);
static auto _CLoginSendSelectCharPacket = reinterpret_cast<_CLoginSendSelectCharPacket_t>(0x005F726D);
static _CLoginSendSelectCharPacket_t _CLoginSendSelectCharPacket_Hook = [](void* pThis, void* edx)
-> void {_CLoginSendSelectCharPacket(pThis, edx); //changing the values here will modify the result of the function
//Client::loggedIn = 1;						//usually put changes here
//Client::UpdateResolution();	//tried to get client to change to different res than login while in main game. failed. this is what's left
};

// IWzResMan 资源管理器初始化参数标志（序列化/重解析行为）
enum RESMAN_PARAM {
	RC_AUTO_SERIALIZE = 0x1,
	RC_AUTO_SERIALIZE_NO_CACHE = 0x2,
	RC_NO_AUTO_SERIALIZE = 0x4,
	RC_DEFAULT_AUTO_SERIALIZE = 0x0,
	RC_AUTO_SERIALIZE_MASK = 0x7,
	RC_AUTO_REPARSE = 0x10,
	RC_NO_AUTO_REPARSE = 0x20,
	RC_DEFAULT_AUTO_REPARSE = 0x0,
	RC_AUTO_REPARSE_MASK = 0x30,
};

// ===== WZ 资源管理器全局指针 =====
// g_rm   - IWzResMan 实例指针的存储地址（客户端全局单例）
// g_root - IWzNameSpace 根命名空间指针
// pNameSpace - PcSetRootNameSpace 函数指针表地址
auto g_rm = (void**)0x00BF14E8; //static?
auto g_root = (void**)0x00BF14E0;
auto pNameSpace = 0x00BF0CD0;

// PcCreateObject 调用时的通用外层对象与挂载优先级占位参数
void* pUnkOuter = 0;
void* nPriority = 0;

// CWvsApp::InitializeResMan - 客户端启动时初始化 WZ 资源管理器的入口
typedef void(__fastcall* _CWvsApp__InitializeResMan_t)(void* pThis, void* edx);	//ty to all the contributors of the ragezone release: Client load .img instead of .wz v62~v92 //ty y785	//char* ecx, char* edx
static auto _CWvsApp__InitializeResMan = reinterpret_cast<_CWvsApp__InitializeResMan_t>(0x009F7159);

// PcCreateObject<IWzResMan> - COM 工厂：创建 WZ 资源管理器对象
typedef void(__cdecl* _PcCreateObject_IWzResMan_t)(const wchar_t* sUOL, void* pObj, void* pUnkOuter);
static auto _PcCreateObject_IWzResMan = reinterpret_cast<_PcCreateObject_IWzResMan_t>(0x009FAF55);
static _PcCreateObject_IWzResMan_t _PcCreateObject_IWzResMan_Hook = [](const wchar_t* sUOL, void* pObj, void* pUnkOuters) {
//-> void {_PcCreateObject_IWzResMan(sUOL, pObj, pUnkOuter); //remove -> part and redefine to replace parts of execution code proper
	_PcCreateObject_IWzResMan(sUOL, pObj, pUnkOuter);
};

// PcCreateObject<IWzNameSpace> - COM 工厂：创建 WZ 命名空间对象
typedef void(__cdecl* _PcCreateObject_IWzNameSpace_t)(const wchar_t* sUOL, void* pObj, void* pUnkOuter);
static auto _PcCreateObject_IWzNameSpace = reinterpret_cast<_PcCreateObject_IWzNameSpace_t>(0x009FAFBA);
static _PcCreateObject_IWzNameSpace_t _PcCreateObject_IWzNameSpace_Hook = [](const wchar_t* sUOL, void* pObj, void* pUnkOuters) {
//-> void {_PcCreateObject_IWzNameSpace(sUOL, pObj, pUnkOuter);
	_PcCreateObject_IWzNameSpace(sUOL, pObj, pUnkOuter);
};

// PcCreateObject<IWzFileSystem> - COM 工厂：创建 WZ 文件系统对象（.wz / 文件夹挂载）
typedef void(__cdecl* _PcCreateObject_IWzFileSystem_t)(const wchar_t* sUOL, void* pObj, void* pUnkOuter);
static auto _PcCreateObject_IWzFileSystem = reinterpret_cast<_PcCreateObject_IWzFileSystem_t>(0x009FB01F);
static _PcCreateObject_IWzFileSystem_t _PcCreateObject_IWzFileSystem_Hook = [](const wchar_t* sUOL, void* pObj, void* pUnkOuters) {
//-> void {_PcCreateObject_IWzFileSystem(sUOL, pObj, pUnkOuter);
	_PcCreateObject_IWzFileSystem(sUOL, pObj, pUnkOuter);
 };

// CWvsApp::Dir_BackSlashToSlash - 将路径中的反斜杠替换为正斜杠
typedef void(__cdecl* _CWvsApp__Dir_BackSlashToSlash_t)(char* sDir);
static auto _CWvsApp__Dir_BackSlashToSlash = reinterpret_cast<_CWvsApp__Dir_BackSlashToSlash_t>(0x009F95FE);
static _CWvsApp__Dir_BackSlashToSlash_t _CWvsApp__Dir_BackSlashToSlash_Hook = [](char* sDir) {
//-> void {_CWvsApp__Dir_BackSlashToSlash(sDir); 
	int myLength = strlen(sDir);
	for (int i = 0; i < myLength; ++i) { if (sDir[i] == '\\') { sDir[i] = '/'; } }
};

// CWvsApp::Dir_upDir - 将路径向上一级目录（去掉末尾文件夹名）
typedef void(__cdecl* _CWvsApp__Dir_upDir_t)(char* sDir);
static auto _CWvsApp__Dir_upDir = reinterpret_cast<_CWvsApp__Dir_upDir_t>(0x009F9644);
static _CWvsApp__Dir_upDir_t _CWvsApp__Dir_upDir_Hook = [](char* sDir) {
//-> void {_CWvsApp__Dir_upDir(sDir); 
	_CWvsApp__Dir_upDir(sDir);
};

// _bstr_ctor / Ztl_bstr_t 构造函数 - 将 char* 路径包装为 COM BSTR
typedef char*(__fastcall* _bstr_ctor_t)(void* pThis, void* edx, const char* str);
static auto _bstr_ctor = reinterpret_cast<_bstr_ctor_t>(0x00406301);
static _bstr_ctor_t _bstr_ctor_Hook = [](void* pThis, void* edx, const char* str) {
	return _bstr_ctor(pThis, edx, str); };

//Ztl_bstr_t
//Ztl_variant_t
//_Ztl_bstr__ctor_Ztl_bstr_t
//_com_ptr_t
//_Ztl_variant_t__dtor_t

//ZXString::assign	00414617 ///v83	//raw data from notes kept for reference in case needed later
//void __thiscall ZXString<char>::Assign(ZXString<char> *this, const char* s, int n) ?	v95
//int? __thiscall ZXString<char>::Assign(void* this, char* a2, size_t a3)	v83

//typedef _com_ptr_t* (__cdecl* _get_unknown_t)(_com_ptr_t* result, Ztl_variant_t* v);
//static auto _get_unknown = reinterpret_cast<_get_unknown_t>(0x00414ADA);
//static _get_unknown_t _get_unknown_Hook = [](_com_ptr_t* result, Ztl_variant_t* v) {
//	return _get_unknown(result, v); };
//
//typedef Ztl_variant_t* (__cdecl* _get_resource_object_t)(Ztl_variant_t* result, Ztl_bstr_t sUOL); //sUOL example L"NameSpace#FileSystem"
//static auto _get_resource_object = reinterpret_cast<_get_resource_object_t>(0x00000000);
//static _get_resource_object_t _get_resource_object_Hook = [](Ztl_variant_t* result, Ztl_bstr_t sUOL) {
//	return _get_resource_object(result, sUOL); };
//
//typedef void (__thiscall* _com_ptr_t_IWzProperty__ctor_t)(_com_ptr_t* pThis, _com_ptr_t* p);
//static auto _com_ptr_t_IWzProperty__ctor = reinterpret_cast<_com_ptr_t_IWzProperty__ctor_t>(0x00000000);
//static _com_ptr_t_IWzProperty__ctor_t _com_ptr_t_IWzProperty__ctor_Hook = [](_com_ptr_t* pThis, _com_ptr_t* p) {
//	return _com_ptr_t_IWzProperty__ctor(pThis, p); };
//
//typedef void(__thiscall* _com_ptr_t_IWzProperty__dtor_t)(_com_ptr_t* pThis);
//static auto _com_ptr_t_IWzProperty__dtor = reinterpret_cast<_com_ptr_t_IWzProperty__dtor_t>(0x00000000);
//static _com_ptr_t_IWzProperty__dtor_t _com_ptr_t_IWzProperty__dtor_Hook = [](_com_ptr_t* pThis) {
//	return _com_ptr_t_IWzProperty__dtor(pThis); };
	
// StringPool::GetString - 按索引从客户端字符串池取本地化文本（可用于汉化 Hook）
typedef ZXString<char>*(__fastcall* _StringPool__GetString_t)(void* pThis, void* edx, ZXString<char>* result, unsigned int nIdx, char formal); //also ty to the creators of MapleClientEditTemplate
static auto _StringPool__GetString = reinterpret_cast<_StringPool__GetString_t>(0x0079E993);//hook stringpool modification //ty !! popcorn //ty darter //ty teto

// IWzFileSystem::Init - 以本地路径初始化 WZ 文件系统（.wz 或文件夹）
typedef HRESULT(__fastcall* _IWzFileSystem__Init_t)(void* pThis, void* edx, void* sPath);	//HRESULT
static auto _IWzFileSystem__Init = reinterpret_cast<_IWzFileSystem__Init_t>(0x009F7964);
static _IWzFileSystem__Init_t _IWzFileSystem__Init_Hook = [](void* pThis, void* edx, void* sPath) {
//-> HRESULT {_IWzFileSystem__Init(pThis, edx, sPath);	//HRESULT
//std::cout << "_IWzFileSystem__Init " << " pThis: " << pThis << " edx: " << edx << " sPath: " << sPath << std::endl;
	return _IWzFileSystem__Init(pThis, edx, sPath);
};

// IWzNameSpace::Mount - 将文件系统挂载到命名空间路径（如 "/" 或 "/Data"）
typedef HRESULT(__fastcall* _IWzNameSpace__Mount_t)(void* pThis, void* edx, void* sPath, void* pDown, int nPriority); //HRESULT
static auto _IWzNameSpace__Mount = reinterpret_cast<_IWzNameSpace__Mount_t>(0x009F790A);
static _IWzNameSpace__Mount_t _IWzNameSpace__Mount_Hook = [](void* pThis, void* edx, void* sPath, void* pDown, int nPriority) {
//-> HRESULT {_IWzNameSpace__Mount(pThis, edx, sPath, pDown, nPriority); //HRESULT //return _IWzNameSpace__Mount(pThis, edx, sPath, pDown, nPriority);
	return _IWzNameSpace__Mount(pThis, edx, sPath, pDown, nPriority);
};

// _lpfn_NextLevel - WZ 属性遍历时获取下一层级节点的回调函数指针
typedef void* (__cdecl* _lpfn_NextLevel_t)(int[]); 
static auto _lpfn_NextLevel = reinterpret_cast<_lpfn_NextLevel_t>(0x0078C8A6);