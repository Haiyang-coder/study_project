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
            TRACE("ִ������ʧ�ܣ�%d ret = %d \r\n", status, ret);
        }
    }
    else
    {
        MessageBox(NULL, _T("�編���������û�"), _T("�����û�ʧ��"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    
}

int CCommand::LockMachine(std::list<CPacket>& lspacket, CPacket& packetIn)
{
	//�ж��Ƿ��Ѿ�������
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

	//����һ���ַ�����ת��
	//TCHAR sPath[MAX_PATH];
	//mbstowcs(sPath, strpath.c_str(), strpath.size() + 1);
	DeleteFileA(strpath.c_str());
    lspacket.push_back(CPacket(9, NULL, 0));

	return 0;
}




int CCommand::MakeDirectoryInfo(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    std::string strPath;//ָ����·��
    std::list<FILEINFO> listFIleInfos;//·��������ļ��б�
    if (packetIn.sCmd != 2 &&
        packetIn.sCmd != 3 &&
        packetIn.sCmd != 4 &&
        packetIn.sCmd != 9)
    {
        OutputDebugStringA("��ǰ����ǻ�ȡ�ļ��б������");
        return -1;
    }
    strPath = packetIn.strData;
    if (_chdir(strPath.c_str()) != 0)
    {
        FILEINFO finfo1;
        finfo1.HaveNext = FALSE;
        listFIleInfos.push_back(finfo1);
        lspacket.push_back(CPacket(2, (BYTE*)&finfo1, sizeof(finfo1)));
        OutputDebugStringA("û��Ȩ�޷���Ŀ¼");
        return -2;
    }

    _finddata_t fdata;
    intptr_t hfind;
    hfind = _findfirst("*.*", &fdata);
    if (hfind == -1)
    {
        OutputDebugStringA("û���ҵ��κ��ļ�");
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
    //������fopen_s�ǰ�ȫ�򿪷�ʽ����Ϊfopen�п��ܳ���
    //�����˾���������޷������ļ�������
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err != 0)
    {
        lspacket.push_back(CPacket(4, (BYTE*)&data, 8));
        return -1;
    }
    if (pFile != NULL)
    {
        //�ȷ����û�����ļ��ж��
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        lspacket.push_back(CPacket(4, (BYTE*)&data, 8));
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024];
        size_t rlen = 0;
        do {
            //����ѭ�������ļ�����һ�η���1024���ֽ�
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
        //������������������Ϣ
        SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        DWORD nFlags = 0;

        switch (mouse.nButton)
        {
        case 0://���
            nFlags = 1;
            break;
        case 1://�Ҽ�
            nFlags = 2;
            break;
        case 2://�н�
            nFlags = 4;
            break;
        case 3://û�а���
            nFlags = 8;
            break;
        default:
            break;
        }

        switch (mouse.nAction)
        {
        case 0://����
            nFlags |= 0x10;
            break;
        case 1://˫��
            nFlags |= 0x20;
            break;
        case 2://����
            nFlags |= 0x40;
            break;
        case 3://�ſ�
            nFlags |= 0x80;
            break;
        default:

            break;
        }
        TRACE("mouse event : %08X x %d y %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
        case 0x11://�������
            //GetMessageExtraInfo ��õ�ǰϵͳ�еļ������Ķ�����Ϣ
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x12://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x14://�н�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x21://���˫��
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://�Ҽ�˫��
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://�н�˫��
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://�������
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://�Ҽ�����
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://�м�����
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://����ſ�
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://�Ҽ��ſ�
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://�н��ſ�
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://����ƶ�
            //mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        default:
            break;
        }
        lspacket.push_back(CPacket(4, NULL, 0));


    }
    else {
        OutputDebugStringA("��ȡ������ʧ��");
        return -1;
    }
    ;
    return 0;
}


int CCommand::SendScreen(std::list<CPacket>& lspacket, CPacket& packetIn)
{
    CImage screen;//GDI
    //��ȡ��ǰ�豸������
    HDC hScreen = ::GetDC(NULL);
    //λ��
    //255*255*255 �����24���ص�ɫ�ʱ�ʾ ����͸���Ⱦ���32������
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);

    //������һ��ȫ�ֵ��ڴ��
    auto hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    IStream* PStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &PStream);
    if (ret == S_OK)
    {
        //���ļ����浽�ڴ�����,
        screen.Save(PStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        PStream->Seek(bg, STREAM_SEEK_SET, NULL);
        //��ȡ�ڴ�ָ�벢������
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
    //���û�֪���㱻������
    //�����ʾ
    //��һ���رյĽӿ�
    //ģ̬�����������治�ܼ����ռ���ͷ�ģ̬ѡ�ĸ�
    CCommand * pcommand = (CCommand*)arg;
    pcommand->dlg.Create(IDD_DIALOG_INFO, NULL);
    pcommand->dlg.ShowWindow(SW_SHOW);
    //�ô���������Ļ
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
    //�����ö�
    pcommand->dlg.SetWindowPos(&pcommand->dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //�������Ĺ���
    ShowCursor(false);

    //���������Χ
    pcommand->dlg.GetWindowRect(&rect);
    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    ClipCursor(rect);
    //����������
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    //�����GetMessage����Ϣ�¡�20msһ�Ρ��Ǻ��̰߳󶨵ģ�ֻ���õ�����߳��ڵ���Ϣ
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        //����esc���˳�
        if (msg.wParam == 0x1b)
        {

            break;
        }
    }
    //�ָ�������
    ClipCursor(NULL);
    ShowCursor(true);
    //�ָ�����������
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    pcommand->dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}