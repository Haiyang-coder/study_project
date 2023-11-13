#include "pch.h"
#include "SeverSocket.h"
#define BUFFER_SIZE 2048000
//私有静态成员必须初始化 
CSeverSocket* CSeverSocket::m_pinstance = nullptr;
CSeverSocket::CHelper CSeverSocket::m_helper;
CSeverSocket::CSeverSocket()
{
	m_client = INVALID_SOCKET;
	m_serv_sock = INVALID_SOCKET;
	m_pinstance = NULL;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("无法初始化套接字环境"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_serv_sock < 0)
	{
		exit(0);
	}

}

CSeverSocket::~CSeverSocket()
{
	closesocket(m_serv_sock);
	WSACleanup();

}

BOOL CSeverSocket::InitSockEnv()
{
	//初始化套接字
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

bool CSeverSocket::InitSocket(short port)
{

	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(sockaddr_in));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.S_un.S_addr = INADDR_ANY;
	serv_adr.sin_port = htons(9527);
	//绑定
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
	{
		return false;
	}
	//todo没有进行验证
	if (listen(m_serv_sock, 1) == -1)
	{
		return false;
	}
	
	return true;
}

int CSeverSocket::RunFunc(SOCKE_CALLBACK callbackFunc, void* arg, short port)
{
	
	bool ret = InitSocket(port);
	if(!ret)
	{
		return -1;
	}
	std::list<CPacket> listPacket;
	m_callback = callbackFunc;
	m_arg = arg;
	int count = 0;
	while (true)
	{
		if (AcceptClient() == false)
		{
			if (count >= 3)
			{
				return -2;
			}
			count++;
		}
		int ret = DealCommand();
		if (ret > 0)
		{
			m_callback(m_arg, ret, listPacket, m_packet);
			while (listPacket.size() > 0)
			{
				Send(listPacket.front());
				listPacket.pop_front();
			}
			
		}
		CloseSocket();
	}
	
	return 0;
}

bool CSeverSocket::AcceptClient()
{
	char buffer[1024];
	sockaddr_in client_adr;
	memset(&client_adr, 0, sizeof(client_adr));
	int cli_sz = sizeof(sockaddr_in);
	m_client = accept(m_serv_sock, (sockaddr*)&client_adr, &cli_sz);
	if (m_client == -1)
	{
		return false;
	}
	TRACE("m_client : %d\r\n", m_client);
	return true;
}

CSeverSocket* CSeverSocket::getInstance()
{
	if (m_pinstance == nullptr)
	{
		m_pinstance = new CSeverSocket();
	}
	return m_pinstance;
}

void CSeverSocket::releaseInstance()
{
	if (m_pinstance != NULL)
	{
		CSeverSocket* tem = m_pinstance;
		m_pinstance = nullptr;
		delete tem;

	}
}

int CSeverSocket::DealCommand()
{
	if (m_client == -1)
	{
		return -1;
	}
	//char buffer[1024];
	char* buffer = new char[BUFFER_SIZE];
	if (buffer == NULL)
	{
		TRACE("内存不足\r\n");
		return -2;
	}
	memset(buffer, 0, BUFFER_SIZE);
	size_t index = 0;
	while (true)
	{
		//目前没有处理粘包问题，默认每次接受只来一个数据包
		size_t len  = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
		if (len <= 0)
		{
			delete[] buffer;
			return -1;
		}
		index += len;
		len = index;
		CPacket packet((BYTE*)buffer, len);
		m_packet = packet;
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUFFER_SIZE - len);
			index -= len;
			delete[] buffer;
			return packet.sCmd;
		}
		
	}
	delete[] buffer;
	return -1;
}

bool CSeverSocket::Send(const char* pData, size_t nize)
{
	if (m_client == -1)
	{
		return false;
	}
	return send(m_client, pData, nize, 0) > 0;
}

bool CSeverSocket::Send(CPacket& pack)
{
	if (m_client == -1)
	{
		CRemteServerTool::Dump((BYTE*)pack.Data(), pack.Size());
		return false;
	}
	CRemteServerTool::Dump((BYTE*)pack.Data(), pack.Size());
	bool end = send(m_client, pack.Data(), pack.Size(), 0) > 0;
	return end;
}


void CSeverSocket::CloseSocket()
{
	if (m_client != INVALID_SOCKET)
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
	
}








