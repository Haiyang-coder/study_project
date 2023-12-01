#pragma once


class CThreadFuncBase
{
};
typedef int (CThreadFuncBase::* FUNCTYPE)(void);
class CThreadWorker
{
public:	
	CThreadWorker();
	CThreadWorker(CThreadFuncBase* obj, FUNCTYPE fun);
	CThreadWorker(const CThreadWorker& worker);
	CThreadWorker& operator=(const CThreadWorker& worker);
	int operator()();
	bool IsValid() const;
private:
	CThreadFuncBase* thiz;
	FUNCTYPE func;

};

