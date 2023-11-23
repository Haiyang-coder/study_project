#pragma once
#include<list>
#include<string>

typedef void*(* listcallBack)(void*);

template<class T>
class CSafeQueue
{
	//利用iocp实现的线程安全的队列
public:
	CSafeQueue();
	~CSafeQueue();
	bool PushBack(const T& data);
	bool PopFront(T& data);
	size_t Size();
	void Clear();
private:
	void threadMain();
private:
	std::list<T> m_listData;
	HANDLE m_hCompleoetionPort;
	HANDLE m_hThread;
public:
	enum
	{
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};
	typedef struct IocpParam
	{
		int nOperator;
		T strData;
		listcallBack lstCall;
		HANDLE hEvent;//pop操作需要的
		IocpParam(int op, T sData, listcallBack cb = NULL)
		{
			nOperator = op;
			strData = sData;
			lstCall = cb;
		}
		IocpParam()
		{
			nOperator = -1;
		}

	}PPARAM;//用于投递信息的结构体

};

