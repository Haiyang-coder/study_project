#include "pch.h"
#include "ThreadPool.h"
#include<atomic>


CThreadRemote::CThreadRemote()
{
	m_hThread = NULL;
}

CThreadRemote::~CThreadRemote()
{
	Stop();
}

CThreadRemote::CThreadRemote(const CThreadRemote& s)
{
	if (this == &s)
	{
		return;
	}
	else
	{
		m_hThread = s.m_hThread;
	}

}


bool CThreadRemote::boolIsValid()
{
	if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
}

bool CThreadRemote::Start()
{
	m_hThread = (HANDLE)_beginthread(TheadEntry, 0, this);
	if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
	{
		m_bStatus = false;
	}
	else
	{
		m_bStatus = true;
	}

	return m_bStatus;
}

bool CThreadRemote::Stop()
{
	if (m_bStatus == false)
	{
		return true;
	}
	m_bStatus = false;
	bool ret = WaitForSingleObject(m_hThread, INFINITE) == WAIT_OBJECT_0;
	UpdateWorker();
	
	return ret;
}

void CThreadRemote::UpdateWorker(const CThreadWorker& woker)
{
	if (!woker.IsValid())
	{
		m_pWorker.store(NULL);
		return;
	}
		
	if (m_pWorker.load() != NULL)
	{
		CThreadWorker* pWorker = m_pWorker.load();
		m_pWorker.store(NULL);
		delete pWorker;
	}
	m_pWorker.store(new CThreadWorker(woker));
	
}

bool CThreadRemote::Isdle()
{//true���У� false:�Ѿ������˹���
	return !m_pWorker.load()->IsValid();
}

void CThreadRemote::TheadEntry(void* arg)
{
	CThreadRemote* thiz = (CThreadRemote*)arg;
	if (thiz)
	{
		thiz->ThreadWorker();
	}
	_endthread();
}

void CThreadRemote::ThreadWorker()
{
	while (m_bStatus)
	{
		CThreadWorker worker = *m_pWorker.load();
		if (worker.IsValid())
		{
			int ret = worker();
			if (ret != 0)
			{
				TRACE("warning\r\n");
			}
			if (ret < 0)
			{
				m_pWorker.store(NULL);

			}
		}
		else
		{
			Sleep(1);
		}
		
	}
}


CTheadPool::CTheadPool(size_t size)
{
	m_vecThead.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		m_vecThead[i] = new CThreadRemote();
	}
}

CTheadPool::CTheadPool()
{
}

CTheadPool::~CTheadPool()
{
	Stop();
	m_vecThead.clear();
}

bool CTheadPool::Invoke()
{	
	int ret = false;
	for (size_t i = 0; i < m_vecThead.size(); i++)
	{
		if (m_vecThead[i]->Start() == false) 
		{
			ret = false;
			break;
		}
	}
	if (ret == false)
	{
		for (size_t i = 0; i < m_vecThead.size(); i++)
		{
			m_vecThead[i]->Stop();
		}
	}
	return ret;
}

void CTheadPool::Stop()
{
	for (size_t i = 0; i < m_vecThead.size(); i++)
	{
		m_vecThead[i]->Stop();
	}
}
//����-1��ʾ����ʧ�� ���ڵ���0 ��ʾ��n���̷߳��������������
int CTheadPool::DispatchWorker(const CThreadWorker& woker)
{
	m_lock.lock();
	int index = -1;
	for (size_t i = 0; i < m_vecThead.size(); i++)
	{
		if (m_vecThead[i]->Isdle())
		{
			index = i;
			m_vecThead[i]->UpdateWorker(woker);
			break;
		}
	}
	m_lock.unlock();
	return index;
}

bool CTheadPool::CheckThreadValid(int index)
{
	if (index < m_vecThead.size())
	{
		return true;
	}
	else
	{
		return false;
	}
	
}
