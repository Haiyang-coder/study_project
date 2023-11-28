#include"pch.h"
#include"IOCPServer.h"
#include <ws2tcpip.h>


ShkOverlapped::ShkOverlapped()
{
}

ShkOverlapped::~ShkOverlapped()
{
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
	if (m_client->m_received > 0)
	{
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&m_client->m_localAddr, &lLength, (sockaddr**)&m_client->m_remoteAddr, &rLength);

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

ShkClient::ShkClient() :m_overlapped(new ACCEPTOVERLAPPED())
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_localAddr, 0, sizeof(m_localAddr));
	m_isBusy = false;
}

ShkClient::~ShkClient()
{
	closesocket(m_sock);
}

void ShkClient::SetOverlapped(PCLIENT& ptr)
{
	m_overlapped->m_client = ptr;
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
