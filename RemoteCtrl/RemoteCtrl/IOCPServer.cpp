#include"pch.h"
#include"IOCPServer.h"
#include <ws2tcpip.h>


ShkOverlapped::ShkOverlapped()
{

}

ShkOverlapped::~ShkOverlapped()
{
	m_buffer.clear();
}

template<OPERATOR op>
AcceptOverlapped<op>::AcceptOverlapped(PCLIENT& client)
{
	m_operator = EAccept;
	m_woker = CThreadWorker(this, (FUNCTYPE)&AcceptOverlapped::AcceptWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = NULL;
}

template<OPERATOR op>
AcceptOverlapped<op>::AcceptOverlapped()
{
	m_operator = EAccept;
	m_woker = CThreadWorker(this, (FUNCTYPE)& AcceptOverlapped::AcceptWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = NULL;
}

template<OPERATOR op>
int AcceptOverlapped<op>::AcceptWorker()
{
	INT lLength = 0, rLength = 0;
	if (*(LPDWORD)*m_client > 0)
	{
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&m_client->m_localAddr, &lLength, (sockaddr**)&m_client->m_remoteAddr, &rLength);
		
		int ret = WSARecv((SOCKET)*m_client, m_client->RecvWSAbuffer(), 1, *m_client, &m_client->Getflags(), *m_client, NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			//todo 报错
		}
		if (!m_server->NewAccept())
		{
			return -2;
		}
	}
	return -1;
}

CIOCPServer::CIOCPServer(const std::string ip, short port) :m_pool(10)
{
	m_sock = INVALID_SOCKET;
	m_hIOCP = INVALID_HANDLE_VALUE;
	inet_pton(AF_INET, ip.c_str(), &(m_addr.sin_addr));
}

CIOCPServer::~CIOCPServer()
{
	closesocket(m_sock);
	auto it = m_mapClient.begin();
	for (; it != m_mapClient.end(); it++)
	{
		it->second.reset();
	}
	m_mapClient.clear();
	CloseHandle(m_hIOCP);
	m_pool.Stop();
}

bool CIOCPServer::StartServer()
{

	CreateSocket();
	m_addr.sin_family = AF_INET;
	if (bind(m_sock, (sockaddr*)&m_addr, sizeof(sockaddr)) == -1)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	if (listen(m_sock, 3) == -1)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
	if (m_hIOCP == NULL)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_hIOCP = INVALID_HANDLE_VALUE;
		return false;
	}
	CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
	m_pool.Invoke();
	m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIOCPServer::ThreadIOCP));
	if (!NewAccept())
	{
		return false;
	}
	return true;

}

int CIOCPServer::AcceptClient()
{
	return 0;
}

bool CIOCPServer::NewAccept()
{
	PCLIENT pClient(new ShkClient());
	pClient->SetOverlapped(pClient);
	m_mapClient.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
	if (AcceptEx(m_sock, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient) == FALSE)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_hIOCP = INVALID_HANDLE_VALUE;
		return false;
	}
}

void CIOCPServer::CreateSocket()
{
	m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int opt = 1;
	setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

int CIOCPServer::ThreadIOCP()
{
	DWORD tranferred = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* lpOverlapped = NULL;
	if (GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE))
	{
		if (tranferred > 0 && CompletionKey != 0)
		{
			ShkOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, ShkOverlapped, m_overlapped);
			switch (pOverlapped->m_operator)
			{
			case EAccept:
			{
				ACCEPTOVERLAPPED* pAccept = (ACCEPTOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pAccept->m_woker);
			}
			break;
			case ERecv:
			{
				RECVOVERLAPPED* recv = (RECVOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(recv->m_woker);
			}
			break;
			case ESend:
			{

			}
			break;
			case EError:
			{

			}
			break;
			default:
				break;
			}

		}
		else
		{
			return -1;
		}


	}

}

ShkClient::operator LPOVERLAPPED()
{
	return &m_overlapped->m_overlapped;
}

ShkClient::ShkClient() 
:m_overlapped(new ACCEPTOVERLAPPED()),
m_RecvOverlapped(new RECVOVERLAPPED()),
m_SendOverlapped(new SENDOVERLAPPED())
//m_SafeQueueVecSendBuffer(this, (SENDCALLBACKK)&ShkClient::SendData)
{
	
	m_SafeQueueVecSendBuffer = new CSafeSendQueue<std::vector<char>>(this, (SENDCALLBACKK)&ShkClient::SendData);
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_localAddr, 0, sizeof(m_localAddr));
	memset(&m_remoteAddr, 0, sizeof(m_remoteAddr));
	m_isBusy = false;
	m_flags = 0;
}

ShkClient::~ShkClient()
{
	m_buffer.clear();
	closesocket(m_sock);
	m_RecvOverlapped.reset();
	m_SendOverlapped.reset();
	m_overlapped.reset();
	m_SafeQueueVecSendBuffer->Clear();
	//todo delete m_safe
	
}

void ShkClient::SetOverlapped(PCLIENT& ptr)
{
	m_overlapped->m_client = ptr.get();
	m_RecvOverlapped->m_client = ptr.get();
	m_SendOverlapped->m_client = ptr.get();
}

ShkClient::operator SOCKET()
{
	return m_sock;
}

ShkClient::operator PVOID()
{
	return &m_buffer[0];
}

ShkClient::operator LPDWORD()
{
	return &m_received;
}

LPWSABUF ShkClient::RecvWSAbuffer()
{
	return &m_RecvOverlapped->m_wsaBuffer;
}

LPWSABUF ShkClient::SendWSAbuffer()
{
	return &m_SendOverlapped->m_wsaBuffer;
}



sockaddr_in* ShkClient::GetLocalAddr()
{
	return &m_localAddr;
}

sockaddr_in* ShkClient::GetRemoteAddr()
{
	return &m_remoteAddr;
}

size_t ShkClient::GetBufferSize() const
{
	return m_buffer.size();
}

int ShkClient::Recv()
{
	int length = recv(m_sock, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
	if (length <= 0)
	{
		return -1;
	}
	m_used += (size_t)length;
	//todo:有一个解析数据还没有完成
	return 0;
}

int ShkClient::Send(void* buffer, size_t nSize)
{
	std::vector<char> data(nSize);
	memcpy(data.data(), buffer, nSize);
	if (m_SafeQueueVecSendBuffer->PushBack(data))
	{
		return 0;
	}
	return -1;
}

DWORD& ShkClient::Getflags()
{
	return m_flags;
}

int ShkClient::SendData(std::vector<char>& data)
{
	if (m_SafeQueueVecSendBuffer->Size() > 0)
	{
		int ret = WSASend(m_sock, SendWSAbuffer(), 1, &m_received, m_flags, &m_SendOverlapped->m_overlapped, NULL);
		if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING)
		{
			return -1;
		}
	}
}


template<OPERATOR op>
ErrorOverlapped<op>::ErrorOverlapped() {
	m_operator = EError;
	m_woker = CThreadWorker(this, (FUNCTYPE)&ErrorOverlapped::ErrorWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
}

template<OPERATOR op>
int ErrorOverlapped<op>::ErrorWorker()
{
	return 0;
}

template<OPERATOR op>
RecvOverlapped<op>::RecvOverlapped()
{
	m_operator = ERecv;
	m_woker = CThreadWorker(this, (FUNCTYPE)&RecvOverlapped::RecvWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}

template<OPERATOR op>
int RecvOverlapped<op>::RecvWorker()
{
	int ret = m_client->Recv();
	return ret;
}

template<OPERATOR op>
SendOverlapped<op>::SendOverlapped()
{
	m_operator = ESend;
	m_woker = CThreadWorker(this, (FUNCTYPE)&SendOverlapped::SendWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}

template<OPERATOR op>
int SendOverlapped<op>::SendWorker()
{
	//send可能不会立即发送完成WSA_IO_PENDING(正在处理状态)
	return 0;
}