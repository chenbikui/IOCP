// IOCP.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


#include "winsock2.h"									//����ͷ�ļ�
#include <ws2tcpip.h>
#include <mswsock.h>
#include <process.h>

#pragma comment (lib,"ws2_32.lib")						//���ӿ��ļ�
// CIOCPApp:
// �йش����ʵ�֣������ IOCP.cpp
//

class CIOCPApp : public CWinApp
{
public:
	CIOCPApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CIOCPApp theApp;