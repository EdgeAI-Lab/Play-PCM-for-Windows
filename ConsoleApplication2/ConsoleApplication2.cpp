#include "pch.h"
#include<windows.h>
#include<mmsystem.h>
#include<stdio.h>
#include<queue>
using std::queue;
#pragma comment(lib,"winmm.lib") 

/*
* some good values for block size and count
*/
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 3
/*
*waveoutprocdecodethread share vars
*/
static CRITICAL_SECTION waveCriticalSection;
static HANDLE freeBufferSema;
static queue<WAVEHDR*> qWHDR;

typedef struct decodeThreadParam
{
	char *filename;
	HWAVEOUT *device;
}decThreadParmT;

static DWORD decodeThreadProc(LPVOID lpdwThreadParam);
static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int main()
{
	HWAVEOUT hWaveOut;
	MMRESULT ret;
	WAVEFORMATEX wfx;

	DWORD decodeThreadId;
	HANDLE hDecodeThread;
	char filename[] = "test.pcm";
	decThreadParmT decThreadData;

	int i;

	freeBufferSema = CreateSemaphore(0,
		0,
		BLOCK_COUNT,
		(LPCWSTR)"bufferSema"
	);

	if (freeBufferSema == NULL)
	{
		printf("Create Buffer Sema ERROR!\n");
		return 1;
	}

	InitializeCriticalSection(&waveCriticalSection);


	wfx.nSamplesPerSec = 8000;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 1;
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample*wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nBlockAlign;
	ret = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	if (ret != MMSYSERR_NOERROR)
	{
		printf("waveoutopen ERR!\n");
		return 1;
	}
	printf("waveout open successfully!\n");
	decThreadData.device = &hWaveOut;
	decThreadData.filename = filename;

	hDecodeThread = CreateThread(NULL, //Choose default security
		0, //Default stack size
		(LPTHREAD_START_ROUTINE)&decodeThreadProc,
		//Routine to execute
		(LPVOID)&decThreadData, //Thread parameter
		0, //Immediately run the thread
		&decodeThreadId //Thread Id
	);

	if (hDecodeThread == NULL)
	{
		printf("Error Creating Decode Thread\n");
		return 1;
	}

	WaitForSingleObject(hDecodeThread, INFINITE);

	Sleep(1000);

	waveOutClose(hWaveOut);
	CloseHandle(freeBufferSema);
	CloseHandle(hDecodeThread);
	DeleteCriticalSection(&waveCriticalSection);

}


static DWORD decodeThreadProc(LPVOID lpdwThreadParam)
{
	char *filename = ((decThreadParmT*)lpdwThreadParam)->filename;
	HWAVEOUT *hWaveOut = ((decThreadParmT*)lpdwThreadParam)->device;
	WAVEHDR* current = NULL;
	WAVEHDR* waveBlocks = NULL;
	char *audioBuf = NULL;
	FILE *fp = NULL;
	int i;

	audioBuf = (char*)malloc(BLOCK_SIZE*BLOCK_COUNT);
	memset(audioBuf, 0, BLOCK_SIZE*BLOCK_COUNT);
	waveBlocks = (WAVEHDR*)malloc(BLOCK_COUNT * sizeof(WAVEHDR));
	memset(waveBlocks, 0, BLOCK_COUNT * sizeof(WAVEHDR));
	for (i = 0; i < BLOCK_COUNT; i++)
	{
		waveBlocks[i].lpData = (audioBuf + i * BLOCK_SIZE);
		waveBlocks[i].dwBufferLength = BLOCK_SIZE;
	}

	fopen_s(&fp,filename, "rb");
	if (fp == NULL)
	{
		printf("open file :%s error\n", filename);
		return -1;
	}

	//preread BLOCK_COUNT data to buffer
	for (i = 0; i < BLOCK_COUNT; i++)
	{
		current = &waveBlocks[i];
		fread(current->lpData, 1, BLOCK_SIZE, fp);
		waveOutPrepareHeader(*hWaveOut, current, sizeof(WAVEHDR));

	}
	for (i = 0; i < BLOCK_COUNT; i++)
	{
		current = &waveBlocks[i];
		printf("i:%x\n", current);
		waveOutWrite(*hWaveOut, current, sizeof(WAVEHDR));
	}

	while (1)
	{
		int size = 0;
		//wait a free pcm buf
		WaitForSingleObject(
			freeBufferSema, // event handle
			INFINITE);    // indefinite wait

		//read data to pcm buf,then write to waveout
		//
		EnterCriticalSection(&waveCriticalSection);
		current = qWHDR.front();
		qWHDR.pop();
		LeaveCriticalSection(&waveCriticalSection);
		printf("f:%x\n", current);
		waveOutUnprepareHeader(*hWaveOut, current, sizeof(WAVEHDR));
		size = fread(current->lpData, 1, BLOCK_SIZE, fp);
		//printf("size:%d\n",size);
		if (size > 0)
		{
			waveOutPrepareHeader(*hWaveOut, current, sizeof(WAVEHDR));
			waveOutWrite(*hWaveOut, current, sizeof(WAVEHDR));
		}
		else
		{
			printf("play over!%d\n", size);
			break;
		}
	}
	printf("decode finished!\n");
	fclose(fp);
	free(audioBuf);
	free(waveBlocks);
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	WAVEHDR *freeWaveHdr = NULL;
	switch (uMsg)
	{
	case WOM_DONE:
		EnterCriticalSection(&waveCriticalSection);
		freeWaveHdr = (WAVEHDR*)dwParam1;
		printf("o:%x\n", freeWaveHdr);
		qWHDR.push(freeWaveHdr);
		LeaveCriticalSection(&waveCriticalSection);
		ReleaseSemaphore(freeBufferSema, 1, NULL);
		break;
	case WOM_OPEN:
		printf("open waveout\n");
		break;
	case WOM_CLOSE:
		printf("close waveout\n");
		break;
	default:
		printf("ERROR MSG!\n");
	}

}