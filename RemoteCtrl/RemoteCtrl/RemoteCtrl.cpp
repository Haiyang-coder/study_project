// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"SeverSocket.h"
#include<direct.h>
#include<Windows.h>

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
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());
    //CSeverSocket::getInstance()->Send();
    return 0;
}

#include<io.h>
#include<list>
typedef struct file_info
{
    file_info()
    {
        IsInvalid = 0;
        ISDirectory = -1;
        memset(szFileName, 0, sizeof(szFileName));
        HaveNext = TRUE;
    }
    BOOL IsInvalid;//是否有效
    char szFileName[256];
    BOOL ISDirectory;//是否为目录， 0否 1是
    BOOL HaveNext;//是否还有 0无 1有

}FILEINFO, *PFILEINFO;


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
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.length());
        finfo.ISDirectory = TRUE;
        finfo.HaveNext = FALSE;
        listFIleInfos.push_back(finfo);
        CPacket pack(2, (BYTE*) & finfo, sizeof(finfo ));
        CSeverSocket::getInstance()->Send(pack);
        OutputDebugStringA("没有权限访问目录");
        return -2;
    }
   
    _finddata_t fdata;
    int hfind = _findfirst("*", &fdata);
    if (hfind == -1)
    {
        OutputDebugStringA("没有找到任何文件");
        return -3;
    }

    do
    {
        FILEINFO finfo;
        finfo.ISDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        //listFIleInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
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
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024];
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CSeverSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        
    }
    CPacket pack(4, NULL, 0);
    CSeverSocket::getInstance()->Send(pack);
    fclose(pFile);
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
            //CSeverSocket* pserver = CSeverSocket::getInstance();
            //int count = 0;
            //if (pserver->InitSocket() == false)
            //{
            //    MessageBox(NULL, _T("Init socket error"), _T("Init socket failed"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}
            //while (pserver != nullptr)
            //{
            //   
            //   
            //    if (pserver->AcceptClient() == false)
            //    {
            //        MessageBox(NULL, _T("Accept error"), _T("Accept failed"), MB_OK | MB_ICONERROR);
            //    }
            //    if (count >= 3)
            //    {
            //       
            //        MessageBox(NULL, _T("try three times error"), _T("try three times failed"), MB_OK | MB_ICONERROR);
            //        exit(0);
            //    }
            //    count++;
            //}
            //int iRet = pserver->DealCommand();
            //todo

            int iCmd = 1;
            switch (iCmd)
            {
            case 1://产看磁盘的分区
                MakeDriverInfo();
                break;
            case 2://查看指定目录下面的文件
                MakeDirectoryInfo();
            default:
                break;
            case 3://打开文件
                RunFile();
                break;
            case 4:
                DownLoadFile();
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
