﻿#pragma once
#include "afxdialogex.h"
#include<string>


// CStatusDlg 对话框

class CStatusDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusDlg)

public:
	CStatusDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CStatusDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = dlg_status };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_info;
};
