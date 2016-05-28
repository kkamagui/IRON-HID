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

/** \file
 *
 *  Header file for Descriptors.c.
 */
 
#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

//#define CHERRY_COMPACT       // Cherry Compact Keyboard
#define MATIAS_TACTILE_PRO   // Matias Tactile Pro Keyboard
//#define MATIAS_LAPTOP_PRO      // Matias Laptop Pro Keyboard

	/* Includes: */
		#include <avr/pgmspace.h>

		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Drivers/USB/Class/MassStorage.h>
		#include <LUFA/Drivers/USB/Class/HID.h>

	/* Macros: */
		/** Endpoint number of the Keyboard HID reporting IN endpoint. */
		#define KEYBOARD_EPNUM               1
		
		/** Size in bytes of the Keyboard HID reporting IN and OUT endpoints. */		
		#define KEYBOARD_EPSIZE              8

		/** Endpoint number of the Mass Storage device-to-host data IN endpoint. */
		#define MASS_STORAGE_IN_EPNUM        3	

		/** Endpoint number of the Mass Storage host-to-device data OUT endpoint. */
		#define MASS_STORAGE_OUT_EPNUM       4	

		/** Size in bytes of the Mass Storage data endpoints. */
		#define MASS_STORAGE_IO_EPSIZE       64

		/** Endpoint number of the User HID reporting IN endpoint. */
		#define USER_EPNUM               	2
		
		/** Size in bytes of the User HID reporting IN and OUT endpoints. */		
		#define USER_EPSIZE              	8

		/** Interface Number of the keyboard */
		#define KEYBOARD_IFNUM				0
		/** Interface Number of the User */
		#define USER_IFNUM					1
		/** Interface Number of the Mass Storage */
		#define MASS_STORAGE_IFNUM			2


	/* Type Defines: */		
		/** Type define for the device configuration descriptor structure. This must be defined in the
		 *  application code, as the configuration descriptor contains several sub-descriptors which
		 *  vary between devices, and which describe the device's usage to the host.
		 */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
			USB_Descriptor_Interface_t            HID_KeyboardInterface;
			USB_HID_Descriptor_t                  HID_KeyboardHID;
	        USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
			
			USB_Descriptor_Interface_t            HID_UserInterface;
			USB_HID_Descriptor_t                  HID_UserHID;
	        USB_Descriptor_Endpoint_t             HID_UserReportINEndpoint;
		} USB_Descriptor_Configuration_Normal_t;
		
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
			USB_Descriptor_Interface_t            MS_Interface;
			USB_Descriptor_Endpoint_t             MS_DataInEndpoint;
			USB_Descriptor_Endpoint_t             MS_DataOutEndpoint;
			
			USB_Descriptor_Interface_t            HID_KeyboardInterface;
			USB_HID_Descriptor_t                  HID_KeyboardHID;
	        USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
			
			USB_Descriptor_Interface_t            HID_UserInterface;
			USB_HID_Descriptor_t                  HID_UserHID;
	        USB_Descriptor_Endpoint_t             HID_UserReportINEndpoint;
		} USB_Descriptor_Configuration_Attack_t;
		
	/* Function Prototypes: */
		uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
		                                    const uint8_t wIndex,
		                                    void** const DescriptorAddress) ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
