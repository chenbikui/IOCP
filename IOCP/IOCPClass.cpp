#include "stdafx.h"
#include "IOCPClass.h"



IOCPClass::IOCPClass()
{
	m_sListenSocket = INVALID_SOCKET;
	m_hCompPort = NULL;
	m_nThread = 0;
	m_hCheckSockerThread = NULL;
	m_bExitThread = FALSE;
	m_bIsCreateIOCP = FALSE;
	m_ulClientCount = 0;
	m_lsClientSocketList.clear();
	m_veSocketIOData.clear();
}

IOCPClass::~IOCPClass()
{
	Release();
}


LPFN_ACCEPTEX lpfnAcceptEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs = NULL;
GUID GuidAcceptEx = WSAID_ACCEPTEX;
GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

int IOCPClass::CharToWideChar(char * pchar,WCHAR * pwchar)
{
	int dwnum = MultiByteToWideChar(CP_ACP,0,pchar,-1,NULL,0);
	MultiByteToWideChar(CP_ACP,0,pchar,-1,pwchar,dwnum);
	pwchar[dwnum]='\0';
	return dwnum-1;
}

int IOCPClass::WideCharToChar(WCHAR * pwchar, char * pchar)
{
	int dwnum = WideCharToMultiByte(CP_ACP,0,pwchar,-1,NULL,0,NULL,FALSE);
	WideCharToMultiByte(CP_ACP,0,pwchar,-1,pchar,dwnum,NULL,FALSE);
	pchar[dwnum]='\0';
	return dwnum-1;
}

char* IOCPClass::GetLocalIPAddress()
{
	char host_name[225] = {0};

	if(gethostname(host_name,sizeof(host_name))==SOCKET_ERROR)
	{
		return NULL; 
	}

	struct hostent *phe=gethostbyname(host_name);
	if(phe==0)
	{
		return NULL; 
	}

	struct in_addr addr;
	memcpy(&addr,phe->h_addr_list[0],sizeof(struct in_addr)); 
	return inet_ntoa(addr); 
}



unsigned int __stdcall CheckClientSocketIsAliveThread(LPVOID lParam)
{
	IOCPClass *pDlg = (IOCPClass*)lParam;
	list<LPCLIENTSOCKETINFO>::iterator itSocket;
	vector<LPSOCKETIODATA>::iterator itData;
	LPCLIENTSOCKETINFO pClientSocket = NULL; 
	LPSOCKETIODATA pDataInfo = NULL;
	int i = 0;
	DWORD dwTime = 0;
	while(!pDlg->m_bExitThread)
	{
		//等待时间
		dwTime = 0;
		while(dwTime < CHECK_CLIENT_TIME)
		{
			if (pDlg->m_bExitThread)
			{
				return 0;
			}
			Sleep(100);
			dwTime += 100;
		}

		itSocket = pDlg->m_lsClientSocketList.begin();
		for (i = 0;i < (int)pDlg->m_ulClientCount;i++)
		{
			TRACE(L"m_lsClientSocketList size: %d\n", pDlg->m_lsClientSocketList.size());
			pClientSocket = *itSocket;

			pDlg->PostMsgToIOCP(pClientSocket,i,L"IS ALIVE");
			itSocket++;
			Sleep(50);
		}

	}
	return 0;
}

unsigned int __stdcall ThreadProc(LPVOID lParam)
{
	IOCPClass *pDlg = (IOCPClass*)lParam;

	LPCLIENTSOCKETINFO pClientSocketInfo = NULL;
	LPSOCKETIODATA pSocketIoData = NULL;
	WSAOVERLAPPED* lpOverlapped = NULL;
	DWORD dwTrans = 0;
	DWORD dwFlags = 0;
	while(!pDlg->m_bExitThread)
	{
		dwTrans = 0;
		BOOL bRet = GetQueuedCompletionStatus(pDlg->m_hCompPort, &dwTrans, (PULONG_PTR)&pClientSocketInfo, &lpOverlapped, MAX_TIMEOUT);
		if(!bRet)
		{
			if (lpOverlapped != NULL)
			{
				TRACE(L"process a failed completed I/O request with error:%u\n",GetLastError());
			}
			else
			{

				if(WAIT_TIMEOUT == GetLastError())
				{
					TRACE(L"Time-out while waiting for completed I/O entry with error:%u\n",GetLastError());
				}
			}

			TRACE(L"GetQueuedCompletionStatus failed with error: %u\n", GetLastError());
			continue;
		}
		else
		{
			pSocketIoData = CONTAINING_RECORD(lpOverlapped, SOCKETIODATA, ol);
			if(NULL == pSocketIoData)
			{
				// Exit thread
				break;
			}

			if((0 == dwTrans) && ( OP_SEND == pSocketIoData->opType || OP_RECV == pSocketIoData->opType))
			{
				// Client leave.
				TRACE(L"Client: <%s : %d> leave.\n", inet_ntoa(pClientSocketInfo->addr.sin_addr), ntohs(pClientSocketInfo->addr.sin_port));
				//closesocket(pClientSocketInfo->sock);
				continue;
			}
			else
			{
				switch(pSocketIoData->opType)
				{
				case OP_ACCEPT: // Accept
					{	
						pDlg->DoAccept(pClientSocketInfo,pSocketIoData);
					}
					break;

				case OP_SEND:
					{
						TRACE(L"recv client <%s : %d> data: %s\n", inet_ntoa(pClientSocketInfo->addr.sin_addr), ntohs(pClientSocketInfo->addr.sin_port), pSocketIoData->buf);
						//这里设置为OP_RECV，是为了发送消息之后，投递一个WSARecv消息，为了处理接收一下client发送的消息
						pSocketIoData->opType = OP_RECV;
						memset(&(pSocketIoData->ol), 0, sizeof(pSocketIoData->ol));
						WSABUF wsaBufSend;
						char szSendBuf[MAX_BUFFER_LEN] = {0};
						memcpy(szSendBuf,"recv:",sizeof("recv:"));
						strcat_s(szSendBuf,sizeof(szSendBuf),pSocketIoData->wsaBuf.buf);
						wsaBufSend.buf = szSendBuf;
						wsaBufSend.len =(u_long) strlen(szSendBuf);
						memset(pSocketIoData->buf, 0, sizeof(pSocketIoData->buf));
						pSocketIoData->wsaBuf.buf = pSocketIoData->buf;
						if(SOCKET_ERROR == WSASend(pClientSocketInfo->sock,&wsaBufSend/* &(pSocketIoData->wsaBuf)*/, 1, &dwTrans, dwFlags, &(pSocketIoData->ol), NULL))
						{
							if(WSA_IO_PENDING != WSAGetLastError())
							{
								TRACE(L"WSASend failed with error code: %d.\n", WSAGetLastError());
								pDlg->DeleteOneClient(pClientSocketInfo,pSocketIoData);
								continue;
							}
						}

						memset(&(pSocketIoData->ol), 0, sizeof(pSocketIoData->ol));
						memset(pSocketIoData->buf, 0, sizeof(pSocketIoData->buf));
						pSocketIoData->wsaBuf.buf = pSocketIoData->buf;
						pSocketIoData->wsaBuf.len = MAX_BUFFER_LEN;

					}

					break;
				case OP_RECV:
					{
						TRACE(L"recv data: %s",pSocketIoData->wsaBuf.buf);
						//接收client发来的消息之后，如果投递一个OP_SEND消息，是把接收的信息再发给client
						pSocketIoData->opType = OP_SEND;
						//接收client发来的消息之后，如果投递一个OP_RECV消息，是为了接收下一个client发送的消息
						//pSocketIoData->opType = OP_RECV;
						dwFlags = 0;			
						dwTrans =  MAX_BUFFER_LEN;
						//WSARecv为投递下一个消息
						if(SOCKET_ERROR == WSARecv(pClientSocketInfo->sock, &(pSocketIoData->wsaBuf), 1, &dwTrans, &dwFlags, &(pSocketIoData->ol), NULL))
						{
							if(WSA_IO_PENDING != WSAGetLastError())
							{
								TRACE(L"WSARecv failed with error code: %d.\n", WSAGetLastError());
								continue;
							}
						}
						//如果投递OP_RECV，可以在这里处理解析client的消息
						//do analysis message.....
					}
					break;

				default:
					break;
				}

			}//switch
		}//else
	}//while

	return 0;
}

BOOL IOCPClass::DoAccept(LPCLIENTSOCKETINFO pClientSockInfo,LPSOCKETIODATA pSocketIoData)
{
    SOCKADDR_IN* remote = NULL;
	SOCKADDR_IN* local = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	lpfnGetAcceptExSockAddrs(pSocketIoData->wsaBuf.buf, pSocketIoData->wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&local, &localLen, (LPSOCKADDR*)&remote, &remoteLen);

	char *pp=inet_ntoa(remote->sin_addr);
	u_short port=ntohs(remote->sin_port);
	TRACE(L"Client <%s : %d> come in.\n", inet_ntoa(remote->sin_addr), ntohs(remote->sin_port));
	TRACE(L"Recv Data: <%s : %d> %s.\n", inet_ntoa(remote->sin_addr), ntohs(remote->sin_port), pSocketIoData->wsaBuf.buf);

	//Add the client socket to container
	LPCLIENTSOCKETINFO pNewClientInfo = new CLIENTSOCKETINFO;
	pNewClientInfo->sock = pSocketIoData->sAccept;
	memcpy(&(pNewClientInfo->addr), remote, sizeof(SOCKADDR_IN));
	m_lsClientSocketList.push_back(pNewClientInfo);
  
	EnterCriticalSection(&m_cs);

	// Associate with IOCP
	if(NULL == CreateIoCompletionPort((HANDLE)(pNewClientInfo->sock), m_hCompPort, (ULONG_PTR)pNewClientInfo, 0))
	{
		TRACE(L"CreateIoCompletionPort failed with error code: %d\n", GetLastError());
		if (pNewClientInfo != NULL)
		{
			closesocket(pNewClientInfo->sock);
			delete pNewClientInfo;
		}
		return FALSE;
	}

	m_ulClientCount++;

	DWORD dwTrans = 0;
	DWORD dwFlags = 0;
	//申请这个内存是为了之后填充投递的IO请求
	//每个客户端对应一个SOCKETIODATA
	//绑定完之后就可以投递Recv了
	LPSOCKETIODATA pNewSockData = new SOCKETIODATA;
	m_veSocketIOData.push_back(pNewSockData);
	pNewSockData->opType = OP_RECV;
	pNewSockData->sAccept = pNewClientInfo->sock;
	//strcpy_s(pNewSockData->buf, MAX_BUFFER_LEN, pSocketIoData->buf);
	//dwTrans = (DWORD)strlen(pNewSockData->buf);
	memset(pNewSockData->buf, 0, sizeof(pNewSockData->buf));
//	pNewSockData->wsaBuf.buf = pNewSockData->buf;
//	dwTrans = pNewSockData->wsaBuf.len = MAX_BUFFER_LEN;
	if(SOCKET_ERROR == WSARecv(pNewSockData->sAccept, &(pNewSockData->wsaBuf), 1, &dwTrans, &dwFlags, &(pNewSockData->ol), NULL))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE(L"WSARecv failed with error code: %d.\n", WSAGetLastError());
			return FALSE;
		}
	}

	LeaveCriticalSection(&m_cs);

	AddClientInfoToList(pp);

	// Post Accept
	memset(&(pSocketIoData->ol), 0, sizeof(pSocketIoData->ol));
	
	return PostAccept(pSocketIoData);
}

BOOL IOCPClass::DeleteOneClient(LPCLIENTSOCKETINFO pClientSockInfo,LPSOCKETIODATA pSockIoData)
{
	//删除socket
	list<LPCLIENTSOCKETINFO>::iterator itSocket;
	LPCLIENTSOCKETINFO pClientSocket = NULL; 
	itSocket = find(m_lsClientSocketList.begin(),m_lsClientSocketList.end(),pClientSockInfo);
	//获取client socket在lsClientSocketList中的位置
	DWORD nPos = (DWORD)distance(m_lsClientSocketList.begin(),itSocket);
	if ( itSocket == m_lsClientSocketList.end())
	{
		return FALSE;
	}
	else
	{
		pClientSocket = *itSocket;
		if (pClientSocket != NULL)
		{
			closesocket(pClientSocket->sock);
			delete pClientSocket;
			pClientSocket = NULL;
		}
		m_lsClientSocketList.erase(itSocket);
	}

	//删除socket对应的数据
	//如果这个参数为空，则查找
	if (pSockIoData == NULL)
	{
		pSockIoData = m_veSocketIOData.at(nPos);
	}
	vector<LPSOCKETIODATA>::iterator itData;
	for(itData = m_veSocketIOData.begin(); itData != m_veSocketIOData.end(); itData++)
	{
		if ((*itData) == pSockIoData)
		{
			if (pSockIoData != NULL)
			{
				delete pSockIoData;
				pSockIoData = NULL;
			}
			m_veSocketIOData.erase(itData);
			break;
		}
	}
	
	EnterCriticalSection(&m_cs);

	m_ulClientCount--;

	LeaveCriticalSection(&m_cs);

	
	//更新列表
	::SendMessage(m_hDlgWnd,WM_SHOW_CLIENT_INFO,(WPARAM)IOCP_MSG_TYPE_UPDATE,NULL);

	return TRUE;
}
BOOL IOCPClass::PostMsgToIOCP(LPCLIENTSOCKETINFO pClientSockInfo,DWORD ulClientIndex,WCHAR *pszText)
{
	char szText[MAX_BUFFER_LEN]={0};

	WideCharToChar(pszText,szText);
	DWORD dwLen = (DWORD)strlen(szText);

	LPSOCKETIODATA pData = NULL;
	pData = m_veSocketIOData.at(ulClientIndex);
	pData->opType = OP_SEND;
	memcpy(pData->buf,szText,dwLen);
	pData->wsaBuf.buf = pData->buf;
	pData->wsaBuf.len = dwLen;
	memset(&(pData->ol),0,sizeof(pData->ol));
	PostQueuedCompletionStatus(m_hCompPort, pData->wsaBuf.len, (ULONG_PTR)pClientSockInfo, &pData->ol);
	return TRUE;
}

BOOL IOCPClass::AddClientInfoToList(char *pszIP)
{
	CString strIndex,strComputerName;
	WCHAR szIP[MAX_BUFFER_LEN]={0};
	strIndex.Format(L"%d",m_ulClientCount);
	CharToWideChar(pszIP,szIP);
//	m_listClient.InsertItem(m_ulClientCount-1,strIndex);
//	m_listClient.SetItemText(m_ulClientCount-1,1,szIP);

	/*in_addr ina;
	ina.S_un.S_addr = inet_addr(pszIP); 
	hostent* phostent=gethostbyaddr((char*)&ina.S_un.S_addr,4,AF_INET);
	CharToWideChar(phostent->h_name,szIP);
	m_listClient.SetItemText(m_ulClientCount-1,2,szIP);*/

	::SendMessage(m_hDlgWnd,WM_SHOW_CLIENT_INFO,(WPARAM)IOCP_MSG_TYPE_ADD,(LPARAM)szIP);
	return TRUE;
}

BOOL IOCPClass::PostAccept(LPSOCKETIODATA pSockIoData)
{
	DWORD dwBytes=0;
	//投递一个OP_ACCEPT消息，是为了接收下一个client的连接，pSockIoData->sAccept为下一个client连接进来分配的socket号
	pSockIoData->sListen = m_sListenSocket;
	pSockIoData->opType = OP_ACCEPT;
	pSockIoData->sAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == pSockIoData->sAccept)
	{
		TRACE(L"WSASocket failed with error code: %d\n", WSAGetLastError());
		return FALSE;
	}

	//(PASCAL FAR * LPFN_ACCEPTEX)(
	//	IN SOCKET sListenSocket,
	//	IN SOCKET sAcceptSocket,
	//	IN PVOID lpOutputBuffer,
	//	IN DWORD dwReceiveDataLength,
	//	IN DWORD dwLocalAddressLength,
	//	IN DWORD dwRemoteAddressLength,
	//	OUT LPDWORD lpdwBytesReceived,
	//	IN LPOVERLAPPED lpOverlapped
	//	);
	if(FALSE == lpfnAcceptEx(pSockIoData->sListen, pSockIoData->sAccept, pSockIoData->wsaBuf.buf, pSockIoData->wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2), 
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, &(pSockIoData->ol)))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			TRACE(L"lpfnAcceptEx failed with error code: %d\n", WSAGetLastError());
			return FALSE;
		}
	}

	return TRUE;
}


UINT IOCPClass::Create(HWND hDlgWnd,TCHAR *lpszServerIP,UINT ulPort)
{
	if (m_bIsCreateIOCP)
	{
		return IOCP_NO_ERROR;
	}
	m_ulClientCount = 0;
    
	if ((lpszServerIP == NULL)||(ulPort == 0))
	{
		return IOCP_ERROR_IP_PORT;
	}
    
	m_hDlgWnd = hDlgWnd;

	m_hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long) 0, 0);
	if (m_hCompPort == NULL) 
	{
		//AfxMessageBox(_T("CreateIoCompletionPort failed with error: ")/*,GetLastError()*/ );
		return IOCP_ERROR_CREATE_IOCP;
	}

	InitializeCriticalSection(&m_cs);

	// Create worker thread
	SYSTEM_INFO si = {0};
	GetSystemInfo(&si);
	m_nThread = 0;
	m_bExitThread = FALSE;
	for(int i = 0; i < (int)si.dwNumberOfProcessors*2; i++)
	{
		m_hThread[m_nThread] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (LPVOID)this, 0, NULL);
		if(NULL == m_hThread[m_nThread])
		{
			AfxMessageBox(_T("_beginthreadex failed with error code: ")/*, GetLastError()*/);
			continue;
		}
		++m_nThread;

		if(m_nThread > MAX_THREAD_COUNT)
		{
			break;
		}
	}


	m_sListenSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_sListenSocket)
	{
		return IOCP_ERROR_SOCKET;
	}

	HANDLE h=CreateIoCompletionPort((HANDLE) m_sListenSocket, m_hCompPort, (u_long) 0, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		//AfxMessageBox(_T("CreateIoCompletionPort m_sListenSocket failed with error: ")/*,GetLastError()*/ );
		return IOCP_ERROR_ASSOSI_IOCP;
	}

	SOCKADDR_IN  addrSvc; //服务器地址信息 
	char szIP[128]={0};
	WideCharToMultiByte(CP_ACP,0,lpszServerIP,(int)_tcslen(lpszServerIP),szIP,sizeof(szIP),NULL,NULL);
	addrSvc.sin_addr.S_un.S_addr = inet_addr(szIP);
	//	addrSvc.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //0 接收所有数据包  
	addrSvc.sin_family   =  AF_INET;   
	addrSvc.sin_port  =  htons(ulPort);//监听 端口

	//绑定服务和端口 
	if (0 != bind(m_sListenSocket,(SOCKADDR*)&addrSvc,sizeof(SOCKADDR)))
	{
		//WSAGetLastError();
		//AfxMessageBox(_T("bind() server socket failed"));
		return IOCP_ERROR_BIND;
	}

	if (0 != listen(m_sListenSocket,100))
	{
		//AfxMessageBox(_T("listen() server socket failed"));
		return IOCP_ERROR_LISTEN;
	}  

	//客户的socket
	//SOCKET sClientSocket;
	//SOCKADDR_IN ClientSockAddr;//客户的addr
	//int iAddrLen=sizeof(SOCKADDR);
	//sClientSocket=WSAAccept(m_sListenSocket,(SOCKADDR *)&ClientSockAddr,&iAddrLen,NULL,NULL);


	DWORD dwBytes = 0;
	if(SOCKET_ERROR == WSAIoctl(m_sListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx,
		sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL))
	{
		TRACE(L"WSAIoctl failed with error code: %d\n", WSAGetLastError());
		if(INVALID_SOCKET != m_sListenSocket)
		{
			closesocket(m_sListenSocket);
			m_sListenSocket = INVALID_SOCKET;
		}
		return IOCP_ERROR_GET_FUNC;
	}

	if(SOCKET_ERROR == WSAIoctl(m_sListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, 
		sizeof(GuidGetAcceptExSockAddrs), &lpfnGetAcceptExSockAddrs, sizeof(lpfnGetAcceptExSockAddrs), 
		&dwBytes, NULL, NULL))
	{
		TRACE(L"WSAIoctl failed with error code: %d\n", WSAGetLastError());
		if(INVALID_SOCKET != m_sListenSocket)
		{
			closesocket(m_sListenSocket);
			m_sListenSocket = INVALID_SOCKET;
		}
		return IOCP_ERROR_GET_FUNC;
	}

	if (FALSE == PostAccept(&m_sIOData))
	{
		return IOCP_ERROR_ACCEPT;
	}

	m_hCheckSockerThread = (HANDLE)_beginthreadex(NULL, 0, CheckClientSocketIsAliveThread, (LPVOID)this, 0, NULL);

	m_bIsCreateIOCP = TRUE;

	return IOCP_NO_ERROR;
}

void IOCPClass::Release()
{
	if (!m_bIsCreateIOCP)
	{
		return;
	}
	m_bIsCreateIOCP = FALSE;
	m_bExitThread = TRUE;
	//	PostQueuedCompletionStatus(m_hCompPort, 0, NULL, NULL);
	WaitForMultipleObjects(m_nThread, m_hThread, TRUE, INFINITE);
	WaitForSingleObject(m_hCheckSockerThread,INFINITE);
	if (m_hCheckSockerThread)
	{
		CloseHandle(m_hCheckSockerThread);
		m_hCheckSockerThread = NULL;
	}
	for(int i = 0; i <(int) m_nThread; i++)
	{
		if (m_hThread[i])
		{
			CloseHandle(m_hThread[i]);
			m_hThread[i] = NULL;
		}
	}

	if(INVALID_SOCKET != m_sListenSocket)
	{
		closesocket(m_sListenSocket);
		m_sListenSocket = INVALID_SOCKET;
	}
	if (m_hCompPort)
	{
		CloseHandle(m_hCompPort);
		m_hCompPort = NULL;
	}
	LPCLIENTSOCKETINFO pClient = NULL;
	list<LPCLIENTSOCKETINFO>::iterator it;
	for (it = m_lsClientSocketList.begin() ;it != m_lsClientSocketList.end(); it++)
	{
		pClient = *it;
		if (pClient != NULL)
		{
			closesocket(pClient->sock);
			delete pClient;
			pClient = NULL;
		}
	}
	m_lsClientSocketList.clear();

	vector<LPSOCKETIODATA>::iterator itData;
	LPSOCKETIODATA pIOData = NULL;
	for (itData = m_veSocketIOData.begin() ;itData != m_veSocketIOData.end(); itData++)
	{
		pIOData = *itData;
		if (pIOData != NULL)
		{
			delete pIOData;
			pIOData = NULL;
		}
	}
	m_veSocketIOData.clear();

	DeleteCriticalSection(&m_cs);

	return;
}