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
    virtual ~ShkOverlapped();
public:
    OVERLAPPED m_overlapped;
    OPERATOR m_operator;//操作
    std::vector<char> m_buffer;//缓冲区
    CThreadWorker m_woker;//工作线程
    CIOCPServer* m_server;//服务器对象
    ShkClient* m_client;//对应的客户端
    WSABUF m_wsaBuffer;

};



template<OPERATOR>
class AcceptOverlapped :public ShkOverlapped, CThreadFuncBase
{
public:
    AcceptOverlapped(PCLIENT& client);
    AcceptOverlapped();
    int AcceptWorker();
    
};
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<OPERATOR>
class RecvOverlapped :public ShkOverlapped, CThreadFuncBase
{
public:
    RecvOverlapped();
    int  RecvWorker();
};
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;

template<OPERATOR>
class SendOverlapped :public ShkOverlapped, CThreadFuncBase
{
public:
    SendOverlapped();
    int SendWorker();
};
typedef SendOverlapped<ESend> SENDOVERLAPPED;

template<OPERATOR>
class ErrorOverlapped :public ShkOverlapped, CThreadFuncBase
{
public:
    ErrorOverlapped();
    int ErrorWorker();
};
typedef ErrorOverlapped<EError> ErrorOVERLAPPED;

class ShkClient:public CThreadFuncBase
{


public:
    ShkClient();
    ~ShkClient();

    void SetOverlapped(PCLIENT& ptr);

    operator SOCKET();
    operator PVOID();
    operator LPOVERLAPPED();
    operator LPDWORD();
    LPWSABUF RecvWSAbuffer();
    LPWSABUF SendWSAbuffer();
    sockaddr_in* GetLocalAddr();
    sockaddr_in* GetRemoteAddr();
    size_t GetBufferSize() const;
    int Recv();
    int Send(void* buffer, size_t nSize);
    DWORD& Getflags();
    int SendData(std::vector<char>& data);
public:
    SOCKET m_sock;
    std::vector<char> m_buffer;
    sockaddr_in m_localAddr;
    sockaddr_in m_remoteAddr;
    bool m_isBusy;
    size_t m_used;//已经使用的缓冲区大小
    DWORD m_received;
    std::shared_ptr<ACCEPTOVERLAPPED>  m_overlapped;
    DWORD m_flags;
    std::shared_ptr<RECVOVERLAPPED> m_RecvOverlapped;
    std::shared_ptr<SENDOVERLAPPED> m_SendOverlapped;
    CSafeSendQueue<std::vector<char>>* m_SafeQueueVecSendBuffer;//发送数据队列
};

class CIOCPServer :
    public CThreadFuncBase
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


