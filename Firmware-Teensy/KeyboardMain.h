/*
            IRON-HID Firmware for At90USB1286 in Teensy2.0++       
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


#ifndef _KEYBOARDMAIN_H_
#define _KEYBOARDMAIN_H_

////////////////////////////////////////////////////////////////////
// Key Structure
////////////////////////////////////////////////////////////////////
#define MAX_KEYCODE        6

typedef struct usbReportStruct
{
	unsigned char cModifier;
	unsigned char cReserved;
	unsigned char vcKeyCode[MAX_KEYCODE];
} USBREPORT;


////////////////////////////////////////////////////////////////////
// Gobal Variable
////////////////////////////////////////////////////////////////////
// TX/RX Pin Number
//int iBlueToothTx = 10;
//int iBlueToothRx = 11;

// CMD Type Define
#define CMD_TYPE_UNKNOWN     0
#define CMD_TYPE_DEBUG      'D'
#define CMD_TYPE_READ       'R'
#define CMD_TYPE_FILE       'F'
#define CMD_TYPE_COMMAND    'C'
#define CMD_TYPE_KEY        'K'
#define CMD_TYPE_LED        'L'
#define CMD_TYPE_ATTACK     'A'

// Define Recv Mode
#define MODE_COUNT          4

#define MODE_CMD_RECV       0
#define MODE_KEY_RECV       1
#define MODE_DATA_RECV      2
#define MODE_ATTACK_START   3
#define MODE_UNKNOWN        4

// Define Buffer Index
#define BUFFER_INDEX_CMD    MODE_CMD_RECV
#define BUFFER_INDEX_KEY    MODE_KEY_RECV


// Define State
#define STATE_RECV_CMD      0
#define STATE_RECV_SECTOR   1
#define STATE_RECV_DATA     2

#define FILE_CHUNK_SIZE     64
#define BULK_CHUNK_SIZE     32

////////////////////////////////////////////////////////////////////
// Gobal Function
////////////////////////////////////////////////////////////////////
void loop();
void ProcessRemoteCMD(void);
void ProcessHostCMD(void);
void SendFileDataToRemote(char* pcBuffer, int iSize);
void SendRemoteDataToHost(int iRecvMode, int iSize);
void SetLEDState(char cLED);
void ToggleLEDState(USBREPORT* pstUSBReport);
char CheckLEDKeyPressed(USBREPORT* pstUSBReport);
bool SaveKeyDataToHost();
void SendKeyToHost(bool bOnlySendRemoteKey);
bool SendRemoteKeyToHost();
void InitMagicStringFinder();
bool MatchMagicString(char cData);
int DumpBufferdDataForMagicString(char* pcBuffer);
void EnterSleep(void);
void EnterDeepSleep(void);
void CheckButtonAndReset(void);
bool ConvertFNKey(USBREPORT* pstUSBReport, unsigned char* pcConvertKey);
bool IsAllKeyIsReleased();
bool SaveKeyDataToHost();
void SendFileDataToRemote(char* pcBuffer, int iSize);
void GetRemoteCMDFromBuffer(char* pcBuffer, int iSize);
void SendKeyToHost(bool bOnlySendRemoteKey);
void SetLEDState(char cLED);
void SendPinPattern();
bool IsRemoteKeyBufferEmpty();
void AddNumberToRemoteKeyBuffer(char* pcNumberString);


#ifdef __cplusplus
extern "C"
{
#endif
void GetSectorData(int iSectorNumber);
#ifdef __cplusplus
}
#endif

extern USBREPORT g_stReport;
extern char g_cAttackStart;
extern char g_vcSectorBuffer[512];
#endif
