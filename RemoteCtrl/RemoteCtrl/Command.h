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
	typedef int(CCommand::* CMDFUNC)(); //成员函数指针
	std::map<int, CMDFUNC> m_mapFuntion;//构造一个数字到函数的映射
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

