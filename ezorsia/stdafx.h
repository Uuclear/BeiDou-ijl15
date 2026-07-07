#pragma once

// stdafx.h — 预编译头（Precompiled Header）
// 集中包含 Windows SDK、Winsock 及项目公共头文件，供各 .cpp 通过 #include "stdafx.h"
// 引用，以加快 MSVC 编译速度并统一依赖顺序（Winsock 须在 windows.h 之前）。

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

// Include before <windows.h> to avoid winsock.h conflicts.
// Needed for hostname->IPv4 resolution (getaddrinfo/InetPton/InetNtop).
#include <winsock2.h>
#include <ws2tcpip.h>

// Windows Header Files
#include <windows.h>

// reference additional headers your program requires here

#include <iostream>
#include "Client.h"
#include "Memory.h"

