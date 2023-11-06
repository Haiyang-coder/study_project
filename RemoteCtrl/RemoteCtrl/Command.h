#pragma once

#include<map>
#include"SeverSocket.h"
#include<list>
#include<atlimage.h>
#include<direct.h>
#include"RemteServerTool.h"
#include<io.h>
#include"LockDialog.h"
#include"LockDialog.h"
#include "resource.h"


class CCommand
{
public:
	CCommand();
	~CCommand();
	int ExcuteCommand(int nCmd);

protected:
	typedef int(CCommand::* CMDFUNC)(); //��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFuntion;//����һ�����ֵ�������ӳ��
	CLockDialog dlg;
	unsigned threadid = 0;
protected:
	int LockMachine();
	int UNLockMachine();
	int TestConnect();
	int DeleteLocalFile();
	int MakeDirectoryInfo();
	int RunFile();
	int DownLoadFile();
	int MouseEvent();
	int SendScreen();
	int MakeDriverInfo();
	static unsigned  threadLockDlg(void* arg);
private:
	
};

