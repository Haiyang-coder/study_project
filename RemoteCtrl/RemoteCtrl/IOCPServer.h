#pragma once
#include"ThreadPool.h"
#include<map>
#include<string>
#include<WinSock2.h>
#include"SafeQueue.h"
#include<MSWSock.h>

class ShkClient;
typedef std::shared_ptr<ShkClient> PCLIENT;

enum OPERATOR
{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};
class CIOCPServer;

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
    CIOCPServer* m_server;//服务器对象
    

};



template<OPERATOR>
class AcceptOverlapped :public ShkOverlapped, CTheadFuncBase
{
public:
    AcceptOverlapped(PCLIENT& client);
    AcceptOverlapped();
    int AcceptWorker();
    
public:
    PCLIENT m_client;
};
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<OPERATOR>
class RecvOverlapped :public ShkOverlapped, CTheadFuncBase
{
public:
    RecvOverlapped() :m_operator(ERecv), m_woker(this, &RecvOverlapped::RecvWorker)
    {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024 * 256);
    }
    int  RecvWorker()
    {
    }
};
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;

template<OPERATOR>
class SendOverlapped :public ShkOverlapped, CTheadFuncBase
{
public:
    SendOverlapped() :m_operator(ESend), m_woker(this, &SendOverlapped::SendWorker)
    {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024 * 256);
    }
    int SendWorker()
    {
    }
};
typedef SendOverlapped<ESend> SENDOVERLAPPED;

template<OPERATOR>
class ErrorOverlapped :public ShkOverlapped, CTheadFuncBase
{
public:
    ErrorOverlapped();
    int ErrorWorker();
};
typedef ErrorOverlapped<EError> ErrorOVERLAPPED;


class ShkClient
{


public:
    ShkClient();
    ~ShkClient();

    void SetOverlapped(PCLIENT& ptr);

    operator SOCKET();


    operator PVOID();
    operator LPOVERLAPPED();
    operator LPDWORD();

public:
    SOCKET m_sock;
    std::vector<char> m_buffer;
    sockaddr m_localAddr;
    sockaddr m_remoteAddr;
    bool m_isBusy;
    DWORD m_received;
    std::shared_ptr<ACCEPTOVERLAPPED>  m_overlapped;


};

class CIOCPServer :
    public CTheadFuncBase
{
public:
    CIOCPServer(const std::string ip = "0.0.0.0", short port = 9527);
    ~CIOCPServer();
    bool StartServer();
    int AcceptClient();
    bool NewAccept();

private:
    void CreateSocket();
    int ThreadIOCP();
   


private:
    CTheadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    std::map<SOCKET, std::shared_ptr<ShkClient>> m_mapClient;
    CSafeQueue<ShkClient> m_lstClient;
    sockaddr_in m_addr;
    

};
