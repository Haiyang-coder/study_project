#include "pch.h"
#include "ClientSocket.h"
#include "pch.h"
#include <Winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 2048000
CPacket::CPacket()
{
}

CPacket::CPacket(const BYTE* pData, size_t& nSize) :hEvent(INVALID_HANDLE_VALUE)
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
	if (this != &packet)
	{
		sHead = packet.sHead;//数据头
		nLength = packet.nLength;//数据长度（从控制命令到校验的长度）
		sCmd = packet.sCmd;//控制命令
		strData = packet.strData;//数据
		sSum = packet.sSum;//校验
		hEvent = packet.hEvent;
	}
	

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
	hEvent = packet.hEvent;
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize,  HANDLE hEvent)
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
	this->hEvent = hEvent;
}

CPacket::~CPacket()
{
}

int CPacket::Size()
{
	return nLength + 6;
}

const char* const CPacket::Data(std::string& strOut) const
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

void CClientSocket::UpdateAddress(int nIp, int nPort)
{
	if (m_nIp != nIp || m_nPort != nPort)
	{
		m_nPort = nPort;
		m_nIp = nIp;
		
	}
}



//私有静态成员必须初始化 
CClientSocket* CClientSocket::m_pinstance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;
CClientSocket::CClientSocket():
	m_nIp(INADDR_ANY),
	m_nPort(0),
	m_client_sock(INVALID_SOCKET),
	m_bAutoClose(true)
{
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

CClientSocket::CClientSocket(const CClientSocket& ss)
{
	if (&ss != this)
	{
		m_client_sock = ss.m_client_sock;
		m_nIp = ss.m_nIp;
		m_nPort = ss.m_nPort;
		m_bAutoClose = ss.m_bAutoClose;
		
	}
	
}

CClientSocket::~CClientSocket()
{
	closesocket(m_client_sock);
	m_client_sock = INVALID_SOCKET;
	WSACleanup();
	TRACE("CClientSocket over\r\n");

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

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	return;
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pbuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_client_sock != INVALID_SOCKET)
	{
		if (m_listSend.size() >  0)
		{
			
			auto head = m_listSend.front();
			if (Send(head) < 0)
			{
				TRACE("发送失败\r\n");
				continue;
			}

			auto pr = m_mapAck.find(head.hEvent);
			do
			{
				//一个发送为什么只对应了一个接受呢?,对于文件传送来说显然不对
				int length = recv(m_client_sock, pbuffer, BUFFER_SIZE - index, 0);
				if (length > 0 || index > 0)
				{
					index += length;
					size_t size = (size_t)index;
					CPacket pack((BYTE*)pbuffer, size);
					if (size > 0)
					{
						//通知对应的事件
						pack.hEvent = head.hEvent;
						pr->second.push_back(pack);
						if (m_mapAutoClose.find(head.hEvent)->second)
						{
							SetEvent(head.hEvent);
						}
						
						memmove(pbuffer, pbuffer + size, index - size);
						index -= size;
					}

				}
				else if (length <= 0 && index <= 0)
				{
					closesocket(m_client_sock);
					SetEvent(head.hEvent);//等到服务器关闭后在通知事情完成
				}
			} while (!m_mapAutoClose.find(head.hEvent)->second);
			m_listSend.pop_front();
			InitSocket();
		}
	}
	closesocket(m_client_sock);
}

bool CClientSocket::InitSocket()
{
	if (m_client_sock != INVALID_SOCKET)
	{
		closeSocket();
	}
	m_client_sock = socket(PF_INET, SOCK_STREAM, 0);
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(sockaddr_in));
	serv_adr.sin_addr.s_addr = htonl(m_nIp);
	serv_adr.sin_family = AF_INET;
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("IP地址格式不正确");
		perror("ip地址不对");
		return false;
	}
	serv_adr.sin_port = htons(m_nPort);
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
		if (((int)len <= 0) && ((int)index <= 0))
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
			memmove(buffer, buffer + len, index - len);
			index -= len;
			return packet.sCmd;
		}
		//todo：出现了构造失败的情况会直接抛弃所有的缓存，这样不行吧？

	}
	return -1;
}

bool CClientSocket::Send(const char* pData, size_t nize)
{
	if (m_client_sock == INVALID_SOCKET)
	{
		return false;
	}
	return send(m_client_sock, pData, nize, 0) > 0;
}

bool CClientSocket::Send(const CPacket& pack)
{
	if (m_client_sock == INVALID_SOCKET)
	{
		return false;
	}
	std::string strOut;
	pack.Data(strOut);
	auto ret = send(m_client_sock, strOut.c_str(), strOut.length(), 0);
	return ret;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPack, bool isAutoClosed)
{
	if (m_client_sock == INVALID_SOCKET)
	{
		std::thread thread1(&CClientSocket::threadEntry, this);
		thread1.detach();
	}
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(pack.hEvent, lstPack));
	m_mapAutoClose.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	m_listSend.push_back(pack);
	//无限等待,发送线程的处理结果
	WaitForSingleObject(pack.hEvent, INFINITE);
	auto itor =  m_mapAck.find(pack.hEvent);
	if (itor != m_mapAck.end())
	{
		m_mapAck.erase(itor);
		return true;
	}
	return false;
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








