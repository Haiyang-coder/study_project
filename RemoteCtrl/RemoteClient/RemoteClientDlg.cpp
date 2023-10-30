
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include"ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_serv_ip(0)
	, m_serv_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, edit_ip, m_serv_ip);
	DDX_Text(pDX, edit_port, m_serv_port);
	DDX_Control(pDX, treedir, m_tree);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t length)
{
	CClientSocket* pClent = CClientSocket::getInstance();
	bool ret = pClent->InitSocket(m_serv_ip, atoi((LPCTSTR)m_serv_port));//todo 返回值处理
	if (ret == false)
	{
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, length);
	pClent->Send(pack);
	int iRet = pClent->DealCommand();
	if (iRet == -1)
	{
		AfxMessageBox("处理命令失败");
		TRACE("sCmd : %d\r\n", pClent->Getpack().sCmd);
		pClent->closeSocket();
		return -1;
	}
	TRACE("sCmd : %d\r\n", pClent->Getpack().sCmd);
	if (bAutoClose)
	{
		pClent->closeSocket();
	}
	
	return nCmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(button_test, &CRemoteClientDlg::OnBnClickedtest)
	
	ON_BN_CLICKED(button_file, &CRemoteClientDlg::OnBnClickedfile)
	ON_NOTIFY(NM_DBLCLK, treedir, &CRemoteClientDlg::OnNMDblclktreedir)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	UpdateData();
	m_serv_ip = 0x7f000001;
	m_serv_port = _T("9527");
	UpdateData(false);
	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedtest()
{
	//true将控件的值赋值给成员变量
	UpdateData();
	SendCommandPacket(1981);
}


void CRemoteClientDlg::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}




void CRemoteClientDlg::OnBnClickedfile()
{
	//取到根目录的文件信息
	int ret = SendCommandPacket(1);
	if(ret == -1)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
	CClientSocket* pClent = CClientSocket::getInstance();
	auto drivers = pClent->Getpack().strData;
	std::string dr;
	m_tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ':';
			auto hitem = m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_tree.InsertItem(NULL, hitem, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
}

//获取双击项目的完整路径
CString CRemoteClientDlg::GetPath(HTREEITEM htree)
{
	CString strRet, strTemp;
	do
	{
		strTemp = m_tree.GetItemText(htree);
		strRet += strTemp + "\\" + strRet;
		htree = m_tree.GetParentItem(htree);
	} while (htree != nullptr);
	return strRet;
}

//删除指定节点的所有子节点
void CRemoteClientDlg::DeleteTreeChileItem(HTREEITEM htree)
{
	HTREEITEM hSub = nullptr;
	do
	{
		hSub = m_tree.GetChildItem(htree);
		if (hSub != nullptr)
		{
			m_tree.DeleteItem(hSub);
		}
	} while (hSub != nullptr);
}


void CRemoteClientDlg::OnNMDblclktreedir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	
	//拿到的是全局坐标
	CPoint pMouse;
	GetCursorPos(&pMouse);
	//转成局部控件坐标
	m_tree.ScreenToClient(&pMouse);
	//获取双击的item
	HTREEITEM hTreeSelected = m_tree.HitTest(pMouse, 0);
	if (hTreeSelected == NULL)
	{
		return;
	}
	if (m_tree.GetChildItem(hTreeSelected) == NULL)
	{
		return;
	}
	DeleteTreeChileItem(hTreeSelected);
	//根据双击的结果获取完整的路径信息
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2,false, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->Getpack().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	//看一下双击的文件是否还有下一个
	//有的话进行马上进行处理
	while (pInfo->HaveNext == TRUE)
	{
		//TRACE("[%s] , d%\r\n", pInfo->szFileName, pInfo->ISDirectory);
		if (pInfo->ISDirectory)
		{
			if (CString(pInfo->szFileName) == "." ||
				CString(pInfo->szFileName) == "..")
			{
				int cmd = pClient->DealCommand();
				if (cmd < 0)
				{
					break;
				}
				pInfo = (PFILEINFO)CClientSocket::getInstance()->Getpack().strData.c_str();
				continue;
			}
			
		}
		HTREEITEM htreeTemp =  m_tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
		if (pInfo->ISDirectory)
		{
			m_tree.InsertItem(NULL, htreeTemp, TVI_LAST);
		}
		int cmd = pClient->DealCommand();
		if (cmd < 0)
		{
			break;
		}
		pInfo = (PFILEINFO)CClientSocket::getInstance()->Getpack().strData.c_str();
		TRACE("ack:%d \r\n", cmd);
	} 

	
	pClient->closeSocket();
}
