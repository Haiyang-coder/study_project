#pragma once
#include"pch.h"
#include"framework.h"
#include<string>
#include<vector>
#include<list>
#include<map>
#include<thread>
#include<mutex>

#pragma pack(push)
#pragma pack(1)
#ifndef WM_SEND_PACKET_ACK
#define WM_SEND_PACKET_ACK (WM_USER + 2)	//���Ͱ�����Ӧ��
#endif // !WM_SEND_PACKET_ACK
#ifndef WM_SEND_PACKET
#define WM_SEND_PACKET (WM_USER + 1) //�������ݰ�����Ϣ
#endif // !WM_SEND_PACKET

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
	
	WORD sHead = 0;//����ͷ
	DWORD nLength = 0;//���ݳ��ȣ��ӿ������У��ĳ��ȣ�
	WORD sCmd = 0;//��������
	std::string strData = "";//����
	WORD sSum = 0;//У��(ֻУ�����ݲ���)



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
	bool m_bAutoClose;
	CClientSocket();
	CClientSocket(const CClientSocket&);
	~CClientSocket();

	CClientSocket& operator=(const CClientSocket&) {};
	BOOL InitSockEnv();

	static void threadEntry(void* arg);
	void threadFunc();
	void threadFuncEx();




public:
	static CClientSocket* getInstance();
	static void releaseInstance();
	bool InitSocket();
	bool InitSocketThread();
	int DealCommand();
	
	bool SendPacket(HWND hWnd, const CPacket& pack,  bool isAutoClosed = true, WPARAM param = 0);
	bool GetFilePath(std::string& strPath);
	bool GetMouseEvent(MOUSEEV& mouse);
	const CPacket& Getpack();
	void closeSocket();
	void UpdateAddress(int, int);

protected:
	bool Send(const char* pData, size_t nize);
	bool Send(const CPacket& pack);

	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	enum ModelEnum
	{
		CSM_AUTOCLOSE = 1 //�Զ��ر�ģʽ
	};
	typedef struct PacketData
	{
		WPARAM wParam;
		std::string strData;
		UINT nMode;
		PacketData(const char* pData, size_t len, UINT mode, WPARAM nParam = 0)
		{
			strData.resize(len);
			memcpy((char*)strData.c_str(), pData, len);
			nMode = mode;
			wParam = nParam;
		}
		PacketData(const PacketData& pack)
		{
			if (&pack != this)
			{
				strData = pack.strData;
				nMode = pack.nMode;
				wParam = pack.wParam;
			}
			
		}

		PacketData& operator=(const PacketData& pack)
		{
			if (&pack != this)
			{
				strData = pack.strData;
				nMode = pack.nMode;
				wParam = pack.wParam;
			}
			return *this;
		}
	} PACKET_DATA;
	

	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapMsgFuction;
	HANDLE m_threadSocket;
	DWORD m_threadSocketID;
	std::mutex m_lock;
	std::map<HANDLE, bool >m_mapAutoClose;
	std::list<CPacket> m_listSend;
	std::map<HANDLE, std::list<CPacket>& >m_mapAck; 
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





