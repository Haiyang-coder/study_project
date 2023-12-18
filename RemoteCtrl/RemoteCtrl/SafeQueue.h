#pragma once
#include<list>
#include<string>
#include<conio.h>
#include<thread>
#include<mutex>
#include<atomic>
#include"ThreadPool.h"

typedef void* (*listcallBack)(void*);

template<class T>
class CSafeQueue
{
public:
	enum OPERATOR
	{
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};

	typedef struct IocpParam
	{
		OPERATOR nOperator;
		T Data;
		listcallBack lstCall;
		HANDLE hEvent;//pop操作需要的
		size_t size;
		IocpParam(OPERATOR op, const T& sData,  HANDLE Event = NULL, listcallBack cb = NULL)
		{
			nOperator = op;
			Data = sData;
			lstCall = cb;
			hEvent = Event;
		}
		IocpParam(OPERATOR op, HANDLE Event = NULL )
		{
			nOperator = op;
			Data = T();
			hEvent = Event;
			
		}
		IocpParam()
		{
			nOperator = EQNone;
		}

	}PPARAM;//用于投递信息的结构体
	//利用iocp实现的线程安全的队列
public:
	CSafeQueue();
	virtual ~CSafeQueue();
	bool PushBack(const T& data);
	virtual bool PopFront(T& data);
	size_t Size();
	void Clear();
protected:
	void threadMain(HANDLE arg);
	virtual void DealParam(PPARAM* pParam);
protected:
	std::list<T> m_listData;
	HANDLE m_hCompleoetionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock; //队列正在析构


};



template<class T>
class CSafeSendQueue:public CSafeQueue<T>,public CThreadFuncBase
{
public:
	typedef int (CThreadFuncBase::* SendCallBack)(T& data);
	CSafeSendQueue(CThreadFuncBase* obj, SendCallBack callback);
	virtual ~CSafeSendQueue();
	
protected:
	virtual bool PopFront(T& data) 
	{
		return false;
	}
	int threadTick()
	{
		if (WaitForSingleObject(CSafeQueue<T>::m_hThread, 0) != WAIT_TIMEOUT)
		{
			return 0;
		}
		if (CSafeQueue<T>::m_listData.size() > 0)
		{
			Popfront();
		}
		Sleep(1);
		return 0;
	}
	bool Popfront()
	{
		typename CSafeQueue<T>::IocpParam* Param = new typename CSafeQueue<T>::IocpParam(CSafeQueue<T>::EQPop, T());
		if (CSafeQueue<T>::m_lock)
		{
			delete Param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CSafeQueue<T>::m_hCompleoetionPort, sizeof(typename CSafeQueue<T>::PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false)
		{
			delete Param;
			return ret;
		}
		return ret;
	}
	virtual void DealParam(typename CSafeQueue<T>::PPARAM* pParam)
	{
		switch (pParam->nOperator)
		{
		case CSafeQueue<T>::EQPush:
		{
			CSafeQueue<T>::m_listData.push_back(pParam->Data);
			delete pParam;

		}
		break;
		case CSafeQueue<T>::EQPop:
		{
			if (CSafeQueue<T>::m_listData.size() > 0)
			{
				pParam->Data = CSafeQueue<T>::m_listData.front();
				if ((m_pBase->*m_callBack)(pParam->Data) == 0)
				{
					CSafeQueue<T>::m_listData.pop_front();
				}
				
			}
			delete pParam;
		}
		break;
		case CSafeQueue<T>::EQClear:
		{
			CSafeQueue<T>::m_listData.clear();
			delete pParam;
		}
		break;
		case CSafeQueue<T>::EQSize:
		{
			pParam->size = CSafeQueue<T>::m_listData.size();
			if (pParam->hEvent != NULL)
			{
				SetEvent(pParam->hEvent);
			}
		}
		break;
		default:
			TRACE("unkown\r\n");
			break;
		}
	}



private:
	CThreadFuncBase* m_pBase;
	SendCallBack m_callBack;
	CThreadRemote m_thread;

};
typedef CSafeSendQueue<std::vector<char>>::SendCallBack SENDCALLBACKK;

template<class T>
inline CSafeSendQueue<T>::CSafeSendQueue(CThreadFuncBase* obj, SendCallBack callback)
	: CSafeQueue<T>(),
	m_pBase(obj),
	m_callBack(callback)
{
	m_thread.Start();
	m_thread.UpdateWorker(::CThreadWorker(this, (FUNCTYPE) & CSafeSendQueue<T>::threadTick));
}
template<class T>
CSafeSendQueue<T>::~CSafeSendQueue()
{
	
	m_pBase = NULL;
	m_callBack = NULL;
	m_thread.Stop();
}






template<class T>
inline CSafeQueue<T>::CSafeQueue()
{
	m_hCompleoetionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
	m_hThread = INVALID_HANDLE_VALUE;
	m_lock = false;
	if (m_hCompleoetionPort != NULL)
	{
		std::thread thrad1(&CSafeQueue<T>::threadMain, this, m_hCompleoetionPort);
		m_hThread = thrad1.native_handle();
		thrad1.detach();
	}
}

template<class T>
inline CSafeQueue<T>::~CSafeQueue()
{
	if (m_lock == true)
	{
		return;
	}
	m_lock = true;
	PostQueuedCompletionStatus(m_hCompleoetionPort, 0, NULL, NULL);
	WaitForSingleObject(m_hThread, INFINITE);
	if (m_hCompleoetionPort != NULL)
	{
		HANDLE tem = m_hCompleoetionPort;
		m_hCompleoetionPort = NULL;
		CloseHandle(tem);
	}
		
}

template<class T>
inline bool CSafeQueue<T>::PushBack(const T& data)
{
	IocpParam* pParam = new IocpParam(EQPush, data);
	if (m_lock == true)
	{
		delete pParam;
		return false;
	}
	bool ret = PostQueuedCompletionStatus(m_hCompleoetionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
	if (ret == false)
	{
		delete pParam;
	}
	return ret;
}

template<class T>
inline bool CSafeQueue<T>::PopFront(T& data)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	IocpParam Param(EQPop, data, hEvent);
	if (m_lock == true)
	{
		if (hEvent != NULL)
		{
			CloseHandle(hEvent);
		}
		return false;
	}
	bool ret = PostQueuedCompletionStatus(m_hCompleoetionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
	if (ret == false)
	{
		CloseHandle(hEvent);
		return ret;
	}
	ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
	if (ret)
	{
		data = Param.Data;
	}
	return ret;
}

template<class T>
inline size_t CSafeQueue<T>::Size()
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	IocpParam Param(EQSize);
	if (m_lock == true)
	{
		if (hEvent != NULL)
		{
			CloseHandle(hEvent);
		}
		return -1;
	}
	bool ret = PostQueuedCompletionStatus(m_hCompleoetionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
	if (ret == false)
	{
		CloseHandle(hEvent);
		return -1;
	}
	ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
	if (ret)
	{
		return Param.size;
	}
	return -1;
}

template<class T>
inline void CSafeQueue<T>::Clear()
{
	if (m_lock == true)
	{
		return ;
	}
	IocpParam Param(EQClear);
	bool ret = PostQueuedCompletionStatus(m_hCompleoetionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
	if (ret == false)
	{
		return ;
	}
	return ;
}

template<class T>
inline void CSafeQueue<T>::threadMain(HANDLE arg)
{
	DWORD dwTransferred = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOVerlapped = NULL;
	int count = 0;
	while(GetQueuedCompletionStatus(m_hCompleoetionPort, &dwTransferred, &CompletionKey, &pOVerlapped, INFINITE))
	{
		if (dwTransferred == 0 && CompletionKey == NULL)
		{
			TRACE("thread is prepare to exit\r\n");
			break;
		}
		IocpParam* pParam = (IocpParam* )CompletionKey;
		DealParam(pParam);
		
	}
	while (GetQueuedCompletionStatus(m_hCompleoetionPort, &dwTransferred, &CompletionKey, &pOVerlapped, 0))
	{
		if (dwTransferred == 0 && CompletionKey == NULL)
		{
			TRACE("thread is prepare to exit\r\n");
			break;
		}
		IocpParam* pParam = (IocpParam*)CompletionKey;
		DealParam(pParam);
	}
	HANDLE temp = m_hCompleoetionPort;
	m_hCompleoetionPort = NULL;
	CloseHandle(temp);
	
}


template<class T>
inline void CSafeQueue<T>::DealParam(PPARAM* pParam)
{
	switch (pParam->nOperator)
	{
	case EQPush:
	{
		m_listData.push_back(pParam->Data);
		delete pParam;

	}
	break;
	case EQPop:
	{
		if (m_listData.size() > 0)
		{
			pParam->Data = m_listData.front();
			m_listData.pop_front();
		}
		if (pParam->hEvent != NULL)
		{
			SetEvent(pParam->hEvent);
		}
	}
	break;
	case EQClear:
	{
		m_listData.clear();
		delete pParam;
	}
	break;
	case EQSize:
	{
		pParam->size = m_listData.size();
		if (pParam->hEvent != NULL)
		{
			SetEvent(pParam->hEvent);
		}
	}
	break;
	default:
		TRACE("unkown\r\n");
		break;
	}
}
