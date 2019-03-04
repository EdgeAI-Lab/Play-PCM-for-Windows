#ifndef _PLAYPCM_H_
#define _PLAYPCM_H_

#include "stdio.h"//�˴�д��˫��������Ϊ���˲��͵�ԭ�����д�ɼ����žͲ�����ʾ��
#include "windows.h"
#include "MMSystem.h"

#define LENGTH 10240

#pragma comment(lib, "winmm.lib")
#pragma warning(disable:4996)

BOOL getOutNumDrive(); //�ж��Ƿ�������豸
BOOL getOutDevCaps(); //�ж�ʱ���ܹ���ȡ�豸����
//BOOL openOutFile(const char *cFileName, FILE **fp);//���ļ�
BOOL initOutDrive(const char *cFileName); //��ʼ���豸
BOOL openOutPcm(LPHWAVEOUT, LPCWAVEFORMATEX, DWORD_PTR);//����Ӧ���豸
LPSTR loadOutAudioBlock(const char* filename, DWORD*blockSize); //������Ӧ���ļ���Ϣ
BOOL writeOutAudioBlock(HWAVEOUT hWaveOut, LPSTR block, DWORD size); //д�����ļ����豸
BOOL closeOutAudioBlock(HWAVEOUT phwo); //�ر��豸

#endif