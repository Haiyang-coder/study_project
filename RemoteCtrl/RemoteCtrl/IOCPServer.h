#pragma once
#include"ThreadFuncBase.h"
#include"ThreadPool.h"
#include<Windows.h>
#include<map>
#include<winsock2.h>
#include<string>


class ShkClient
{
public:
	ShkClient();
	~ShkClient();

private:

};

enum OPERATOR
{
    ENone,
    EAccept,
    ERecv,
    ESedn,
    EError
};
class ShkOverlapped
{
public:
    ShkOverlapped();
    ~ShkOverlapped();
public:
    OVERLAPPED m_overlapped;
    OPERATOR m_operator;
    std::vector<char> m_buffer;
    CThreadWorker m_woker;

private:

};




class CIOCPServer :
    public CTheadFuncBase
{
public:
    CIOCPServer(const std::string ip = "0.0.0.0", short port = 9527) :m_pool(10)
    {
        
        m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
        if (bind(m_sock, (sockaddr*)&addr, sizeof(sockaddr)) == -1)
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return;
        }
        if (listen(m_sock, 3) == -1)
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return;
        }
        m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
        if (m_hIOCP == NULL)
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return;
        }
        CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
        m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIOCPServer::ThreadIOCP));

    };
    ~CIOCPServer() {};
private:
    int ThreadIOCP()
    {
        DWORD tranferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* lpOverlapped = NULL;
        if (GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE))
        {
            if (tranferred > 0 && CompletionKey != 0)
            {
                ShkOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, ShkOverlapped, m_overlapped);
                if (pOverlapped->m_operator)
                {

                }
            }
            

        }
        
    }
private:
    CTheadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    std::map<SOCKET, std::shared_ptr<ShkClient>> m_mapClient;
};

