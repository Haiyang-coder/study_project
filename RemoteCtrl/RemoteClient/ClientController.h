#pragma once
#include<thread>


#include"ClientSocket.h"
#include"StatusDlg.h"
#include"WatchDialog.h"
#include"RemoteClientDlg.h"
#include<map>
#include"resource.h"
#include"RemteClientTool.h"

#define WM_SEND_PACKET (WM_USER + 1) //���Ͱ�����
#define WM_SEND_DATA (WM_USER + 2)	//��������
#define WM_SHOW_STATUS (WM_USER + 3)	//״̬չʾ
#define WM_SHOW_WATCH (WM_USER + 4)	//���չʾ
#define WM_SEND_MESSAGE (WM_USER + 0x1000)	//�Զ�����Ϣ����


class CClientController
{
public:
	
	//����ģʽ
	static CClientController* getInstance();
	//����	
	int Invoke(CWnd*& pMainWnd);
	//��ʼ��
	int InitController();
	//������Ϣ
	LRESULT SendMessage(MSG msag);
	void UpdateAddress(int ip, int port);
	int DealCommand();
	void closeSocket();
	bool SendPacket(const CPacket& packet);
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t length = 0);
	int GetImage(CImage& image);
	int DownLoadFile(const CString& strPath);
	void StartWatchScreen();

protected:
	CClientController();
	~CClientController();
	static void releaseInstance();
private:
	void threadFunc();
	void threadDownLoadFile();
	void threadWatchDlg();

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
	HANDLE m_threadDownLoadHandle;
	HANDLE m_threadWatchHandle;
	bool m_isCLosed = true;//����Զ�̼����ʾ����
	DWORD m_nthreadId;
	FILE* m_pfile;
	//�����ļ���Զ��·��
	CString m_strFileRemotePath;
	//�����ļ��ı��ر���·��
	CString m_strFileLocalPath;
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

