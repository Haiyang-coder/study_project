#include "pch.h"
#include "Packet.h"
#define BUFFER_SIZE 4096
CPacket::CPacket()
{
}

CPacket::CPacket(const BYTE* pData, size_t& nSize)
{
	size_t i = 0;
	for (i = 0; i < nSize; i++)
	{
		//填充数据头
		if (*(WORD*)(pData + i) == 0xFEFF)
		{
			sHead = *(WORD*)(pData + i);
			i += 2;
			break;
		}
	}
	//如果数据包不全，或者数据包没有完全接收到
	if (i + 4 + 2 + 2 > nSize)
	{
		nSize = 0;
		return;
	}

	nLength = *(DWORD*)(pData + i);
	i += 4;
	if (nLength + i > nSize)
	{
		//数据没有接受完整
		nSize = 0;
		return;
	}

	sCmd = *(WORD*)(pData + i);
	i += 2;
	if (nLength > 4)
	{
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.data(), pData + i, nLength - 4);
		i += nLength - 4;
	}
	sSum = *(WORD*)(pData + i);
	i += 2;
	size_t sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;
	}
	if (sum == sSum)
	{
		nSize = i;//head length data
	}
	return;
}

CPacket::CPacket(const CPacket& packet)
{
	sHead = packet.sHead;//数据头
	nLength = packet.nLength;//数据长度（从控制命令到校验的长度）
	sCmd = packet.sCmd;//控制命令
	strData = packet.strData;//数据
	sSum = packet.sSum;//校验
}

CPacket& CPacket::operator=(const CPacket& packet)
{
	if (this == &packet)
	{
		return *this;
	}
	sHead = packet.sHead;//数据头
	nLength = packet.nLength;//数据长度（从控制命令到校验的长度）
	sCmd = packet.sCmd;//控制命令
	strData = packet.strData;//数据
	sSum = packet.sSum;//校验
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
{
	sHead = 0xFEFF;
	nLength = nSize + 4;
	sCmd = nCmd;
	if (nSize > 0)
	{
		strData.resize(nSize);
		memcpy((void*)strData.data(), pData, nSize);
	}
	else {
		strData.clear();
	}
	sSum = 0;
	for (size_t i = 0; i < strData.size(); i++)
	{
		sSum += BYTE(strData[i]) & 0xFF;
	}
}

CPacket::~CPacket()
{
}

int CPacket::Size()
{
	return nLength + 6;
}

const char* CPacket::Data()
{
	strOut.resize(nLength + 6);
	BYTE* pData = (BYTE*)strOut.c_str();
	*(WORD*)pData = sHead; pData += 2;
	*(DWORD*)pData = nLength; pData += 4;
	*(WORD*)pData = sCmd; pData += 2;
	memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
	*(WORD*)pData = sSum;
	return strOut.c_str();
}
