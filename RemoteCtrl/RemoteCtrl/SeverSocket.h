#pragma once
#include"pch.h"
#include"framework.h"
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
	WORD sSum = 0;//У��
	std::string strOut;	//�������Ĕ���

private:

};
#pragma pack(pop)


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





 