// IOCPDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "IOCPClass.h"

// CIOCPDlg �Ի���
class CIOCPDlg : public CDialog
{
// ����
public:
	CIOCPDlg(CWnd* pParent = NULL);	// ��׼���캯��

	IOCPClass m_iocp;
	CString m_strServerIP;
	CString m_strPort;

	BOOL AddClientInfoToList(WCHAR *pszIP);
	BOOL UpdateList();
	LRESULT OnShowClient(WPARAM wParam,LPARAM lParam);

// �Ի�������
	enum { IDD = IDD_IOCP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnBnClickedButtonSendMsg();
public:
	CListCtrl m_listClient;
public:
	afx_msg void OnBnClickedButtonClose();
public:
	afx_msg void OnBnClickedButtonExit();
};
