#pragma once
#include"ThreadFuncBase.h"
using FUNCTYPE = int(CTheadFuncBase::*)();
class CThreadWorker
{
public:
	CThreadWorker();
	CThreadWorker(CTheadFuncBase* obj, FUNCTYPE fun);
	CThreadWorker(const CThreadWorker& worker);
	CThreadWorker& operator=(const CThreadWorker& worker);
	int operator()();
	bool IsValid();
private:
	CTheadFuncBase* thiz;
	FUNCTYPE func;

};

