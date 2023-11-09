#include "pch.h"
#include "ClientController.h"
CClientController* CClientController::m_pInstance = nullptr;
CClientController::CHelper CClientController::m_helper;
std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;

CClientController::CClientController()
	:m_StatusDlg(&m_RemoteDlg),
	m_WatchDlg(&m_RemoteDlg)
{
	

	//m_thread =  (HANDLE)_beginthread(NULL, 0, this);
}
CClientController::~CClientController()
{
	m_nthreadId = -1;
	m_threadHandle = INVALID_HANDLE_VALUE;
}
void CClientController::releaseInstance()
{
	if (m_pInstance != NULL)
	{
		CClientController* tem = m_pInstance;
		m_pInstance = nullptr;
		delete tem;

	}
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MsgInfo* pmsg = (MsgInfo*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;

			auto itor  = m_mapFunc.find(msg.message);
			if (itor != m_mapFunc.end())
			{
				auto func = itor->second;
				pmsg->result = (this->*func)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else
			{
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			auto itor = m_mapFunc.find(msg.message);
			if (itor != m_mapFunc.end())
			{
				auto func = itor->second;
				(this->*func)(msg.message, msg.wParam, msg.lParam);
			}
		}
		
		
		
	}
	
}
LRESULT CClientController::OnSendPack(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	CPacket* pPack = (CPacket*)wParam;
	return pclient->Send(*pPack);
}
LRESULT CClientController::OnSendData(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	char* pPack = (char*)wParam;
	return pclient->Send(pPack, (int)lParam);
}
LRESULT CClientController::OnShowStatus(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_StatusDlg.ShowWindow(SW_SHOW);
}
LRESULT CClientController::OnShowWatch(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_WatchDlg.DoModal();
}



CClientController* CClientController::getInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CClientController();
		struct
		{
			UINT nMsg;
			MSGFUNC func;
		}data[] = 
			{
				{WM_SEND_PACKET, &CClientController::OnSendPack},
				{WM_SEND_DATA, &CClientController::OnSendData},
				{WM_SHOW_STATUS, &CClientController::OnShowStatus},
				{WM_SHOW_WATCH, &CClientController::OnShowWatch},
				{(UINT)-1, NULL}
			};

		for (int i = 0; data[i].func != nullptr; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(data[i].nMsg, data[i].func));
		}

	}
	return nullptr;
}


int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_RemoteDlg;
	return m_RemoteDlg.DoModal();
}

int CClientController::InitController()
{
	//这里我尝试用c++的线程实现
	std::thread threadController = std::thread(&CClientController::threadFunc, this);
	threadController.detach();
	m_threadHandle = threadController.native_handle();
	m_nthreadId = GetThreadId(m_threadHandle);

	m_StatusDlg.Create(dlg_status, &m_RemoteDlg);
	return 0;
}

LRESULT CClientController::SendMessage(MSG msg)
{
	//创建了一个事件，目的是等线程完成了任务，可以通过事件反馈给事件的发送者
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == INVALID_HANDLE_VALUE)
	{
		return -2;
	}
	//创建了一个消息结构体，包括了给线程的参数和执行后的结果（线程填写的）
	MsgInfo info(msg);
	PostThreadMessage(m_nthreadId, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	//通过事件通知消息发送者完成了处理
	WaitForSingleObject(hEvent, -1);
	return info.result;
}

void CClientController::UpdateAddress(int ip, int port)
{
	CClientSocket::getInstance()->UpdateAddress(ip, port);
}

int CClientController::DealCommand()
{
	return CClientSocket::getInstance()->DealCommand();
}

void CClientController::closeSocket()
{
	CClientSocket::getInstance()->closeSocket();
}

bool CClientController::SendPacket(const CPacket& packet)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	if (pclient->InitSocket() == false)
	{
		return false;
	}
	pclient->Send(packet);
	return false;
}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t length)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	CPacket pack(nCmd, pData, length);
	if (pclient->InitSocket() == false)
	{
		return false;
	}
	pclient->Send(pack);
	
	int iRet = pclient->DealCommand();
	if (iRet == -1)
	{
		AfxMessageBox("处理命令失败");
		TRACE("sCmd : %d\r\n", iRet);
		pclient->closeSocket();
		return -1;
	}
	TRACE("sCmd : %d\r\n", iRet);
	if (bAutoClose)
	{
		pclient->closeSocket();
	}
	return 0;
}

int CClientController::GetImage(CImage& image)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	//BYTE* pdata = (BYTE*)pclient->Getpack().strData.c_str();
	return CRemteClientTool::Byte2Image(image, pclient->Getpack().strData);
	
}
