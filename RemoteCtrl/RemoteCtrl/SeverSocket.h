#pragma once
#include"pch.h"
#include"framework.h"
#include"RemteServerTool.h"
#include<list>
#include"Packet.h"


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

typedef void(* SOCKE_CALLBACK)(void* arg, int status, std::list<CPacket>&, CPacket& packetIn);
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
	int RunFunc(SOCKE_CALLBACK callbackFunc, void* arg, short port = 9527);
protected:
	static void releaseInstance();
	bool InitSocket(short port = 9527);
	
	bool AcceptClient();
	int DealCommand();
	bool Send(const char* pData, size_t nize);
	bool Send(CPacket& pack);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);
	const CPacket& Getpack();
	void CloseSocket();
	


private:
	SOCKE_CALLBACK m_callback;
	void* m_arg;
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





 