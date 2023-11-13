#pragma once
#include<Windows.h>
#include <string>
#include<atlimage.h>
class CRemteClientTool
{

public:
    static void Dump(BYTE* pData, size_t nSize)
    {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))
                strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
			  

        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());

    }

    static int Byte2Image(CImage& image, const std::string& strBuffer)
    {
		//:拿到数据后要将数据存入缓存中
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hmem == NULL)
		{
			TRACE("内存不足，无法申请足够的空间\r\n");
			Sleep(10);
			return -1;
		}
		IStream* pstream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hmem, TRUE, &pstream);
		if (hRet == S_OK)
		{
			ULONG length = 0;
			pstream->Write(strBuffer.c_str(), strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pstream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != nullptr)
			{
				image.Destroy();
			}
			image.Load(pstream);
		}
		return hRet;
    }

};

