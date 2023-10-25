#pragma once
#include"pch.h"
#include"framework.h"
#include<string>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket();
	CPacket(const BYTE*, size_t& nSize);
	CPacket(const CPacket& packet);
	CPacket& operator=(const CPacket& packet);
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize);
	~CPacket();

	int Size();
	const char* Data();
public:
	WORD sHead = 0;//����ͷ
	DWORD nLength = 0;//���ݳ��ȣ��ӿ������У��ĳ��ȣ�
	WORD sCmd = 0;//��������
	std::string strData = "";//����
	WORD sSum = 0;//У��(ֻУ�����ݲ���)
	std::string strOut;	//�������Ĕ���

private:

};
#pragma pack(pop)

typedef struct MOUSEEVENT
{
	MOUSEEVENT()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//��� �ƶ� ˫��
	WORD nButton;//��� �Ҽ� �н�
	POINT ptXY; //����
}MOUSEEV, * PMOUSEEV;

class CClientSocket
{
private:

	CClientSocket();
	CClientSocket(const CClientSocket&) {};
	~CClientSocket();

	CClientSocket& operator=(const CClientSocket&) {};
	BOOL InitSockEnv();




public:
	static CClientSocket* getInstance();
	static void releaseInstance();
	bool InitSocket(const std::string strIPAddress);
	int DealCommand();
	bool Send(const char* pData, size_t nize);
	bool Send(CPacket& pack);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);



private:
	static CClientSocket* m_pinstance;
	class CHelper
	{
	public:
		CHelper()
		{
			getInstance();
		}
		~CHelper()
		{
			releaseInstance();
		}

	};
	static CHelper m_helper;
	SOCKET m_client_sock;
	CPacket m_packet;
};





