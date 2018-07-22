
#pragma once
#include "IOCP.h"
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

#define  MAX_THREAD_COUNT   64
#define  MAX_BUFFER_LEN     512
#define  MAX_TIMEOUT        1000
#define  CHECK_CLIENT_TIME        1000*100


//error
#define  IOCP_NO_ERROR            0x00000000
#define  IOCP_ERROR_IP_PORT       0x00000001
#define  IOCP_ERROR_CREATE_IOCP   0x00000002
#define  IOCP_ERROR_ASSOSI_IOCP   0x00000003
#define  IOCP_ERROR_BIND          0x00000004
#define  IOCP_ERROR_LISTEN        0x00000005
#define  IOCP_ERROR_GET_FUNC      0x00000006
#define  IOCP_ERROR_ACCEPT        0x00000007
#define  IOCP_ERROR_SOCKET        0x00000008

#define  WM_SHOW_CLIENT_INFO      WM_USER +100
#define  IOCP_MSG_TYPE_ADD        0x00000001
#define  IOCP_MSG_TYPE_UPDATE     0x00000002

typedef enum _OPERATION_INFO_
{
	OP_NULL,
	OP_ACCEPT,
	OP_SEND,
	OP_RECV
}OPERATIONINFO;

typedef struct _CLIENT_SOCKET_INFO_
{
public:
	_CLIENT_SOCKET_INFO_()
	{
		clean();
	}
	~_CLIENT_SOCKET_INFO_()
	{
		clean();
	}
protected:
	void clean()
	{
		sock = INVALID_SOCKET;
		memset(&addr, 0, sizeof(addr));
		addr.sin_addr.S_un.S_addr = INADDR_ANY;
		addr.sin_port = htons(0);
		addr.sin_family = AF_INET;
	}
public:	
	SOCKET sock;
	SOCKADDR_IN addr;
}CLIENTSOCKETINFO, *LPCLIENTSOCKETINFO;

typedef struct _SOCKET_IO_DTATA_
{
public: 
	_SOCKET_IO_DTATA_()
	{
		clean();
	}
	~_SOCKET_IO_DTATA_()
	{
		clean();
	}
	void clean()
	{
		ZeroMemory(&ol, sizeof(ol));
		memset(buf, 0, sizeof(buf));
		sAccept = INVALID_SOCKET;
		sListen = INVALID_SOCKET;
		wsaBuf.buf = buf;
		wsaBuf.len = MAX_BUFFER_LEN;
		opType =  OP_NULL;
	}
public:
	WSAOVERLAPPED ol;
	SOCKET sAccept; // Only valid with AcceptEx
	SOCKET sListen; // Only valid with AcceptEx
	WSABUF wsaBuf;
	char buf[MAX_BUFFER_LEN];
	OPERATIONINFO opType;
}SOCKETIODATA, *LPSOCKETIODATA;



class IOCPClass
{
public:

	HWND m_hDlgWnd;
	SOCKET m_sListenSocket;
	HANDLE m_hCompPort;
	CRITICAL_SECTION m_cs;
	BOOL m_bIsCreateIOCP;

	SOCKETIODATA m_sIOData;
	HANDLE m_hThread[MAX_THREAD_COUNT];
	DWORD m_nThread;
	HANDLE m_hCheckSockerThread;
	BOOL m_bExitThread;
	DWORD m_ulClientCount;
	list<LPCLIENTSOCKETINFO> m_lsClientSocketList;
	vector<LPSOCKETIODATA> m_veSocketIOData;


public:
	IOCPClass();
	~IOCPClass();

	UINT Create(HWND hDlgWnd,TCHAR *lpszServerIP,UINT ulPort);
	void Release();

	int CharToWideChar(char * pchar,WCHAR * pwchar);
	int WideCharToChar(WCHAR * pwchar, char * pchar);


	BOOL DoAccept(LPCLIENTSOCKETINFO pClientSockInfo,LPSOCKETIODATA pSocketIoData);
	BOOL PostAccept(LPSOCKETIODATA pSockIoData);
	BOOL PostMsgToIOCP(LPCLIENTSOCKETINFO pClientSockInfo,DWORD ulClientIndex,WCHAR *pszText);
	BOOL AddClientInfoToList(char *pszIP);
	BOOL DeleteOneClient(LPCLIENTSOCKETINFO pClientSockInfo,LPSOCKETIODATA pSockIoData = NULL);

};
