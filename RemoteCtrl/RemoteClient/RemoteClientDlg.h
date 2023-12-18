﻿
// RemoteClientDlg.h: 头文件
//

#pragma once
#include"StatusDlg.h"
#ifndef WM_SEND_PACKET_ACK
#define WM_SEND_PACKET_ACK (WM_USER + 2)	//发送包数据应答
#endif // !WM_SEND_PACKET_ACK
#ifndef WM_SEND_PACKET
#define WM_SEND_PACKET (WM_USER + 1) //发送数据包的消息
#endif // !WM_SEND_PACKET



// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



private:
	
	void LoadFileCurrent();
	CString GetPath(HTREEITEM htree);
	void DeleteTreeChileItem(HTREEITEM htree);
	void LoadFileInfo();
// 实现
protected:
	
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;
	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	bool m_isCLosed = true;//这是远程监控显示界面

	

	afx_msg void OnBnClickedtest();
	DWORD m_serv_ip;
	CString m_serv_port;
	afx_msg LRESULT OnSendPacketACK(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedfile();
	CTreeCtrl m_tree;
	afx_msg void OnNMDblclktreedir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClicktreedir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_list;
	afx_msg void OnNMRClicklistfile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void Ondownloadfile();
	afx_msg void Ondeletefile();
	afx_msg void Onopenfile();
private:
	void Str2tree(const std::string& driver, CTreeCtrl& tree);
	void UpdateFileInfo(const struct file_info finfo, HTREEITEM lParam);
	void UpdateDownLoadFile(const std::string& strData, FILE* pFile);

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedstartwatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnIpnFieldchangedip(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeport();
};
