// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include"RemoteClientDlg.h"
#include"ClientController.h"


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
	ON_MESSAGE(WM_SEND_PACKET_ACK, &CWatchDialog::OnSendPacketACK)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreen(CPoint& point ,bool isScreen)
{
	//进行鼠标坐标的转换
	CRect clientRect;
	if (!isScreen)
	{
		ClientToScreen(&point);//转换为相对屏幕左上角的坐标,屏幕内的绝对坐标
	}
	m_picture.ScreenToClient(&point);
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

	//SetTimer(0, 50, NULL);

	return TRUE;  
}



//timer计时器定时刷新显示区域，这里有线程冲突问题
void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (nIDEvent == 0)
	//{
	//	CClientController* pParent = (CClientController*)GetParent();
	//	if (GetIsFull())
	//	{
	//		CRect rect;
	//		m_picture.GetWindowRect(rect);
	//		m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect
	//			.Width(), rect.Height(), SRCCOPY);
	//		//将缓存的图像显示在页面
	//		if (m_iObjWidth == -1)
	//		{
	//			m_iObjWidth = m_image.GetWidth();
	//			m_iObhHeight = m_image.GetHeight();
	//		}
	//		//要对画面进行适配
	//		m_picture.InvalidateRect(NULL);
	//		m_image.Destroy();
	//		SetImageStatus(false);
	//		//TRACE("更新图片完成: kuan: d% , 高 : d%\r\n",m_iObjWidth, m_iObhHeight);
	//	}
	//}
	//CDialog::OnTimer(nIDEvent);
}
void CWatchDialog::SetImageStatus(bool isFull)
{
	m_isFull = isFull;
}
bool CWatchDialog::GetIsFull() const
{
	return m_isFull;
}

CImage& CWatchDialog::getImage()
{
	return m_image;
}

LRESULT CWatchDialog::OnSendPacketACK(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || lParam == -2)
	{
		//错误处理
	}
	else if(lParam == 1)
	{
		//对方关闭了套接字
	}
	else
	{
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != NULL)
		{
			switch (pPacket->sCmd)
			{
			case 5:
			{
				CClientController* pParent = (CClientController*)GetParent();
			}
				break;
			case 6:
			{
				CRemteClientTool::Byte2Image(m_image, pPacket->strData);
				CRect rect;
				m_picture.GetWindowRect(rect);
				m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect
					.Width(), rect.Height(), SRCCOPY);
				//将缓存的图像显示在页面
				m_picture.InvalidateRect(NULL);
				m_image.Destroy();
				SetImageStatus(false);
			}
				
				break;
			case 7:
				break;
			case 8:
				break;
			default:
				break;
			}
		}
		else 
		{

		}
	}
	return 0;
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
	CClientController* pclient = CClientController::getInstance();
	CPacket packet(5, (BYTE*) & event, sizeof(event));
	pclient->SendCommandPacket(GetSafeHwnd(), 5);
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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5,true, (BYTE*) &event, sizeof(event));
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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	

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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

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
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
}


void CWatchDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CClientController::getInstance()->SetScreenClose(true);
	CDialog::OnClose();
}


void CWatchDialog::OnBnClickedButton1()
{
	// 这是锁机操作

	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedButton2()
{
	// 这是解锁操作
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}
