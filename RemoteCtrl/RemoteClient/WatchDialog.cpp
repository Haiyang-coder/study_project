﻿// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include"RemoteClientDlg.h"
#include"ClientSocket.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(dlg_watch, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, idc_watch, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(idc_watch, &CWatchDialog::OnStnClickedwatch)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CWatchDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CWatchDialog::OnBnClickedButton2)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreen(CPoint& point ,bool isScreen)
{
	//进行鼠标坐标的转换
	CRect clientRect;
	if (isScreen)
	{
		ScreenToClient(&point);
	}
	m_picture.GetWindowRect(clientRect);
	int width =  clientRect.Width();
	int height = clientRect.Height();
	//要动态获取目标桌面的大小
	int widthRemote = m_iObjWidth;
	int heighRemote = m_iObhHeight;
	int x = point.x * widthRemote / width;
	int y = point.y * heighRemote / height;
	return CPoint(x, y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(0, 50, NULL);

	return TRUE;  
}



//timer计时器定时刷新显示区域，这里有线程冲突问题
void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->GetIsFull())
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			//将缓存的图像显示在页面
			//pParent->getImage().BitBlt(GetDC()->GetSafeHdc(), 0, 0,  SRCCOPY);
			pParent->getImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect
				.Width(), rect.Height(), SRCCOPY);
			if (m_iObjWidth == -1)
			{
				m_iObjWidth = pParent->getImage().GetWidth();
				m_iObhHeight = pParent->getImage().GetHeight();
			}
			//要对画面进行适配
			m_picture.InvalidateRect(NULL);
			pParent->getImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return ;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//双击
	CClientSocket* pclient = CClientSocket::getInstance();
	CPacket packet(5, (BYTE*) & event, sizeof(event));
	pclient->Send(packet);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//按下
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//弹起
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);

	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//右键
	event.nAction = 1;//双击
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);

	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//右键
	event.nAction = 2;//按下
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);

	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//左键
	event.nAction = 3;//弹起,服务端要修改
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);

	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(point);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 8;//没有按键
	event.nAction = 0;//移动
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);;

	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedwatch()
{
	if (m_iObhHeight == -1 || m_iObjWidth == -1)
	{
		return;
	}
	CPoint pt;
	GetCursorPos(&pt);
	//坐标转换
	CPoint remote = UserPoint2RemoteScreen(pt, true);
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 0;//单机
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
}


void CWatchDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->m_isCLosed = false;
	CDialog::OnClose();
}


void CWatchDialog::OnBnClickedButton1()
{
	// 这是锁机操作

	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDialog::OnBnClickedButton2()
{
	// 这是解锁操作
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
