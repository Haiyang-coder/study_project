#pragma once
#include"pch.h"
#include"framework.h"
#include<string>
#include<vector>
#include<list>
#include<map>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket();
	CPacket(const BYTE*, size_t& nSize);
	CPacket(const CPacket& packet);
	CPacket& operator=(const CPacket& packet);
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize, HANDLE hEvent  = INVALID_HANDLE_VALUE);
	~CPacket();
public:
	int Size();
	const char* const Data(std::string& strOut) const;
public:
	HANDLE hEvent;
	WORD sHead = 0;//����ͷ
	DWORD nLength = 0;//���ݳ��ȣ��ӿ������У��ĳ��ȣ�
	WORD sCmd = 0;//��������
	std::string strData = "";//����
	WORD sSum = 0;//У��(ֻУ�����ݲ���)
	//std::string strOut;	//�������Ĕ���



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

typedef struct file_info
{
	file_info()
	{
		IsInvalid = 0;
		ISDirectory = -1;
		memset(szFileName, 0, sizeof(szFileName));
		HaveNext = TRUE;
	}
	BOOL IsInvalid;//�Ƿ���Ч
	char szFileName[256];
	BOOL ISDirectory;//�Ƿ�ΪĿ¼�� 0�� 1��
	BOOL HaveNext;//�Ƿ��� 0�� 1��

}FILEINFO, * PFILEINFO;


class CClientSocket
{
private:

	CClientSocket();
	CClientSocket(const CClientSocket&);
	~CClientSocket();

	CClientSocket& operator=(const CClientSocket&) {};
	BOOL InitSockEnv();

	static void threadEntry(void* arg);
	void threadFunc();




public:
	static CClientSocket* getInstance();
	static void releaseInstance();
	bool InitSocket();
	int DealCommand();
	bool Send(const char* pData, size_t nize);
	bool Send(const CPacket& pack);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);
	const CPacket& Getpack();
	void closeSocket();
	int UpdateAddress(int, int);



private:
	std::list<CPacket&> m_listSend;
	std::map<HANDLE, std::list<CPacket> >m_mapAck; 
	int m_nIp;
	int m_nPort;
	std::vector<char> m_buffer;
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





