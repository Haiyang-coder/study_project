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
#define WM_SEND_PACKET_ACK (WM_USER + 2)	//发送包数据应答
#endif // !WM_SEND_PACKET_ACK
#ifndef WM_SEND_PACKET
#define WM_SEND_PACKET (WM_USER + 1) //发送数据包的消息
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
	
	WORD sHead = 0;//数据头
	DWORD nLength = 0;//数据长度（从控制命令到校验的长度）
	WORD sCmd = 0;//控制命令
	std::string strData = "";//数据
	WORD sSum = 0;//校验(只校验数据部分)



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
	WORD nAction;//点击 移动 双击
	WORD nButton;//左键 右键 中建
	POINT ptXY; //坐标
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
	BOOL IsInvalid;//是否有效
	char szFileName[256];
	BOOL ISDirectory;//是否为目录， 0否 1是
	BOOL HaveNext;//是否还有 0无 1有

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
		CSM_AUTOCLOSE = 1 //自动关闭模式
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





