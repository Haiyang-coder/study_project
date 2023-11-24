#pragma once
#include<list>
#include<string>
#include<conio.h>
#include<thread>
#include<mutex>
#include<atomic>

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
	~CSafeQueue();
	bool PushBack(const T& data);
	bool PopFront(T& data);
	size_t Size();
	void Clear();
private:
	void threadMain(HANDLE arg);
	void DealParam(PPARAM* pParam);
private:
	std::list<T> m_listData;
	HANDLE m_hCompleoetionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock; //队列正在析构


};

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
	HANDLE tem = m_hCompleoetionPort;
	PostQueuedCompletionStatus(m_hCompleoetionPort, 0, NULL, NULL);
	WaitForSingleObject(m_hThread, INFINITE);
	m_hCompleoetionPort = NULL;
	CloseHandle(tem);
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
	
	CloseHandle(m_hCompleoetionPort);
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
