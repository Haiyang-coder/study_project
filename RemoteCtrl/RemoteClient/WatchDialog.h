#pragma once
#include "afxdialogex.h"


// CWatchDialog 对话框
#ifndef WM_SEND_PACKET_ACK
#define WM_SEND_PACKET_ACK (WM_USER + 2)	//发送包数据应答
#endif // !WM_SEND_PACKET_ACK


class CWatchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDialog();


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = dlg_watch };
#endif
public:
	int m_iObjWidth = -1;
	int m_iObhHeight = -1;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


	DECLARE_MESSAGE_MAP()
public:
	void SetImageStatus(bool isFull = false);
	bool GetIsFull() const;
	CImage& getImage();
	CPoint UserPoint2RemoteScreen(CPoint& point, bool isScreen = false);
public:
	
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	CImage m_image;//缓存
	bool m_isFull = false;//缓存是否为慢的
	afx_msg LRESULT OnSendPacketACK(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedwatch();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
