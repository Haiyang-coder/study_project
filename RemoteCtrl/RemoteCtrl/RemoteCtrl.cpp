// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"SeverSocket.h"
#include<Windows.h>
#include"RemteServerTool.h"
#include"Command.h"
#include<thread>
#include<conio.h>
#include"SafeQueue.h"
#include<MSWSock.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 唯一的应用程序对象 这是fiest的代码

CWinApp theApp;
void showError()
{
    LPWSTR lpMEessageBuf = NULL;
    //strerror(errno)://标准c语言库
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMEessageBuf, 0, NULL);
    OutputDebugString(lpMEessageBuf);
    LocalFree(lpMEessageBuf);
    exit(0);
}
bool isAdmin() 
{
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess, TOKEN_QUERY, &hToken))
    {
        showError();
        return false;
    }
    TOKEN_ELEVATION eve;
    DWORD len = 0;
    if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE)
    {
        showError();
        return false;
    }
    if (len == sizeof(eve))
    {
        CloseHandle(hToken);
        return eve.TokenIsElevated;
    }
    printf("length of tokeninfomation is %d\r\n");
    return false;
}

void WriteRegisterTable(const CString& strPath)
{
    int ret = 0;
    CString strBugkey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    CString strpath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    char sPath[MAX_PATH] = "";
    char sSys[MAX_PATH] = "";
    std::string strExe = "\\RemoteCtrl.exe ";
    GetCurrentDirectoryA(MAX_PATH, sPath);
    GetSystemDirectoryA(sSys, sizeof(sSys));
    std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
    ret = system(strCmd.c_str());
    TRACE("ret system = %d \r\n", ret);
    HKEY hkey = NULL;
    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strBugkey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hkey);
    if (ret != ERROR_SUCCESS)
    {
        RegCloseKey(hkey);
        MessageBox(NULL, _T("设置自动开机启动失败，是否权限不足 \r\n 程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }


    ret = RegSetValueEx(hkey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strpath, strpath.GetLength() * sizeof(TCHAR));
    if (ret != ERROR_SUCCESS)
    {
        RegCloseKey(hkey);
        MessageBox(NULL, _T("设置自动开机启动失败，是否权限不足 \r\n 程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
    RegCloseKey(hkey);
}
void WriteStartUpDir(const CString& strPath)
{
    
    CString strcmd =  GetCommandLine();
    strcmd.Replace(_T("\""), _T(""));
    BOOL ret = CopyFile(strcmd, strPath, FALSE);
    if (ret == FALSE)
    {
        MessageBox(NULL, _T("复制文件失败，是否权限不足\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        exit(0);
    }
}
void ChooseAutoInvoke()
{
    //开机启动的权限和启动用户的权限是一样的，权限不够不可以
    //开机启动对环境变量有影响，如果依赖动态库，可能启动失败
    CString strPathStartup = _T("C:\\Users\\lenovo\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\");
    CString strpath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    if (PathFileExists(strpath))
    {
        return;
    }
  
    CString strInfo = _T("该程序只能用于合法用途");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret != IDOK)
    {
        exit(0);
    }
    //WriteRegisterTable();
    WriteStartUpDir(strPathStartup);
   
}
void RunAdmin()
{
    HANDLE hToken = NULL;
    BOOL ret = LogonUser(_T("Administrator"), NULL, NULL, LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT,&hToken);
    if (!ret)
    {
        showError();
        exit(0);
    }
    OutputDebugString(L"logon admin success\r\n");
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = { 0 };
    TCHAR sPath[MAX_PATH] = _T("");
    GetCurrentDirectory(MAX_PATH, sPath);
    CString strCmd = sPath;
    strCmd += _T("RemoteCtrl.exe");
    //ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
    ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE,NULL, (LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL,NULL, &si, &pi);
    if (!ret)
    {
        showError();
        TRACE("创建进程失败");
    }
    WaitForSingleObject(pi.hProcess, INFINITY);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

bool Init()
{
    HMODULE hModule = ::GetModuleHandle(nullptr);
    if (hModule == nullptr)
    {
        wprintf(L"错误: GetModuleHandle 失败\n");
        return false;
    }
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    {
        // TODO: 在此处为应用程序的行为编写代码。
        wprintf(L"错误: MFC 初始化失败\n");
        return false;
    }
}
void test()
{
    CSafeQueue<std::string> lstStrings;
    ULONGLONG tick0 = GetTickCount64();
    ULONGLONG tick1 = GetTickCount64();
    ULONGLONG ticktoal = GetTickCount64();
    int count = 0;

    while (true)
    {
        if (count++ >= 10000)
        {
            break;
        }
        if (GetTickCount64() - tick0 > 13)
        {
            lstStrings.PushBack("hello world ");
            tick0 = GetTickCount64();
        }
        if (GetTickCount64() - tick1 > 20)
        {
            std::string str;
            lstStrings.PopFront(str);
            TRACE("str is = %s\r\n", str.c_str());
            tick1 = GetTickCount64();
        }
        Sleep(1);
    }
    lstStrings.Clear();
    
}


int main()
{
   
    //if (!Init())
    //{
    //    return 1;
    //}


    //if (isAdmin())
    //{
    //    
    //    TRACE("current is run ia administrator!\r\n");
    //}
    //else
    //{
    //    //RunAdmin();
    //    TRACE("current is run is a normal user!\r\n");
    //}
    //CCommand command;
    ////ChooseAutoInvoke();
    //CSeverSocket* pserver = CSeverSocket::getInstance();
    //int ret = pserver->RunFunc(&CCommand::RunCommand, &command);
    //switch (ret)
    //{
    //case -1:
    //    MessageBox(NULL, _T("Init socket error"), _T("Init socket failed"), MB_OK | MB_ICONERROR);
    //    exit(0);
    //    break;
    //case -2:
    //    MessageBox(NULL, _T("try three times error"), _T("try three times failed"), MB_OK | MB_ICONERROR);
    //    exit(0);
    //    break;
    //default:
    //    break;
    //}
    
}
