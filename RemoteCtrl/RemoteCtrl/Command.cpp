#include "pch.h"
#include "Command.h"


CCommand::CCommand()
{
	struct CommandExecute{
		int nCmd;
		CMDFUNC func;
	};
	CommandExecute data[] = {
		{ 1,&CCommand::MakeDriverInfo },
		{ 2,&CCommand::MakeDirectoryInfo },
		{ 3,&CCommand::RunFile },
		{ 4,&CCommand::DownLoadFile },
		{ 5,&CCommand::MouseEvent },
		{ 6,&CCommand::SendScreen },
		{ 7,&CCommand::LockMachine },
		{ 8,&CCommand::UNLockMachine },
		{ 9,&CCommand::DeleteLocalFile },
		{ 1981,&CCommand::TestConnect },
		{ -1,NULL }

	};
	for (int i = 0; data[i].nCmd != -1; i++)
	{
		m_mapFuntion.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}
	
}

CCommand::~CCommand()
{
}

int CCommand::ExcuteCommand(int nCmd, std::list<CPacket>& lspacket, CPacket& packetIn)
{
	std::map<int, CMDFUNC>::iterator itor = m_mapFuntion.find(nCmd);
	if (itor == m_mapFuntion.end())
	{
		return -1; 
	}
	
	return (this->*itor->second)(lspacket, packetIn);
}

void CCommand::RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& packetIn)
{
    CCommand* thiz = (CCommand*)arg;
    if (status > 0)
    {
        int ret = thiz->ExcuteCommand(status, lstPacket, packetIn);
        if (ret < 0)
        {
            TRACE("执行命令失败：%d ret = %d \r\n", status, ret);
        }
    }
    else
    {
        MessageBox(NULL, _T("如法正常接入用户"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    
}

int CCommand::LockMachine(std::list<CPacket>& lspacket, CPacket& packetIn)
{
	//判断是否已经锁机了
	if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
	{
		//_beginthread(threadLockDlg, 0, NULL);
		_beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
	}
	
    lspacket.push_back(CPacket(7, NULL, 0));

	return 0;
}

int  CCommand::UNLockMachine(std::list<CPacket>& lspacket, CPacket& packetIn)
{
	//::SendMessage(dlg.m_hWnd,WM_KEYDOWN, 0x1b, 0x01E001);
	PostThreadMessage(threadid, WM_KEYDOWN, 0x1b, 0x01E001);
    lspacket.push_back(CPacket(8, NULL, 0));
	return 0;
}

int  CCommand::TestConnect(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    lspacket.push_back(CPacket(1981, NULL, 0));
	return 0;
}

int  CCommand::DeleteLocalFile(std::list<CPacket>& lspacket, CPacket& packetIn)
{
	std::string strpath = packetIn.strData;

	//进行一个字符集的转变
	//TCHAR sPath[MAX_PATH];
	//mbstowcs(sPath, strpath.c_str(), strpath.size() + 1);
	DeleteFileA(strpath.c_str());
    lspacket.push_back(CPacket(9, NULL, 0));

	return 0;
}




int CCommand::MakeDirectoryInfo(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    std::string strPath;//指定的路径
    std::list<FILEINFO> listFIleInfos;//路径下面的文件列表
    if (packetIn.sCmd != 2 &&
        packetIn.sCmd != 3 &&
        packetIn.sCmd != 4 &&
        packetIn.sCmd != 9)
    {
        OutputDebugStringA("当前命令不是获取文件列表的命令");
        return -1;
    }
    strPath = packetIn.strData;
    if (_chdir(strPath.c_str()) != 0)
    {
        FILEINFO finfo1;
        finfo1.HaveNext = FALSE;
        listFIleInfos.push_back(finfo1);
        lspacket.push_back(CPacket(2, (BYTE*)&finfo1, sizeof(finfo1)));
        OutputDebugStringA("没有权限访问目录");
        return -2;
    }

    _finddata_t fdata;
    intptr_t hfind;
    hfind = _findfirst("*.*", &fdata);
    if (hfind == -1)
    {
        OutputDebugStringA("没有找到任何文件");
        FILEINFO finfo2;
        finfo2.HaveNext = FALSE;
        lspacket.push_back(CPacket(2, (BYTE*)&finfo2, sizeof(finfo2)));
        return -3;
    }

    do
    {
        FILEINFO finfo3;
        finfo3.ISDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo3.szFileName, fdata.name, strlen(fdata.name));
        TRACE("%s\r\n", finfo3.szFileName);
        lspacket.push_back(CPacket(2, (BYTE*)&finfo3, sizeof(finfo3)));
    } while (!_findnext(hfind, &fdata));

    FILEINFO finfo;
    finfo.HaveNext = FALSE;
    lspacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
    return 0;
}

int CCommand::RunFile(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    std::string strPath = packetIn.strData;
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    lspacket.push_back(CPacket(3, NULL, 0));
    return 0;
}

int CCommand::DownLoadFile(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    std::string strPath = packetIn.strData;
    long long data = 0;
    FILE* pFile = NULL;
    //这里用fopen_s是安全打开方式，因为fopen有可能出现
    //返回了句柄但是你无法操作文件的现象
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err != 0)
    {
        lspacket.push_back(CPacket(4, (BYTE*)&data, 8));
        return -1;
    }
    if (pFile != NULL)
    {
        //先发给用户这个文件有多大
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        lspacket.push_back(CPacket(4, (BYTE*)&data, 8));
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024];
        size_t rlen = 0;
        do {
            //而后循环发送文件部分一次发送1024个字节
            rlen = fread(buffer, 1, 1024, pFile);
            lspacket.push_back(CPacket(4, (BYTE*)buffer, rlen));
        } while (rlen >= 1024);

    }
    else {
        lspacket.push_back(CPacket(4, NULL, 0));
    }
   
    fclose(pFile);
    return 0;
}

int CCommand::MouseEvent(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    MOUSEEV mouse;
    if (packetIn.sCmd == 5)
    {
        memcpy(&mouse, packetIn.strData.c_str(), sizeof(MOUSEEV));
        //先设置了鼠标的坐标信息
        SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        DWORD nFlags = 0;

        switch (mouse.nButton)
        {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中建
            nFlags = 4;
            break;
        case 3://没有按键
            nFlags = 8;
            break;
        default:
            break;
        }

        switch (mouse.nAction)
        {
        case 0://单击
            nFlags |= 0x10;
            break;
        case 1://双击
            nFlags |= 0x20;
            break;
        case 2://按下
            nFlags |= 0x40;
            break;
        case 3://放开
            nFlags |= 0x80;
            break;
        default:

            break;
        }
        TRACE("mouse event : %08X x %d y %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
        case 0x11://左键单击
            //GetMessageExtraInfo 获得当前系统中的键盘鼠标的额外信息
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x12://右键单击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x14://中建单击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://中建双击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中建放开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://鼠标移动
            //mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        default:
            break;
        }
        lspacket.push_back(CPacket(4, NULL, 0));


    }
    else {
        OutputDebugStringA("获取鼠标参数失败");
        return -1;
    }
    ;
    return 0;
}


int CCommand::SendScreen(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    CImage screen;//GDI
    //获取当前设备上下文
    HDC hScreen = ::GetDC(NULL);
    //位宽
    //255*255*255 这就是24比特的色彩表示 加上透明度就是32比特了
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);

    //创建了一个全局的内存块
    auto hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    IStream* PStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &PStream);
    if (ret == S_OK)
    {
        //将文件保存到内存区域,
        screen.Save(PStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        PStream->Seek(bg, STREAM_SEEK_SET, NULL);
        //获取内存指针并且上锁
        PBYTE pData = (PBYTE)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        lspacket.push_back(CPacket(6, pData, nSize));
        GlobalUnlock(hMem);


    }
    PStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}

int CCommand::MakeDriverInfo(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    std::string result = "";
    for (int i = 1; i <= 26; i++)
    {
        if (_chdrive(i) == 0)
        {
            if (result.size() > 0)
                result += ',';
            result += 'A' + i - 1;
        }
    }
    lspacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
    return 0;
}

unsigned  __stdcall CCommand::threadLockDlg(void* arg)
{
    //让用户知道你被锁机了
    //最顶层显示
    //留一个关闭的接口
    //模态（不结束后面不能激活独占）和非模态选哪个
    CCommand * pcommand = (CCommand*)arg;
    pcommand->dlg.Create(IDD_DIALOG_INFO, NULL);
    pcommand->dlg.ShowWindow(SW_SHOW);
    //让窗口铺满屏幕
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    rect.bottom = LONG(rect.bottom * 1.10);
    pcommand->dlg.MoveWindow(rect);
    CWnd* ptext = pcommand->dlg.GetDlgItem(IDC_STATIC);
    if (ptext)
    {
        CRect rtText;
        ptext->GetWindowRect(rtText);
        int width = rtText.Width();
        int height = rtText.Height();
        int x = (rect.right - width) / 2;
        int y = (rect.bottom - height) / 2;
        ptext->MoveWindow(x, y, rtText.Width(), rtText.Height());

    }
    //窗口置顶
    pcommand->dlg.SetWindowPos(&pcommand->dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //限制鼠标的功能
    ShowCursor(false);

    //限制鼠标活动范围
    pcommand->dlg.GetWindowRect(&rect);
    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    ClipCursor(rect);
    //隐藏任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    //这里的GetMessage，消息甭。20ms一次。是和线程绑定的，只能拿到这个线程内的消息
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        //按下esc键退出
        if (msg.wParam == 0x1b)
        {

            break;
        }
    }
    //恢复鼠标操作
    ClipCursor(NULL);
    ShowCursor(true);
    //恢复任务栏操作
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    pcommand->dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}