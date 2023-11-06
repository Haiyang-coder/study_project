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
                    iRet = command.ExcuteCommand(pserver->Getpack().sCmd);
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
