// IOCP.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


#include "winsock2.h"									//引用头文件
#include <ws2tcpip.h>
#include <mswsock.h>
#include <process.h>

#pragma comment (lib,"ws2_32.lib")						//链接库文件
// CIOCPApp:
// 有关此类的实现，请参阅 IOCP.cpp
//

class CIOCPApp : public CWinApp
{
public:
	CIOCPApp();

// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CIOCPApp theApp;