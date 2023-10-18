#include "pch.h"
#include "SeverSocket.h"




CPacket::CPacket()
{
}

CPacket::CPacket(const BYTE* pData, size_t& nSize)
{
	size_t i = 0;
	for (i = 0; i < nSize; i++)
	{
		//�������ͷ
		if (*(WORD*)(pData + i) == 0xFEFF)
		{
			sHead = *(WORD*)(pData + i);
			i += 2;
			break;
		}
	}
	//������ݰ���ȫ���������ݰ�û����ȫ���յ�
	if (i + 4 + 2 + 2 > nSize)
	{
		nSize = 0;
		return;
	}

	nLength = *(DWORD*)(pData + i);
	i += 4;
	if (nLength + i > nSize)
	{
		//����û�н�������
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
	nSize = 0;
	return;
}

CPacket::CPacket(const CPacket& packet)
{
	sHead = packet.sHead;//����ͷ
	nLength = packet.nLength;//���ݳ��ȣ��ӿ������У��ĳ��ȣ�
	sCmd = packet.sCmd;//��������
	strData = packet.strData;//����
	sSum = packet.sSum;//У��
}

CPacket& CPacket::operator=(const CPacket& packet)
{
	if (this == &packet)
	{
		return *this;
	}
	sHead = packet.sHead;//����ͷ
	nLength = packet.nLength;//���ݳ��ȣ��ӿ������У��ĳ��ȣ�
	sCmd = packet.sCmd;//��������
	strData = packet.strData;//����
	sSum = packet.sSum;//У��
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
	}else {
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

const char* CPacket::Data()
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



//˽�о�̬��Ա�����ʼ�� 
CSeverSocket* CSeverSocket::m_pinstance = nullptr;
CSeverSocket::CHelper CSeverSocket::m_helper;
CSeverSocket::CSeverSocket()
{
	m_client = INVALID_SOCKET;
	m_serv_sock = INVALID_SOCKET;
	m_pinstance = NULL;
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
	//��ʼ���׽���
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
	//��
	if (bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
	{
		return false;
	}
	//todoû�н�����֤
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
#define BUFFER_SIZE 4096
int CSeverSocket::DealCommand()
{
	if (m_client == -1)
	{
		return -1;
	}
	//char buffer[1024];
	char* buffer = new char[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	size_t index = 0;
	while (true)
	{
		//Ŀǰû�д���ճ�����⣬Ĭ��ÿ�ν���ֻ��һ�����ݰ�
		size_t len  = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
		if (len <= 0)
		{
			return -1;
		}
		index += len;
		//len = index;
		CPacket packet((BYTE*)buffer, len);
		m_packet = packet;
		if (len > 0)
		{
			memmove(buffer, buffer + len, BUFFER_SIZE - len);
			index -= len;
			return packet.sCmd;
		}
		
	}
	
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
		return false;
	}
	return  send(m_client, pack.Data(), pack.Size(), 0);
}

bool CSeverSocket::GetFilePath(std::string& strPath)
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









