
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClientDlg.h"
#include"ClientController.h"


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
public:

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
	DDX_Control(pDX, listFile, m_list);
}


void CRemoteClientDlg::LoadFileCurrent()
{
	auto htree = m_tree.GetSelectedItem();
	auto  strPath = GetPath(htree);

	m_list.DeleteAllItems();
	int nCmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),2, false, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	file_info* pInfo = (file_info*)CClientSocket::getInstance()->Getpack().strData.c_str();
	CClientController* pClient = CClientController::getInstance();
	//看一下双击的文件是否还有下一个
	//有的话进行马上进行处理
	while (pInfo->HaveNext == TRUE)
	{
		if (!pInfo->ISDirectory)
		{
			m_list.InsertItem(0, pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		if (cmd < 0)
		{
			break;
		}
		pInfo = (file_info*)CClientSocket::getInstance()->Getpack().strData.c_str();
		TRACE("ack:%d \r\n", cmd);
	}
	 


	pClient->closeSocket();
}



BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(button_test, &CRemoteClientDlg::OnBnClickedtest)
	
	ON_BN_CLICKED(button_file, &CRemoteClientDlg::OnBnClickedfile)
	ON_NOTIFY(NM_DBLCLK, treedir, &CRemoteClientDlg::OnNMDblclktreedir)
	ON_NOTIFY(NM_CLICK, treedir, &CRemoteClientDlg::OnNMClicktreedir)
	ON_NOTIFY(NM_RCLICK, listFile, &CRemoteClientDlg::OnNMRClicklistfile)
	ON_COMMAND(id_downloadfile, &CRemoteClientDlg::Ondownloadfile)
	ON_COMMAND(id_deletefile, &CRemoteClientDlg::Ondeletefile)
	ON_COMMAND(id_openfile, &CRemoteClientDlg::Onopenfile)
	ON_BN_CLICKED(button_startwatch, &CRemoteClientDlg::OnBnClickedstartwatch)
	ON_WM_TIMER()
	ON_EN_CHANGE(edit_port, &CRemoteClientDlg::OnEnChangeport)
	ON_MESSAGE(WM_SEND_PACKET_ACK, &CRemoteClientDlg::OnSendPacketACK)
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
	m_serv_ip = 0x7F000001;//0xAFB28315 //0x7F000001
	m_serv_port = _T("9527");
	CClientController* pClent = CClientController::getInstance();
	pClent->UpdateAddress(m_serv_ip, atoi((LPCTSTR)m_serv_port));
	UpdateData(false);
	m_dlgStatus.Create(dlg_status,this);
	m_dlgStatus.ShowWindow(SW_HIDE);
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
	auto ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1981);
	TRACE("test : result: %d \r\n");
}


LRESULT CRemoteClientDlg::OnSendPacketACK(WPARAM wParam, LPARAM lParam)
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
		
		if (wParam != NULL)
		{
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd)
			{
			case 1://获取驱动信息
				Str2tree(head.strData, m_tree);
				break;
			case 2://获取文件信息
				UpdateFileInfo(*(file_info*)head.strData.c_str(), HTREEITEM(lParam));
				break;
			case 3:
				TRACE("run file down");
				break;
			case 4:
				UpdateDownLoadFile(head.strData, (FILE*)lParam);
				break;
			case 9:
				TRACE("down");
				break;
			case 1981:
				TRACE("test cmd success");
				break;
			default:
				TRACE("unkown cmd");
				break;
			}
			
		}
		else
		{

		}
	}
	
	return LRESULT();
}

void CRemoteClientDlg::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}




void CRemoteClientDlg::OnBnClickedfile()
{
	std::list<CPacket> lstpacket;
	//取到根目录的文件信息
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1, true, NULL, 0);
	if(ret == 0)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
}

//获取双击项目的完整路径
CString CRemoteClientDlg::GetPath(HTREEITEM htree)
{
	CString strRet, strTemp;
	do
	{
		strTemp = m_tree.GetItemText(htree);
		strRet = strTemp + "\\" + strRet;
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

void CRemoteClientDlg::LoadFileInfo()
{
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

	DeleteTreeChileItem(hTreeSelected);
	m_list.DeleteAllItems();
	//根据双击的结果获取完整的路径信息
	CString strPath = GetPath(hTreeSelected);
	int nCmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCSTR)strPath, strPath.GetLength(),(WPARAM) hTreeSelected);
}



void CRemoteClientDlg::OnNMDblclktreedir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
	
}


void CRemoteClientDlg::OnNMClicktreedir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClicklistfile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	//获取鼠标点击位置
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	GetCursorPos(&ptList);
	m_list.ScreenToClient(&ptList);
	int listSelected =  m_list.HitTest(ptList);
	if (listSelected < 0)
	{
		return;
	}
	//加载菜单资源
	CMenu menu;
	menu.LoadMenu(menu_right_click);
	CMenu* pPupup =menu.GetSubMenu(0);
	if (pPupup != nullptr)
	{
		pPupup->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN,ptMouse.x,ptMouse.y,this);
	}
}


void CRemoteClientDlg::Ondownloadfile()
{
	//获取文件名
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	//获取文件的绝对路径
	HTREEITEM hseleted = m_tree.GetSelectedItem();
	strFile = GetPath(hseleted) + strFile;
	TRACE("[%s]\r\n", LPCSTR(strFile));
	//调用controller
	int ret = CClientController::getInstance()->DownLoadFile(strFile);
	if (ret != 0)
	{
		MessageBox(_T("下载失败"));
		TRACE("下载失败:%d \r\n", ret);
	}
	
	
	
}


void CRemoteClientDlg::Ondeletefile()
{
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int seleced = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(seleced, 0);
	strFile = strPath + strFile;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("删除文件失败，请重试");
		TRACE("文件传输失败：%d\r\n", ret);
		return;
	}
	LoadFileCurrent();
}


void CRemoteClientDlg::Onopenfile()
{
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int seleced = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(seleced, 0);
	strFile = strPath + strFile;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("打开文件失败，请重试");
		TRACE("文件传输失败：%d\r\n", ret);
		return;
	}
}

void CRemoteClientDlg::Str2tree(const std::string& drivers, CTreeCtrl& tree)
{
	//auto drivers = head.strData;
	std::string dr;
	tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ':';
			auto hitem = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			tree.InsertItem(NULL, hitem, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
	if (drivers.size() > 0)
	{
		dr += ':';
		auto hitem = tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
		tree.InsertItem(NULL, hitem, TVI_LAST);
		dr.clear();
	}
}

void CRemoteClientDlg::UpdateFileInfo(const file_info finfo, HTREEITEM lParam)
{
	if (finfo.HaveNext == FALSE)
	{
		return;
	}

	if (finfo.ISDirectory)
	{
		if (CString(finfo.szFileName) == "." ||
			CString(finfo.szFileName) == "..")
		{
			return;
		}
		HTREEITEM htreeTemp = m_tree.InsertItem(finfo.szFileName, lParam, TVI_LAST);
		m_tree.InsertItem(NULL, htreeTemp, TVI_LAST);
		m_tree.Expand((HTREEITEM)lParam, TVE_EXPAND);
	}
	else {//如果是文件就插入到list里面去
		m_list.InsertItem(0, finfo.szFileName);
	}
}

void CRemoteClientDlg::UpdateDownLoadFile(const std::string& strData, FILE* pFile)
{
	static LONGLONG length = 0, index = 0;
	if (length == 0)
	{
		length = *(long long*)strData.c_str();
		if (length == 0)
		{
			AfxMessageBox("文件长度为0，或者无法读取文件");
			CClientController::getInstance()->DownLoadEnd();
			return;
		}
	}
	else if (length > 0 && index >= length)
	{
		fclose(pFile);
		length = 0;
		index = 0;
		CClientController::getInstance()->DownLoadEnd();

	}
	else
	{
		while (index < length)
		{
			//开始接收服务端发过来的文件信息
			fwrite(strData.c_str(), 1, strData.size(), pFile);
			index += strData.size();

		}
		fclose(pFile);
		CClientController::getInstance()->DownLoadEnd();
		CClientController::getInstance()->closeSocket();
	}
}



void CRemoteClientDlg::OnBnClickedButton1()
{
}

void CRemoteClientDlg::OnBnClickedstartwatch()
{
	CClientController::getInstance()->StartWatchScreen();
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnIpnFieldchangedip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CClientController* pClent = CClientController::getInstance();
	pClent->UpdateAddress(m_serv_ip, atoi((LPCTSTR)m_serv_port));
	*pResult = 0;
}


void CRemoteClientDlg::OnEnChangeport()
{
	UpdateData();
	CClientController* pClent = CClientController::getInstance();
	pClent->UpdateAddress(m_serv_ip, atoi((LPCTSTR)m_serv_port));
}
