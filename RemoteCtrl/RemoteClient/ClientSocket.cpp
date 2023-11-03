#include "pch.h"
#include "ClientSocket.h"
#include "pch.h"
#include <Winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 409600
CPacket::CPacket()
{
}

CPacket::CPacket(const BYTE* pData, size_t& nSize)
{
	size_t i = 0;
	for (i = 0; i < nSize; i++)
	{
		//填充数据头
		if (*(WORD*)(pData + i) == 0xFEFF)
		{
			sHead = *(WORD*)(pData + i);
			i += 2;
			break;
		}
	}
	//如果数据包不全，或者数据包没有完全接收到
	if (i + 4 + 2 + 2 > nSize)
	{
		nSize = 0;
		return;
	}

	nLength = *(DWORD*)(pData + i);
	i += 4;
	if (nLength + i > nSize)
	{
		//数据没有接受完整
		nSize = 0;
		return;
	}

	sCmd = *(WORD*)(pData + i);
	i += 2;
	if (nLength > 4)
	{
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.data(), pData + i, nLength - 4);
		i += nLength - 4;
	}
	sSum = *(WORD*)(pData + i);
	i += 2;
	size_t sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;
	}
	if (sum == sSum)
	{
		nSize = i;//head length data
	}
	return;
}

CPacket::CPacket(const CPacket& packet)
{
	sHead = packet.sHead;//数据头
	nLength = packet.nLength;//数据长度（从控制命令到校验的长度）
	sCmd = packet.sCmd;//控制命令
	strData = packet.strData;//数据
	sSum = packet.sSum;//校验
}

CPacket& CPacket::operator=(const CPacket& packet)
{
	if (this == &packet)
	{
		return *this;
	}
	sHead = packet.sHead;//数据头
	nLength = packet.nLength;//数据长度（从控制命令到校验的长度）
	sCmd = packet.sCmd;//控制命令
	strData = packet.strData;//数据
	sSum = packet.sSum;//校验
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
{
	sHead = 0xFEFF;
	nLength = nSize + 4;
	sCmd = nCmd;
	if (nSize > 0)
	{
		strData.resize(nSize);
		memcpy((void*)strData.data(), pData, nSize);
	}
	else {
		strData.clear();
	}
	sSum = 0;
	for (size_t i = 0; i < strData.size(); i++)
	{
		sSum += BYTE(strData[i]) & 0xFF;
	}
}

CPacket::~CPacket()
{
}

int CPacket::Size()
{
	return nLength + 6;
}

const char* const CPacket::Data()
{
	strOut.resize(nLength + 6);
	BYTE* pData = (BYTE*)strOut.c_str();
	*(WORD*)pData = sHead; pData += 2;
	*(DWORD*)pData = nLength; pData += 4;
	*(WORD*)pData = sCmd; pData += 2;
	memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
	*(WORD*)pData = sSum;
	return strOut.c_str();
}



//私有静态成员必须初始化 
CClientSocket* CClientSocket::m_pinstance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;
CClientSocket::CClientSocket()
{
	m_client_sock = INVALID_SOCKET;
	m_pinstance = NULL;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("无法初始化套接字环境"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	
	if (m_client_sock < 0)
	{
		exit(0);
	}
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
}

CClientSocket::~CClientSocket()
{
	closesocket(m_client_sock);
	WSACleanup();

}

BOOL CClientSocket::InitSockEnv()
{
	//初始化套接字
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

bool CClientSocket::InitSocket(DWORD strIPAddress, int nPort)
{
	if (m_client_sock != INVALID_SOCKET)
	{
		closeSocket();
	}
	m_client_sock = socket(PF_INET, SOCK_STREAM, 0);
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(sockaddr_in));
	serv_adr.sin_addr.s_addr = htonl(strIPAddress);
	serv_adr.sin_family = AF_INET;
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("IP地址格式不正确");
		perror("ip地址不对");
		return false;
	}
	serv_adr.sin_port = htons(nPort);
	int ret = connect(m_client_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret < 0)
	{
		AfxMessageBox("连接失败");
		perror("connect");
		return false;
	}

	return true;
}


CClientSocket* CClientSocket::getInstance()
{
	if (m_pinstance == nullptr)
	{
		m_pinstance = new CClientSocket();
	}
	return m_pinstance;
}

void CClientSocket::releaseInstance()
{
	if (m_pinstance != NULL)
	{
		CClientSocket* tem = m_pinstance;
		m_pinstance = nullptr;
		delete tem;

	}
}

int CClientSocket::DealCommand()
{
	if (m_client_sock == -1)
	{
		return -1;
	}
	char* buffer = m_buffer.data();
	static size_t index = 0;
	while (true)
	{
		
		size_t len = recv(m_client_sock, buffer + index, BUFFER_SIZE - index, 0);
		if (len <= 0 && index == 0)
		{
			return -1;
		}
		index += len;
		len = index;
		//无论收多少数据都丢给流水线构造
		//流水线告诉你构造了多少字节的数据
		CPacket packet((BYTE*)buffer, len);
		m_packet = packet;
		if (len > 0)
		{
			//根据流水线的反馈，你把缓存中已经构造完成的数据拿出缓存
			memmove(buffer, buffer + len, BUFFER_SIZE - len);
			index -= len;
			return packet.sCmd;
		}
		//todo：出现了构造失败的情况会直接抛弃所有的缓存，这样不行吧？

	}
	return -1;
}

bool CClientSocket::Send(const char* pData, size_t nize)
{
	if (m_client_sock == -1)
	{
		return false;
	}
	return send(m_client_sock, pData, nize, 0) > 0;
}

bool CClientSocket::Send(CPacket& pack)
{
	if (m_client_sock == -1)
	{
		return false;
	}
	return  send(m_client_sock, pack.Data(), pack.Size(), 0);
}

bool CClientSocket::GetFilePath(std::string& strPath)
{
	if (m_packet.sCmd == 2 ||
		m_packet.sCmd == 3 ||
		m_packet.sCmd == 4)

	{
		strPath = m_packet.strData;
		return true;
	}
	return false;
}

bool CClientSocket::GetMouseEvent(MOUSEEV& mouse)
{
	if (m_packet.sCmd == 5)
	{
		memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
		return true;
	}
	return false;
}

const CPacket& CClientSocket::Getpack()
{
	return m_packet;
}

void CClientSocket::closeSocket()
{
	closesocket(m_client_sock);
	m_client_sock = INVALID_SOCKET;
}








