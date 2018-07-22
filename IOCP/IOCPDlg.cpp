// IOCPDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IOCP.h"
#include "IOCPDlg.h"
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CIOCPDlg 对话框




CIOCPDlg::CIOCPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIOCPDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CIOCPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENT, m_listClient);
}

BEGIN_MESSAGE_MAP(CIOCPDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CIOCPDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CIOCPDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_SEND_MSG, &CIOCPDlg::OnBnClickedButtonSendMsg)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CIOCPDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CIOCPDlg::OnBnClickedButtonExit)
	ON_MESSAGE(WM_SHOW_CLIENT_INFO,OnShowClient)
END_MESSAGE_MAP()


// CIOCPDlg 消息处理程序

BOOL CIOCPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

//	ShowWindow(SW_MINIMIZE);

	
	char* szIP = m_iocp.GetLocalIPAddress();
	m_strServerIP= szIP;
	GetDlgItem(IDC_EDIT_IP)->SetWindowText(m_strServerIP);
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("4321"));
	GetDlgItem(IDC_EDIT_CLIENT_INDEX)->SetWindowText(_T("1"));

	m_listClient.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT); //设置list风格

	m_listClient.InsertColumn(0,_T("序号"),LVCFMT_LEFT, 40);
	m_listClient.InsertColumn(1,_T("IP地址"),LVCFMT_LEFT, 100);
	m_listClient.InsertColumn(2,_T("计算机名"),LVCFMT_LEFT,180);

	GetDlgItem(IDC_BUTTON_EXIT)->EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CIOCPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIOCPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CIOCPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CIOCPDlg::OnBnClickedOk()
{
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(m_strPort);
	if (m_strPort.IsEmpty()||m_strServerIP.IsEmpty())
	{
		AfxMessageBox(_T("ip or port is empty"));
		return ;
	}
	if (IOCP_NO_ERROR != m_iocp.Create(m_hWnd,m_strServerIP.GetBuffer(0),_wtoi(m_strPort.GetBuffer(0))))
	{
		AfxMessageBox(_T("IOCP create failed"));
		return ;
	}
	m_strServerIP.ReleaseBuffer();
	m_strPort.ReleaseBuffer();
    GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_EXIT)->EnableWindow(TRUE);
	////	OnOK();
}


void CIOCPDlg::OnBnClickedCancel()
{
	m_iocp.Release();

	OnCancel();
}

void CIOCPDlg::OnBnClickedButtonSendMsg()
{
	CString strClientIndex;
	GetDlgItem(IDC_EDIT_CLIENT_INDEX)->GetWindowText(strClientIndex);
	int iClientIndex = _wtoi(strClientIndex.GetBuffer(0));
	strClientIndex.ReleaseBuffer();
	if ((m_iocp.m_ulClientCount <=0) || (iClientIndex <=0)|| (iClientIndex > (int)m_iocp.m_ulClientCount))
	{
		AfxMessageBox(L"Not find the client,please confirm the client does it exist");
		return;
	}
	list<LPCLIENTSOCKETINFO>::iterator it;
	it = m_iocp.m_lsClientSocketList.begin();
	//找到相应的客户端
	DWORD dwClient=0;
	for(int i = 0; i< (iClientIndex-1); i++)
	{
		dwClient++;
		it++;
	}

	LPCLIENTSOCKETINFO pSockInfo;
    pSockInfo = *it;

	m_iocp.PostMsgToIOCP(pSockInfo,dwClient,L"sendmsg");

}

void CIOCPDlg::OnBnClickedButtonClose()
{
	CString strClientIndex;
	GetDlgItem(IDC_EDIT_CLIENT_INDEX)->GetWindowText(strClientIndex);
	int iClientIndex = _wtoi(strClientIndex.GetBuffer(0));
	strClientIndex.ReleaseBuffer();
	if ((m_iocp.m_ulClientCount <=0) || (iClientIndex <=0)|| (iClientIndex > (int)m_iocp.m_ulClientCount))
	{
		AfxMessageBox(L"Not find the client,please confirm the client does it exist");
		return;
	}

	list<LPCLIENTSOCKETINFO>::iterator it;
	it = m_iocp.m_lsClientSocketList.begin();
	//找到相应的客户端
	DWORD dwClient=0;
	for(int i = 0; i< (iClientIndex-1); i++)
	{
		dwClient++;
		it++;
	}

	LPCLIENTSOCKETINFO pSockInfo;
	pSockInfo = *it;


	if (FALSE == m_iocp.DeleteOneClient(pSockInfo))
	{
		AfxMessageBox(L"not find the client socket");
		return ;
	}
}

void CIOCPDlg::OnBnClickedButtonExit()
{
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_EXIT)->EnableWindow(FALSE);
    m_listClient.DeleteAllItems();

	m_iocp.Release();
}


BOOL CIOCPDlg::UpdateList()
{
	char *pszIP = NULL;
	CString strIndex,strComputerName;
	WCHAR szIP[MAX_BUFFER_LEN]={0};
//	in_addr ina;
	LPCLIENTSOCKETINFO pClient = NULL;
	list<LPCLIENTSOCKETINFO>::iterator it;
	m_listClient.DeleteAllItems();
	int i = 0;
	for (it = m_iocp.m_lsClientSocketList.begin() ;it != m_iocp.m_lsClientSocketList.end(); it++)
	{
		pClient = *it;
		pszIP = inet_ntoa(pClient->addr.sin_addr);
		strIndex.Format(L"%d",i+1);
		m_iocp.CharToWideChar(pszIP,szIP);
		m_listClient.InsertItem(i,strIndex);
		m_listClient.SetItemText(i,1,szIP);

		/*ina.S_un.S_addr = inet_addr(pszIP); 
		hostent* phostent=gethostbyaddr((char*)&ina.S_un.S_addr,4,AF_INET);
		m_iocp.CharToWideChar(phostent->h_name,szIP);
		m_listClient.SetItemText(i,2,szIP);*/
		i++;
	}
	
	return TRUE;
}

LRESULT CIOCPDlg::OnShowClient(WPARAM wParam,LPARAM lParam)
{
	DWORD dwMsgType = (DWORD)wParam;
	TCHAR *pText = (TCHAR*)lParam;
	switch(wParam)
	{
	case IOCP_MSG_TYPE_ADD:
		{
			AddClientInfoToList(pText);
		}
		break;
	case IOCP_MSG_TYPE_UPDATE:
		{
			UpdateList();
		}
		break;
	default:
		;
	}
	return 0;
}

BOOL CIOCPDlg::AddClientInfoToList(WCHAR *pszIP)
{
	CString strIndex;
	strIndex.Format(L"%d",m_iocp.m_ulClientCount);
	m_listClient.InsertItem(m_iocp.m_ulClientCount-1,strIndex);
	m_listClient.SetItemText(m_iocp.m_ulClientCount-1,1,pszIP);

	//CString strIndex,strComputerName;
	//WCHAR szIP[MAX_BUFFER_LEN]={0};
	//strIndex.Format(L"%d",m_ulClientCount);
	//CharToWideChar(pszIP,szIP);
	//m_listClient.InsertItem(m_ulClientCount-1,strIndex);
	//m_listClient.SetItemText(m_ulClientCount-1,1,szIP);

	/*in_addr ina;
	ina.S_un.S_addr = inet_addr(pszIP); 
	hostent* phostent=gethostbyaddr((char*)&ina.S_un.S_addr,4,AF_INET);
	CharToWideChar(phostent->h_name,szIP);
	m_listClient.SetItemText(m_ulClientCount-1,2,szIP);*/
	return TRUE;
}