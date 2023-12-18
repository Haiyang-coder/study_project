#include "pch.h"
#include "ThreadWorker.h"

CThreadWorker::CThreadWorker():thiz(NULL),func(NULL)
{
}

CThreadWorker::CThreadWorker(CThreadFuncBase* obj, FUNCTYPE fun) :thiz(obj), func(fun)
{
}

CThreadWorker::CThreadWorker(const CThreadWorker& worker)
{
	if (this == &worker)
	{
		thiz = worker.thiz;
		func = worker.func;
	}
	
}

CThreadWorker& CThreadWorker::operator=(const CThreadWorker& worker)
{
	if (this != &worker)
	{
		thiz = worker.thiz;
		func = worker.func;
	}
	return *this;
}

int CThreadWorker::operator()()
{
	if (IsValid())
	{
		return (thiz->*func)();
	}
	return -1;
}
bool CThreadWorker::IsValid() const
{
	return thiz != NULL && func != NULL;
}
