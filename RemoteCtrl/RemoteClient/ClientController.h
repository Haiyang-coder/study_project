#pragma once
#include<thread>


#include"ClientSocket.h"
#include"StatusDlg.h"
#include"WatchDialog.h"
#include"RemoteClientDlg.h"
#include<map>
#include"resource.h"

#define WM_SEND_PACKET (WM_USER + 1) //发送包数据
#define WM_SEND_DATA (WM_USER + 2)	//发送数据
#define WM_SHOW_STATUS (WM_USER + 3)	//状态展示
#define WM_SHOW_WATCH (WM_USER + 4)	//监控展示
#define WM_SEND_MESSAGE (WM_USER + 0x1000)	//自定义消息处理


class CClientController
{
public:
	
	//单例模式
	static CClientController* getInstance();
	//启动	
	int Invoke(CWnd*& pMainWnd);
	//初始化
	int InitController();
	//发送消息
	LRESULT SendMessage(MSG msag);


protected:
	CClientController();
	~CClientController();
	static void releaseInstance();
private:
	void threadFunc();
private:
	LRESULT OnSendPack(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatch(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo& operator=(const MsgInfo& m)
		{
			if (&m != this)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
				//hEvent = m.hEvent;
			}
		}
		MsgInfo(MSG m)
		{
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
			//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		MsgInfo()
		{
			result = 0;
			memset(&msg, 0, sizeof(MSG));
			//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
			//hEvent = m.hEvent;
		}
		void clear()
		{
			//CloseHandle(hEvent);
			//hEvent = INVALID_HANDLE_VALUE;
			result = 0;
		}
	};
	typedef LRESULT(CClientController::* MSGFUNC)(UINT msg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	HANDLE m_threadHandle;
	DWORD m_nthreadId;

	static CClientController* m_pInstance;
	CRemoteClientDlg m_RemoteDlg;
	CWatchDialog m_WatchDlg;
	CStatusDlg m_StatusDlg;

	class CHelper
	{
	public:
		CHelper()
		{
			CClientController::getInstance();
		}
		~CHelper()
		{
			CClientController::releaseInstance();
		}

	};
	static CHelper m_helper;
};

