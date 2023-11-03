// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include"RemoteClientDlg.h"


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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


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
			//要对画面进行适配

			m_picture.InvalidateRect(NULL);
			pParent->getImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}
