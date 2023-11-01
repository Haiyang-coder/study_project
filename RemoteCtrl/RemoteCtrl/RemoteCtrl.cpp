// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"SeverSocket.h"
#include<direct.h>
#include<Windows.h>
#include<atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象 这是fiest的代码

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))
            strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
        

    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
 
}

int MakeDriverInfo()
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
    result += ',';
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

#include<io.h>
#include<list>



int MakeDirectoryInfo()
{
    std::string strPath;//指定的路径
    std::list<FILEINFO> listFIleInfos;//路径下面的文件列表
    if (CSeverSocket::getInstance()->GetFilePath(strPath) == false)
    {
        OutputDebugStringA("当前命令不是获取文件列表的命令");
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0 )
    {
        FILEINFO finfo1;
        finfo1.HaveNext = FALSE;
        listFIleInfos.push_back(finfo1);
        CPacket pack(2, (BYTE*) & finfo1, sizeof(finfo1 ));
        CSeverSocket::getInstance()->Send(pack);
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
        CPacket pack(2, (BYTE*)&finfo2, sizeof(finfo2));
        CSeverSocket::getInstance()->Send(pack);
        return -3;
    }

    do
    {
        FILEINFO finfo3;
        finfo3.ISDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo3.szFileName, fdata.name, strlen(fdata.name));
        TRACE("%s\r\n", finfo3.szFileName);
        CPacket pack(2, (BYTE*)&finfo3, sizeof(finfo3));
        CSeverSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));

    FILEINFO finfo;
    finfo.HaveNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile()
{
    std::string strPath = "";
    CSeverSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

int DownLoadFile()
{
    std::string strPath = "";
    CSeverSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    //这里用fopen_s是安全打开方式，因为fopen有可能出现
    //返回了句柄但是你无法操作文件的现象
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err != 0)
    {
        CPacket pack(4, (BYTE*)&data, 8);
        CSeverSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL)
    {
        //先发给用户这个文件有多大
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        CSeverSocket::getInstance()->Send(head);
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024];
        size_t rlen = 0;
        do {
            //而后循环发送文件部分一次发送1024个字节
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CSeverSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        
    }
    CPacket pack(4, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    fclose(pFile);
    return 0;
}

int MouseEvent()
{
    MOUSEEV mouse;
    if (CSeverSocket::getInstance()->GetMouseEvent(mouse))
    {
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
        CPacket pack(4, NULL, 0);
        CSeverSocket::getInstance()->Send(pack);

        
    }
    else {
        OutputDebugStringA("获取鼠标参数失败");
        return -1;
    }
    ;
    return 0;
}


int SendScreen()
{
    CImage screen;//GDI
    //获取当前设备上下文
    HDC hScreen =  ::GetDC(NULL);
    //位宽
    //255*255*255 这就是24比特的色彩表示 加上透明度就是32比特了
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
   
    //创建了一个全局的内存块
    auto hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    IStream* PStream = NULL;
    HRESULT ret =  CreateStreamOnHGlobal(hMem, TRUE, &PStream);
    if (ret == S_OK)
    {
        //将文件保存到内存区域,
        screen.Save(PStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        PStream->Seek(bg, STREAM_SEEK_SET, NULL);
        //获取内存指针并且上锁
        PBYTE pData = (PBYTE) GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, pData, nSize);
        CSeverSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);

        
    }
    PStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}

#include"LockDialog.h"
CLockDialog dlg;
unsigned threadid = 0;
unsigned  threadLockDlg(void* arg)
{
    //让用户知道你被锁机了
    //最顶层显示
    //留一个关闭的接口
    //模态（不结束后面不能激活独占）和非模态选哪个
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    //让窗口铺满屏幕
    CRect rect;
    //rect.left = 0;
    //rect.top = 0;
    //rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    //rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) - 50;
    //窗口置顶
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //限制鼠标的功能
    ShowCursor(false);

    //限制鼠标活动范围
    dlg.GetWindowRect(&rect);
    ClipCursor(rect);
    //隐藏任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    MSG msg;
    //这里的GetMessage，消息甭。20ms一次。是和线程绑定的，只能拿到这个线程内的消息
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
    ShowCursor(true);
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}
int LockMachine()
{
    //判断是否已经锁机了
    if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
    {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, nullptr, 0, &threadid);
    }
    CPacket pack(7, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    
    return 0;
}
 
int UNLockMachine()
{
    //::SendMessage(dlg.m_hWnd,WM_KEYDOWN, 0x1b, 0x01E001);
    PostThreadMessage(threadid ,WM_KEYDOWN, 0x1b, 0x01E001);
    CPacket pack(7, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect()
{
    CPacket pack(1981, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

int DeleteLocalFile()
{
    std::string strpath = "";
    CSeverSocket::getInstance()->GetFilePath(strpath);
    //进行一个字符集的转变
    //TCHAR sPath[MAX_PATH];
    //mbstowcs(sPath, strpath.c_str(), strpath.size() + 1);
    DeleteFileA(strpath.c_str());
    CPacket pack(9, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    return 0;
}

int ExcuteCommand(int nCmd)
{
    int iRet = 0;
    switch (nCmd)
    {
    case 1://产看磁盘的分区
        iRet = MakeDriverInfo();
        break;
    case 2://查看指定目录下面的文件
        iRet = MakeDirectoryInfo();
        break;
    case 3://打开文件
        iRet = RunFile();
        break;
    case 4://下载文件
        iRet = DownLoadFile();
        break;
    case 5://鼠标事件操作
        iRet = MouseEvent();
        break;
    case 6://屏幕远程监控==发送屏幕的截图
        iRet = SendScreen();
        break;
    case 7://锁机
        iRet = LockMachine();
        break;
    case 8://锁机
        iRet = UNLockMachine();
        break;
    case 9://删除文件
        iRet = DeleteLocalFile();
        break;
    case 1981://连接测试
        iRet = TestConnect();
        break;
    default:
        break;

    }
    return iRet;
}
int main()
{
    int nRetCode = 0;
    HMODULE hModule = ::GetModuleHandle(nullptr);
    std::cout << "hello word \n" << std::endl;
    std::cout << std::flush;

    
    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。sadasdjhi
            CSeverSocket* pserver = CSeverSocket::getInstance();
            int count = 0;
            if (pserver->InitSocket() == false)
            {
                MessageBox(NULL, _T("Init socket error"), _T("Init socket failed"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (pserver != nullptr)
            {
                if (pserver->AcceptClient() == false)
                {
                    MessageBox(NULL, _T("Accept error"), _T("Accept failed"), MB_OK | MB_ICONERROR);
                    if (count >= 3)
                    {

                        MessageBox(NULL, _T("try three times error"), _T("try three times failed"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    count++;
                }
                int iRet = pserver->DealCommand();
                if (iRet > 0)
                {
                    iRet = ExcuteCommand(pserver->Getpack().sCmd);
                    if (iRet != 0)
                    {
                        TRACE("执行命令失败：%d ret = %d \r\n", pserver->Getpack().sCmd, iRet);
                    }
                    pserver->CloseSocket();
                }

            }
           
           
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
