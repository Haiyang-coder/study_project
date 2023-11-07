#pragma once
#include"pch.h"
#include"framework.h"
#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket();
	CPacket(const BYTE*, size_t& nSize);
	CPacket(const CPacket& packet);
	CPacket& operator=(const CPacket& packet);
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize);
	~CPacket();

	int Size();
	const char* Data();
public:
	WORD sHead = 0;//数据头
	DWORD nLength = 0;//数据长度（从控制命令到校验的长度）
	WORD sCmd = 0;//控制命令
	std::string strData = "";//数据
	WORD sSum = 0;//校验(只校验数据部分)
	std::string strOut;	//整個包的數據

private:

};
#pragma pack(pop)
