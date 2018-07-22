// IOCPDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CIOCPDlg �Ի���




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


// CIOCPDlg ��Ϣ�������

BOOL CIOCPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

//	ShowWindow(SW_MINIMIZE);

	
	char* szIP = m_iocp.GetLocalIPAddress();
	m_strServerIP= szIP;
	GetDlgItem(IDC_EDIT_IP)->SetWindowText(m_strServerIP);
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(_T("4321"));
	GetDlgItem(IDC_EDIT_CLIENT_INDEX)->SetWindowText(_T("1"));

	m_listClient.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT); //����list���

	m_listClient.InsertColumn(0,_T("���"),LVCFMT_LEFT, 40);
	m_listClient.InsertColumn(1,_T("IP��ַ"),LVCFMT_LEFT, 100);
	m_listClient.InsertColumn(2,_T("�������"),LVCFMT_LEFT,180);

	GetDlgItem(IDC_BUTTON_EXIT)->EnableWindow(FALSE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIOCPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
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
	//�ҵ���Ӧ�Ŀͻ���
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
	//�ҵ���Ӧ�Ŀͻ���
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