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
 *  Header file for SCSI.c.
 */

#define CDROM
 
#ifndef _SCSI_H_
#define _SCSI_H_

	// Includes: 
		#include <avr/io.h>
		#include <avr/pgmspace.h>

		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Drivers/USB/Class/MassStorage.h>
		#include <LUFA/Drivers/Peripheral/Serial.h>

		#include "KeyboardFirmware.h"
		#include "Descriptors.h"
		#include "SCSI_Codes.h"
	
	// Macros: 
		/** 
		 *	Macro to set the current SCSI sense data to the given key, additional sense code and additional sense qualifier. This
		 *  is for convenience, as it allows for all three sense values (returned upon request to the host to give information about
		 *  the last command failure) in a quick and easy manner.
		 *
		 *  \param[in] key    New SCSI sense key to set the sense code to
		 *  \param[in] acode  New SCSI additional sense key to set the additional sense code to
		 *  \param[in] aqual  New SCSI additional sense key qualifier to set the additional sense qualifier code to
		 */
		#ifdef CDROM
			#define SCSI_SET_SENSE(key, acode, aqual)  MACROS{ SenseData_CD.SenseKey = key;              \
															   SenseData_CD.AdditionalSenseCode = acode; \
															   SenseData_CD.AdditionalSenseQualifier = aqual; }MACROE
		#else
			#define SCSI_SET_SENSE(key, acode, aqual)  MACROS{ SenseData_MS.SenseKey = key;              \
															   SenseData_MS.AdditionalSenseCode = acode; \
															   SenseData_MS.AdditionalSenseQualifier = aqual; }MACROE

		#endif
		/** Macro for the \ref SCSI_Command_ReadWrite_10() function, to indicate that data is to be read from the storage medium. */
		#define DATA_READ           true

		/** Macro for the \ref SCSI_Command_ReadWrite_10() function, to indicate that data is to be written to the storage medium. */
		#define DATA_WRITE          false

		/** Value for the DeviceType entry in the SCSI_Inquiry_Response_t enum, indicating a Block Media device. */
		#define DEVICE_TYPE_BLOCK   0x00
		
		/** Value for the DeviceType entry in the SCSI_Inquiry_Response_t enum, indicating a CD-ROM device. */
		#define DEVICE_TYPE_CDROM   0x05

		// Copy Data From DataflashManager.h
                /** Total number of bytes of the storage medium, comprised of one or more Dataflash ICs. */
		#define VIRTUAL_MEMORY_BYTES                ((unsigned long)200 * 1024)

		/** Block size of the device. This is kept at 512 to remain compatible with the OS despite the underlying
		 *  storage media (Dataflash) using a different native block size.
		*/
		#ifdef CDROM
			#define VIRTUAL_MEMORY_BLOCK_SIZE           2048
		#else
			#define VIRTUAL_MEMORY_BLOCK_SIZE           512
		#endif
	    	/** Total number of blocks of the virtual memory for reporting to the host as the device's total capacity. */
	  	#define VIRTUAL_MEMORY_BLOCKS              (VIRTUAL_MEMORY_BYTES / VIRTUAL_MEMORY_BLOCK_SIZE)

        	/** Total number of logical drives within the device - must be non-zero. */
		#define TOTAL_LUNS                          1
		
		/** Blocks in each LUN, calculated from the total capacity divided by the total number of Logical Units in the device. */
		 #define LUN_MEDIA_BLOCKS                   (VIRTUAL_MEMORY_BLOCKS / TOTAL_LUNS)

		#define DF_MANUFACTURER_ATMEL 	0x1F
		#define DATAFLASH_PAGE_SIZE		512
		
	/* Function Prototypes: */
		bool SCSI_DecodeSCSICommand(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
		
		#if defined(INCLUDE_FROM_SCSI_C)
			static bool SCSI_Command_Inquiry(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Request_Sense(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Read_Capacity_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_Send_Diagnostic(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
			static bool SCSI_Command_ReadWrite_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
			                                      const bool IsDataRead);
			static void SCSI_Command_Get_Config(USB_ClassInfo_MS_Device_t* MSInterfaceInfo);
			static void SCSI_Command_Read_TOC(USB_ClassInfo_MS_Device_t* MSInterfaceInfo);
			static void SCSI_Command_Mode_Sense(USB_ClassInfo_MS_Device_t* MSInterfaceInfo);
			static void SCSI_Command_Mode_Sense_10(USB_ClassInfo_MS_Device_t* MSInterfaceInfo);
			static void SCSI_Command_Read_Disk_Info(USB_ClassInfo_MS_Device_t* MSInterfaceInfo);
		#endif
		
		void DataflashManager_ReadBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                 const uint32_t BlockAddress,
                                 uint16_t TotalBlocks);
		void DataflashManager_WriteBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                  const uint32_t BlockAddress,
                                  uint16_t TotalBlocks);


		
#endif
