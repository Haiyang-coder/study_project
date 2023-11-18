#include "pch.h"
#include "ClientController.h"
CClientController* CClientController::m_pInstance = nullptr;
CClientController::CHelper CClientController::m_helper;
std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
bool CClientController::thisEnd = false;
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
	m_threadWatchHandle = INVALID_HANDLE_VALUE;
	m_threadDownLoadHandle = INVALID_HANDLE_VALUE;
	TRACE("CClientController over \r\n");
}
void CClientController::releaseInstance()
{
	if (m_pInstance != NULL)
	{
		thisEnd = true;
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
	return;
}
unsigned _stdcall CClientController::threadFuncEntry(void* arg)
{
	CClientController* pclientCtr = (CClientController*)arg;
	pclientCtr->threadFunc();
	_endthreadex(0);
	return 0;
}
void CClientController::threadDownLoadFile()
{
	std::list<CPacket> plstPack;
	CClientSocket* pclientSocket = CClientSocket::getInstance();
	int ret = SendCommandPacket(m_RemoteDlg.GetSafeHwnd(), 4, false, (BYTE*)(LPCSTR)m_strFileRemotePath, m_strFileRemotePath.GetLength(),(WPARAM)m_pfile);
	if (ret < 0)
	{
		AfxMessageBox("ִ����������ʧ����");
		TRACE("����ʧ�ܣ�%d\r\n", ret);
		fclose(m_pfile);
		m_StatusDlg.ShowWindow(SW_HIDE);
		m_RemoteDlg.EndWaitCursor();
		return;
	}
	long long nCount = 0;
	long long nLenth = *(long long*)pclientSocket->Getpack().strData.c_str();
	while (nCount < nLenth)
	{
		//��ʼ���շ���˷��������ļ���Ϣ
		ret = pclientSocket->DealCommand();
		if (ret < 0)
		{
			AfxMessageBox("�ļ�����ʧ�ܣ�������");
			TRACE("�ļ�����ʧ�ܣ�%d\r\n", ret);
			break;
		}
		fwrite(pclientSocket->Getpack().strData.c_str(), 1, pclientSocket->Getpack().strData.size(), m_pfile);
		nCount += pclientSocket->Getpack().strData.size();

	}
	fclose(m_pfile);
	pclientSocket->closeSocket();
	m_StatusDlg.ShowWindow(SW_HIDE);
	m_RemoteDlg.EndWaitCursor();
	m_RemoteDlg.MessageBox(_T("�������"), _T("���"));
	return;
	
}
void CClientController::threadWatchDlg()
{
	CClientController* pclient = CClientController::getInstance();
	while (!m_isCLosed)
	{
		if (m_WatchDlg.GetIsFull() == false)
		{
			std::list<CPacket> listPack;
			int ret = SendCommandPacket(m_WatchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			//todo:�����Ϣ��Ӧ����WM_send_packet_ack
			//���Ʒ���Ƶ��
			if (ret == 6)
			{
				
				if (CRemteClientTool::Byte2Image(m_WatchDlg.getImage(), listPack.front().strData) == 0)
				{
					m_WatchDlg.SetImageStatus(true);
				}
				else
				{
					TRACE("��ȡͼƬʧ�� ret = %d\r\n", ret);
				}

			}
			else
			{
				Sleep(10);
			}
		}
		else {
			Sleep(1);
		}
	}
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
				{WM_SHOW_STATUS, &CClientController::OnShowStatus},
				{WM_SHOW_WATCH, &CClientController::OnShowWatch},
				{(UINT)-1, NULL}
			};

		for (int i = 0; data[i].func != nullptr; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(data[i].nMsg, data[i].func));
		}

	}
	return m_pInstance;
}


int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_RemoteDlg;
	return m_RemoteDlg.DoModal();
}

int CClientController::InitController()
{
	//�����ҳ�����c++���߳�ʵ��
	//std::thread threadController = std::thread(&CClientController::threadFuncEntry, this);
	//m_threadHandle = threadController.native_handle();
	//m_nthreadId = GetThreadId(m_threadHandle);

	//m_StatusDlg.Create(dlg_status, &m_RemoteDlg);
	//threadController.detach();
	m_threadHandle = (HANDLE)_beginthreadex(NULL, 0, &CClientController::threadFuncEntry, this, 0, &m_nthreadId);
	m_StatusDlg.Create(dlg_status, &m_RemoteDlg);
	return 0;
}

LRESULT CClientController::SendMessage(MSG msg)
{
	//������һ���¼���Ŀ���ǵ��߳���������񣬿���ͨ���¼��������¼��ķ�����
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == INVALID_HANDLE_VALUE)
	{
		return -2;
	}
	//������һ����Ϣ�ṹ�壬�����˸��̵߳Ĳ�����ִ�к�Ľ�����߳���д�ģ�
	MsgInfo info(msg);
	PostThreadMessage(m_nthreadId, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	//ͨ���¼�֪ͨ��Ϣ����������˴���
	WaitForSingleObject(hEvent, INFINITY);
	CloseHandle(hEvent);
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


bool CClientController::SendCommandPacket(HWND hwnd, int nCmd, bool bAutoClose, BYTE* pData, size_t length, WPARAM param)
{
	CClientSocket* pclient = CClientSocket::getInstance();
	return pclient->SendPacket(hwnd, CPacket(nCmd, pData, length), bAutoClose, param);
}


int CClientController::DownLoadFile(const CString& strPath)
{
	CFileDialog filedlg(FALSE, NULL, strPath, OFN_OVERWRITEPROMPT, NULL, &m_RemoteDlg);
	if (filedlg.DoModal() == IDOK)//ģ̬
	{
		m_strFileRemotePath = strPath;
		m_strFileLocalPath = filedlg.GetPathName();//��ȡ���ش洢·��
		m_pfile = fopen(m_strFileLocalPath, "wb+");
		if (m_pfile == NULL)
		{
			AfxMessageBox("�޷��򿪱���ָ�����ļ�");
			m_StatusDlg.ShowWindow(SW_HIDE);
			m_RemoteDlg.EndWaitCursor();
			return -1;
		}
		int ret = SendCommandPacket(m_RemoteDlg.GetSafeHwnd(), 4, false, (BYTE*)(LPCSTR)m_strFileRemotePath, m_strFileRemotePath.GetLength(), (WPARAM)m_pfile);
		/*std::thread threadDownLoadFile(&CClientController::threadDownLoadFile, this);
		Sleep(50);
		threadDownLoadFile.detach();*/
		//m_threadDownLoadHandle = threadDownLoadFile.native_handle();
		m_RemoteDlg.BeginWaitCursor();
		m_StatusDlg.ShowWindow(SW_SHOW);
		m_StatusDlg.m_info.SetWindowTextA("��������ִ����");
		m_StatusDlg.CenterWindow(&m_RemoteDlg);
		m_StatusDlg.SetActiveWindow();
	}
	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isCLosed = false;
	std::thread threadWatch(&CClientController::threadWatchDlg, this);
	m_WatchDlg.DoModal();
	threadWatch.detach();
	
}

void CClientController::SetScreenClose(bool flag)
{
	m_isCLosed = flag;
}

void CClientController::DownLoadEnd()
{
	m_StatusDlg.ShowWindow(SW_HIDE);
	m_RemoteDlg.EndWaitCursor();
	m_RemoteDlg.MessageBox("�������", "���");
}
