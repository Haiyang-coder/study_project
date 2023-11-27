#pragma once
#include"ThreadWorker.h"
#include<vector>
#include<mutex>
class CThreadRemote
{
public:
	CThreadRemote();
	~CThreadRemote();

	bool boolIsValid();
	bool Start();
	bool Stop();
	void UpdateWorker(const CThreadWorker& woker = CThreadWorker());
	bool Isdle();
private:
	static void TheadEntry(void* arg);
	virtual void ThreadWorker();
protected:
	//С������ֹ�߳�ѭ��
	//�����㾯����־
	//����������

private:
	HANDLE m_hThread;
	bool m_bStatus;//true: �߳���������  false:�߳̽�Ҫ�ر�
	std::atomic<CThreadWorker> m_worker;
};



class CTheadPool
{
public:
	CTheadPool(size_t size);
	CTheadPool();
	~CTheadPool();

	bool Invoke();
	void Stop();
	int DispatchWorker(const CThreadWorker& woker);
	bool CheckThreadValid(int index);

private:
	std::vector<CThreadRemote> m_vecThead;
	std::mutex m_lock;

};

