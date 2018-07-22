// IOCPDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "IOCPClass.h"

// CIOCPDlg 对话框
class CIOCPDlg : public CDialog
{
// 构造
public:
	CIOCPDlg(CWnd* pParent = NULL);	// 标准构造函数

	IOCPClass m_iocp;
	CString m_strServerIP;
	CString m_strPort;

	BOOL AddClientInfoToList(WCHAR *pszIP);
	BOOL UpdateList();
	LRESULT OnShowClient(WPARAM wParam,LPARAM lParam);

// 对话框数据
	enum { IDD = IDD_IOCP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
