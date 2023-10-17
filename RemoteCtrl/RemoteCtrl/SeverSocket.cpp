#include "pch.h"
#include "SeverSocket.h"

//私有静态成员必须初始化
CSeverSocket* CSeverSocket::m_pinstance = nullptr;
CSeverSocket::CHelper CSeverSocket::m_helper;
CSeverSocket::CSeverSocket()
{
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

bool CSeverSocket::InitSocket()
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

void CSeverSocket::DealCommand()
{
	char buffer[1024];
	while (true)
	{
		recv(m_client, buffer, sizeof(buffer), 0);
		send(m_client, buffer, sizeof(buffer), 0);
		
	}
	closesocket(m_client);
}









