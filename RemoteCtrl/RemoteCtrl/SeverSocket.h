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
	WORD nAction;//点击 移动 双击
	WORD nButton;//左键 右键 中建
	POINT ptXY; //坐标
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
	BOOL IsInvalid;//是否有效
	char szFileName[256];
	BOOL ISDirectory;//是否为目录， 0否 1是
	BOOL HaveNext;//是否还有 0无 1有

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





 