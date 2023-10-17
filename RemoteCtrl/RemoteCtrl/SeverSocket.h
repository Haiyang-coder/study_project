#pragma once
#include"pch.h"
#include"framework.h"


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
};

extern CSeverSocket server;

 