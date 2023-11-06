#pragma once
#include"pch.h"
#include"framework.h"
#include"RemteServerTool.h"
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
}MOUSEEV, *PMOUSEEV;

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

class CSeverSocket
{
private: 
	
	CSeverSocket();
	CSeverSocket(const CSeverSocket&) {};
	~CSeverSocket();

	CSeverSocket& operator=(const CSeverSocket&) {};
	BOOL InitSockEnv();

	
	

public:
	static CSeverSocket* getInstance();
	static void releaseInstance();
	bool InitSocket();
	bool AcceptClient();
	int DealCommand();
	bool Send(const char* pData, size_t nize);
	bool Send(CPacket& pack);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);
	const CPacket& Getpack();
	void CloseSocket();
	


private:
	 static CSeverSocket* m_pinstance; 
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
	 SOCKET m_serv_sock;
	 SOCKET m_client;
	 CPacket m_packet;
};





 