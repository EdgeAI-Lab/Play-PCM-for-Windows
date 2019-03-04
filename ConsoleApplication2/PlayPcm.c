#include "pch.h"
#include "PlayPcm.h"

BOOL getOutNumDrive()//音频数量
{
	int count = waveOutGetNumDevs();
	return count < 1 ? FALSE : TRUE;//1 音频输入数量
}

BOOL getOutDevCaps()
{
	WAVEOUTCAPS waveOutcaps;
	MMRESULT mmResult = waveOutGetDevCaps(0, &waveOutcaps, sizeof(WAVEINCAPS));//2设备描述
	if (MMSYSERR_NOERROR != mmResult)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL openOutFile(const char *cFileName, FILE**fp)
{
	FILE *fpRead = NULL;
	fpRead = fopen(cFileName, "rb");
	if (NULL == fpRead)
	{
		fp = NULL;
		return FALSE;
	}
	*fp = fpRead;
	fclose(fpRead);
	return TRUE;
}

BOOL initOutDrive()//BOOL initDrive(const char*cFileName)
{
	if (FALSE == getOutNumDrive())
	{
		return FALSE;
	}
	if (FALSE == getOutDevCaps())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL openOutPcm(LPHWAVEOUT phwo, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback)
{
	if (MMSYSERR_NOERROR != waveOutOpen(0, 0, pwfx, 0, 0, WAVE_FORMAT_QUERY))
	{
		//fprintf(stderr, "不支持文件格式！\n");
		return FALSE;
	}

	if (MMSYSERR_NOERROR != waveOutOpen(phwo, WAVE_MAPPER, pwfx, 0, 0, CALLBACK_NULL))
	{
		//fprintf(stderr, "文件打开失败！\n");
		return FALSE;
	}
	return TRUE;
}

LPSTR loadOutAudioBlock(const char*filename, DWORD*blockSize)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD size = 0;
	DWORD readBytes = 0;
	void* block = NULL;
	if ((hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	do 
	{
		if ((size = GetFileSize(hFile, NULL)) == 0)
		{
			break;
		}
		if ((block = HeapAlloc(GetProcessHeap(),0, size)) == NULL)
		{
			break;
		}
		ReadFile(hFile, block, size,&readBytes,NULL);
	} while (0);
	CloseHandle(hFile);
	*blockSize = size;
	return (LPSTR)block;
}

BOOL writeOutAudioBlock(HWAVEOUT phwo, LPSTR block, DWORDsize)
{
	WAVEHDR wHdr = { 0 };
	wHdr.dwBufferLength = size;
	wHdr.lpData = block;
	if (MMSYSERR_NOERROR != waveOutPrepareHeader(phwo, &wHdr, sizeof(WAVEHDR)))
	{
		//fprintf(stderr, "文件缓冲区准备失败！\n");
		return FALSE;
	}


	if (MMSYSERR_NOERROR != waveOutWrite(phwo, &wHdr, sizeof(WAVEHDR)))
	{
		//fprintf(stderr, "写文件缓冲区失败！\n");
		return FALSE;
	}
	Sleep(500);

	while (WAVERR_STILLPLAYING == waveOutUnprepareHeader(phwo, &wHdr, sizeof(WAVEHDR)))
	{
		Sleep(100);
	}
	return TRUE;
}

BOOL closeOutAudioBlock(HWAVEOUT phwo)
{
	if (MMSYSERR_NOERROR != waveOutClose(phwo))
	{
		//fprintf(stderr, "写文件缓冲区失败！\n");
		return FALSE;
	}
	return TRUE;
}