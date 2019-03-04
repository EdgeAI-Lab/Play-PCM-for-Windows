#ifndef _PLAYPCM_H_
#define _PLAYPCM_H_

#include "stdio.h"//此处写成双引号是因为新浪博客的原因，如果写成尖括号就不能显示。
#include "windows.h"
#include "MMSystem.h"

#define LENGTH 10240

#pragma comment(lib, "winmm.lib")
#pragma warning(disable:4996)

BOOL getOutNumDrive(); //判断是否有输出设备
BOOL getOutDevCaps(); //判断时候能够获取设备描述
//BOOL openOutFile(const char *cFileName, FILE **fp);//打开文件
BOOL initOutDrive(const char *cFileName); //初始化设备
BOOL openOutPcm(LPHWAVEOUT, LPCWAVEFORMATEX, DWORD_PTR);//打开相应的设备
LPSTR loadOutAudioBlock(const char* filename, DWORD*blockSize); //加载相应的文件信息
BOOL writeOutAudioBlock(HWAVEOUT hWaveOut, LPSTR block, DWORD size); //写声音文件到设备
BOOL closeOutAudioBlock(HWAVEOUT phwo); //关闭设备

#endif