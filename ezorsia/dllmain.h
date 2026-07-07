#pragma once

// dllmain.h
// DLL 入口模块对外暴露的辅助函数声明。

// 创建调试控制台并将标准输出重定向到控制台窗口。
void CreateConsole();

// 安装 ijl15 导出函数转发 Hook（声明保留，实际由 ijl15 模块实现）。
void CreateHook();
