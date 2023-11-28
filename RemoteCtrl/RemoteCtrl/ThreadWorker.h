#pragma once


class CTheadFuncBase
{
};
typedef int (CTheadFuncBase::* FUNCTYPE)(void);
class CThreadWorker
{
public:	
	CThreadWorker();
	CThreadWorker(CTheadFuncBase* obj, FUNCTYPE fun);
	CThreadWorker(const CThreadWorker& worker);
	CThreadWorker& operator=(const CThreadWorker& worker);
	int operator()();
	bool IsValid() const;
private:
	CTheadFuncBase* thiz;
	FUNCTYPE func;

};

