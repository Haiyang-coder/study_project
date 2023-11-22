// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"SeverSocket.h"
#include<Windows.h>
#include"RemteServerTool.h"
#include"Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 唯一的应用程序对象 这是fiest的代码

CWinApp theApp;
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
            CCommand command;
            //ChooseAutoInvoke();
            CSeverSocket* pserver = CSeverSocket::getInstance();
            int ret = pserver->RunFunc(&CCommand::RunCommand, &command);
            switch (ret)
            {
            case -1:
                MessageBox(NULL, _T("Init socket error"), _T("Init socket failed"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            case -2:
                MessageBox(NULL, _T("try three times error"), _T("try three times failed"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            default:
                break;
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
