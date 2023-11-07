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
	int ExcuteCommand(int nCmd, std::list<CPacket>& lspacket, CPacket&);
	static void RunCommand(void* arg, int status, std::list<CPacket>&, CPacket&);

protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& packetIn); //��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFuntion;//����һ�����ֵ�������ӳ��
	CLockDialog dlg;
	unsigned threadid = 0;
protected:
	int LockMachine(std::list<CPacket>&, CPacket& packetIn);
	int UNLockMachine(std::list<CPacket>&, CPacket& packetIn);
	int TestConnect(std::list<CPacket>&, CPacket& packetIn);
	int DeleteLocalFile(std::list<CPacket>&, CPacket& packetIn);
	int MakeDirectoryInfo(std::list<CPacket>&, CPacket& packetIn);
	int RunFile(std::list<CPacket>&, CPacket& packetIn);
	int DownLoadFile(std::list<CPacket>&, CPacket& packetIn);
	int MouseEvent(std::list<CPacket>&, CPacket& packetIn);
	int SendScreen(std::list<CPacket>&, CPacket& packetIn);
	int MakeDriverInfo(std::list<CPacket>&, CPacket& packetIn);
	static unsigned  threadLockDlg(void* arg);
private:
	
};

