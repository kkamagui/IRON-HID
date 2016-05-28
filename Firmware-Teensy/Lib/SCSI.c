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
  Copyright (c) 2011 Curtis Reno

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
 *  SCSI command processing routines, for SCSI commands issued by the host. Mass Storage
 *  devices use a thin "Bulk-Only Transport" protocol for issuing commands and status information,
 *  which wrap around standard SCSI device commands for controlling the actual storage medium.
 */
 
#define  INCLUDE_FROM_SCSI_C
#include "SCSI.h"
#include <stdio.h>
#include "KeyboardMain.h"

/** 
 *	Structure to hold the SCSI response data to a SCSI INQUIRY command. This gives information about the device's
 *  features and capabilities.
 */
SCSI_Inquiry_Response_t InquiryData_MS = 
	{
		.DeviceType          = DEVICE_TYPE_BLOCK,
		.PeripheralQualifier = 0,
			
		.Removable           = true,
			
		.Version             = 0,
			
		.ResponseDataFormat  = 2,
		.NormACA             = false,
		.TrmTsk              = false,
		.AERC                = false,

		.AdditionalLength    = 0x1F,
			
		.SoftReset           = false,
		.CmdQue              = false,
		.Linked              = false,
		.Sync                = false,
		.WideBus16Bit        = false,
		.WideBus32Bit        = false,
		.RelAddr             = false,

#if defined CHERRY_COMPACT
		.VendorID            = "Cherry",
		.ProductID           = "Utility Disk",
#elif defined MATIAS_TACTILE_PRO
		.VendorID            = "Matias",
		.ProductID           = "Utility Disk",
#elif defined MATIAS_LAPTOP_PRO
		.VendorID            = "Matias",
		.ProductID           = "Utility Disk",
#else
		.VendorID            = "General",
		.ProductID           = "Utility Disk",
#endif
		.RevisionID          = {'0','.','0','0'},
	};

SCSI_Inquiry_Response_t InquiryData_CD = 
	{
		.DeviceType          = DEVICE_TYPE_CDROM,
		.PeripheralQualifier = 0,
			
		.Removable           = true,
			
		.Version             = 2,
			
		.ResponseDataFormat  = 2,
		.NormACA             = false,
		.TrmTsk              = false,
		.AERC                = false,

		.AdditionalLength    = 0x1F,
			
		.SoftReset           = false,
		.CmdQue              = false,
		.Linked              = false,
		.Sync                = false,
		.WideBus16Bit        = false,
		.WideBus32Bit        = false,
		.RelAddr             = false,
		
#if defined CHERRY_COMPACT
		.VendorID            = "Cherry",
		.ProductID           = "Utility Disk",
#elif defined MATIAS_TACTILE_PRO
		.VendorID            = "Matias",
		.ProductID           = "Utility Disk",
#elif defined MATIAS_LAPTOP_PRO
		.VendorID            = "Matias",
		.ProductID           = "Utility Disk",
#else
		.VendorID            = "General",
		.ProductID           = "Utility Disk",
#endif
		.RevisionID          = {'0','.','0','0'},
	};


/** 
 * 	Structure to hold the sense data for the last issued SCSI command, which is returned to the host after a SCSI REQUEST SENSE
 *  command is issued. This gives information on exactly why the last command failed to complete.
 */
SCSI_Request_Sense_Response_t SenseData_MS =
	{
		.ResponseCode        = 0x70,
		.AdditionalLength    = 0x0A,
	};

SCSI_Request_Sense_Response_t SenseData_CD =
	{
		.ResponseCode        = 0x05,
		.AdditionalLength    = 0x24,
	};


/** 
 *	Main routine to process the SCSI command located in the Command Block Wrapper read from the host. This dispatches
 *  to the appropriate SCSI command handling routine if the issued command is supported by the device, else it returns
 *  a command failure due to a ILLEGAL REQUEST.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 */
bool SCSI_DecodeSCSICommand(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess = false;

	/*
	// For debugging
	Serial_TxString("D;;Command ");
	MyItoA(vcBuffer, (unsigned char)MSInterfaceInfo->State.CommandBlock.SCSICommandData[0]);
	Serial_TxString(vcBuffer);
	Serial_TxString(";");
	*/
	SCSI_SET_SENSE(SCSI_SENSE_KEY_GOOD,
				   SCSI_ASENSE_NO_ADDITIONAL_INFORMATION,
				   SCSI_ASENSEQ_NO_QUALIFIER);

	/* Run the appropriate SCSI command hander function based on the passed command */
	switch (MSInterfaceInfo->State.CommandBlock.SCSICommandData[0])
	{
		case SCSI_CMD_INQUIRY:
			CommandSuccess = SCSI_Command_Inquiry(MSInterfaceInfo);	
			//Serial_TxString("D;0;SCSI_CMD_INQUIRY\r\n;");
			break;

		case SCSI_CMD_REQUEST_SENSE:
			CommandSuccess = SCSI_Command_Request_Sense(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_REQUEST_SENSE\r\n;");
			break;

		case SCSI_CMD_READ_CAPACITY_10:
			CommandSuccess = SCSI_Command_Read_Capacity_10(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_REQUEST_CAPACITY\r\n;");
			break;

		case SCSI_CMD_SEND_DIAGNOSTIC:
			CommandSuccess = SCSI_Command_Send_Diagnostic(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_SEND_DIAGNOSTIC\r\n;");
			break;

		case SCSI_CMD_WRITE_10:
			CommandSuccess = SCSI_Command_ReadWrite_10(MSInterfaceInfo, DATA_WRITE);
			//Serial_TxString("D;0;SCSI_CMD_WRITE_10\r\n;");
			break;

		case SCSI_CMD_READ_10:
			CommandSuccess = SCSI_Command_ReadWrite_10(MSInterfaceInfo, DATA_READ);
			//Serial_TxString("D;0;SCSI_CMD_READ_10\r\n;");
			break;

		case SCSI_CMD_GET_CONFIG:
			CommandSuccess = true;
			SCSI_Command_Get_Config(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_GET_CONFIG\r\n;");
			break;

		case SCSI_CMD_READ_TOC:
			CommandSuccess = true;
			SCSI_Command_Read_TOC(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_READ_TOC\r\n;");
			break;

		case SCSI_CMD_MODE_SENSE:
			CommandSuccess = true;
			SCSI_Command_Mode_Sense(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_MODE_SENSE\r\n;");
			break;
			
		case SCSI_CMD_MODE_SENSE_10:
			CommandSuccess = true;
			SCSI_Command_Mode_Sense_10(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_MODE_SENSE_10\r\n;");
			break;

		case SCSI_CMD_READ_DISK_INFO:
			CommandSuccess = true;
			SCSI_Command_Read_Disk_Info(MSInterfaceInfo);
			//Serial_TxString("D;0;SCSI_CMD_READ_10\r\n;");
			break;

		case SCSI_CMD_READ_CD:
			CommandSuccess = SCSI_Command_ReadWrite_10(MSInterfaceInfo, DATA_READ);
			//Serial_TxString("D;0;SCSI_CMD_READ_CD\r\n");
			break;
	
		case SCSI_CMD_TEST_UNIT_READY:
		case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
		case SCSI_CMD_VERIFY_10:
		case SCSI_CMD_SET_CD_SPEED:
		case SCSI_CMD_SYNCHRONIZE_CACHE:
			/* These commands should just succeed, no handling required */
			CommandSuccess = true;
			MSInterfaceInfo->State.CommandBlock.DataTransferLength = 0;
			break;

		default:
			/* Update the SENSE key to reflect the invalid command */
			SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
		                   SCSI_ASENSE_INVALID_COMMAND,
		                   SCSI_ASENSEQ_NO_QUALIFIER);
			break;
	}

	/* Check if command was successfully processed */
	if (CommandSuccess)
	{
		SCSI_SET_SENSE(SCSI_SENSE_KEY_GOOD,
		               SCSI_ASENSE_NO_ADDITIONAL_INFORMATION,
		               SCSI_ASENSEQ_NO_QUALIFIER);
		return true;
	}

	return false;
	/*
	#ifdef CDROM
		return (SenseData_CD.SenseKey == SCSI_SENSE_KEY_GOOD);
	#else
		return (SenseData_MS.SenseKey == SCSI_SENSE_KEY_GOOD);
	#endif
	*/
}

/** 
 *	Command processing for an issued SCSI INQUIRY command. This command returns information about the device's features
 *  and capabilities to the host.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 *
 *  \return Boolean true if the command completed successfully, false otherwise.
 */
static bool SCSI_Command_Inquiry(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	uint16_t AllocationLength  = SwapEndian_16(*(uint16_t*)&MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]);
	#ifdef CDROM
		uint16_t BytesTransferred  = (AllocationLength < sizeof(InquiryData_CD))? AllocationLength :
									   sizeof(InquiryData_CD);
	#else
		uint16_t BytesTransferred  = (AllocationLength < sizeof(InquiryData_MS))? AllocationLength :
									   sizeof(InquiryData_MS);
	#endif

	/* Only the standard INQUIRY data is supported, check if any optional INQUIRY bits set */
	if ((MSInterfaceInfo->State.CommandBlock.SCSICommandData[1] & ((1 << 0) | (1 << 1))) ||
	     MSInterfaceInfo->State.CommandBlock.SCSICommandData[2])
	{
		/* Optional but unsupported bits set - update the SENSE key and fail the request */
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
		               SCSI_ASENSE_INVALID_FIELD_IN_CDB,
		               SCSI_ASENSEQ_NO_QUALIFIER);

		return false;
	}
	
	#ifdef CDROM
		Endpoint_Write_Stream_LE(&InquiryData_CD, BytesTransferred, NO_STREAM_CALLBACK);
	#else
		Endpoint_Write_Stream_LE(&InquiryData_MS, BytesTransferred, NO_STREAM_CALLBACK);
	#endif
	uint8_t PadBytes[AllocationLength - BytesTransferred];
	
	/* Pad out remaining bytes with 0x00 */
	Endpoint_Write_Stream_LE(&PadBytes, sizeof(PadBytes), NO_STREAM_CALLBACK);

	/* Finalize the stream transfer to send the last packet */
	Endpoint_ClearIN();

	/* Succeed the command and update the bytes transferred counter */
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= BytesTransferred;
	
	return true;
}

/** 
 *	Command processing for an issued SCSI REQUEST SENSE command. This command returns information about the last issued command,
 *  including the error code and additional error information so that the host can determine why a command failed to complete.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 *
 *  \return Boolean true if the command completed successfully, false otherwise.
 */
static bool SCSI_Command_Request_Sense(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	uint8_t  AllocationLength = MSInterfaceInfo->State.CommandBlock.SCSICommandData[4];
	#ifdef CDROM
		uint8_t  BytesTransferred = (AllocationLength < sizeof(SenseData_CD))? AllocationLength : sizeof(SenseData_CD);
	#else
		uint8_t  BytesTransferred = (AllocationLength < sizeof(SenseData_MS))? AllocationLength : sizeof(SenseData_MS);
	#endif
	uint8_t PadBytes[AllocationLength - BytesTransferred];

	#ifdef CDROM
		Endpoint_Write_Stream_LE(&SenseData_CD, BytesTransferred, NO_STREAM_CALLBACK);
	#else
		Endpoint_Write_Stream_LE(&SenseData_MS, BytesTransferred, NO_STREAM_CALLBACK);
	#endif

	Endpoint_Write_Stream_LE(&PadBytes, sizeof(PadBytes), NO_STREAM_CALLBACK);
	Endpoint_ClearIN();

	/* Succeed the command and update the bytes transferred counter */
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= BytesTransferred;

	return true;
}

/** 
 *	Command processing for an issued SCSI READ CAPACITY (10) command. This command returns information about the device's capacity
 *  on the selected Logical Unit (drive), as a number of OS-sized blocks.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 *
 *  \return Boolean true if the command completed successfully, false otherwise.
 */
static bool SCSI_Command_Read_Capacity_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
#ifdef CDROM
	uint32_t LastBlockAddressInLUN = (VIRTUAL_MEMORY_BYTES - 1);
#else
	uint32_t LastBlockAddressInLUN = (LUN_MEDIA_BLOCKS - 1);
#endif
	uint32_t MediaBlockSize        = VIRTUAL_MEMORY_BLOCK_SIZE;

	Endpoint_Write_Stream_BE(&LastBlockAddressInLUN, sizeof(LastBlockAddressInLUN), NO_STREAM_CALLBACK);
	Endpoint_Write_Stream_BE(&MediaBlockSize, sizeof(MediaBlockSize), NO_STREAM_CALLBACK);
	Endpoint_ClearIN();
	
	/* Succeed the command and update the bytes transferred counter */
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= 8;
	
	return true;
}

/** 
 *	Command processing for an issued SCSI SEND DIAGNOSTIC command. This command performs a quick check of the Dataflash ICs on the
 *  board, and indicates if they are present and functioning correctly. Only the Self-Test portion of the diagnostic command is
 *  supported.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 *
 *  \return Boolean true if the command completed successfully, false otherwise.
 */
static bool SCSI_Command_Send_Diagnostic(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	uint8_t ReturnByte;

	/* Check to see if the SELF TEST bit is not set */
	if (!(MSInterfaceInfo->State.CommandBlock.SCSICommandData[1] & (1 << 2)))
	{
		/* Only self-test supported - update SENSE key and fail the command */
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
		               SCSI_ASENSE_INVALID_FIELD_IN_CDB,
		               SCSI_ASENSEQ_NO_QUALIFIER);

		return false;
	}
		
	/* Succeed the command and update the bytes transferred counter */
	MSInterfaceInfo->State.CommandBlock.DataTransferLength = 0;
	
	return true;
}

/** 
 *	Command processing for an issued SCSI READ (10) or WRITE (10) command. This command reads in the block start address
 *  and total number of blocks to process, then calls the appropriate low-level Dataflash routine to handle the actual
 *  reading and writing of the data.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface structure that the command is associated with
 *  \param[in] IsDataRead  Indicates if the command is a READ (10) command or WRITE (10) command (DATA_READ or DATA_WRITE)
 *
 *  \return Boolean true if the command completed successfully, false otherwise.
 */
static bool SCSI_Command_ReadWrite_10(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                      const bool IsDataRead)
{
	uint32_t BlockAddress;
	uint16_t TotalBlocks;
	
	/* Load in the 32-bit block address (SCSI uses big-endian, so have to reverse the byte order) */
	BlockAddress = SwapEndian_32(*(uint32_t*)&MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]);

	/* Load in the 16-bit total blocks (SCSI uses big-endian, so have to reverse the byte order) */
	TotalBlocks  = SwapEndian_16(*(uint16_t*)&MSInterfaceInfo->State.CommandBlock.SCSICommandData[7]);
	/* Check if the block address is outside the maximum allowable value for the LUN */
	if (BlockAddress >= LUN_MEDIA_BLOCKS)
	{
		/* Block address is invalid, update SENSE key and return command fail */
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
		               SCSI_ASENSE_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE,
		               SCSI_ASENSEQ_NO_QUALIFIER);
		
		//Serial_TxString("D;0;Out of range;");
		return false;
	}

	#if (TOTAL_LUNS > 1)
	/* Adjust the given block address to the real media address based on the selected LUN */
	BlockAddress += ((uint32_t)MSInterfaceInfo->State.CommandBlock.LUN * LUN_MEDIA_BLOCKS);
	#endif

	#ifdef CDROM
	/* Determine if the packet is a READ (10) or WRITE (10) command, call appropriate function */
	if (IsDataRead == DATA_READ)
	{
		DataflashManager_ReadBlocks(MSInterfaceInfo, BlockAddress * 4, TotalBlocks * 4);
	}
	else
	{
		DataflashManager_WriteBlocks(MSInterfaceInfo, BlockAddress * 4, TotalBlocks * 4);
	}
	#else
	/* Determine if the packet is a READ (10) or WRITE (10) command, call appropriate function */
	if (IsDataRead == DATA_READ)
	{
		DataflashManager_ReadBlocks(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
	else
	{
		DataflashManager_WriteBlocks(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
	
	#endif
	/* Update the bytes transferred counter and succeed the command */
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= ((uint32_t)TotalBlocks * VIRTUAL_MEMORY_BLOCK_SIZE);
	
	return true;
}

/**
 *	Command processing for SCSI Get Configuration (0x46)
 */
static void SCSI_Command_Get_Config(USB_ClassInfo_MS_Device_t* MSInterfaceInfo)
{
	// Test for "Profile List Feature"
	if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00 && 
		MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x00)
	{
		char GetConfig[8];
		
		GetConfig[0] =0x00;
		GetConfig[1] =0x00;
		GetConfig[2] =0x00;
		GetConfig[3] =0x04;
		GetConfig[4] =0x00;
		GetConfig[5] =0x08;
		GetConfig[6] =0x01;
		GetConfig[7] =0x00;
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();

		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Test for "Logical Unit serial number"
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 &&
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x01 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x08)
	{
		char GetConfig[8];
		
		GetConfig[0] =0x01;
		GetConfig[1] =0x08;
		GetConfig[2] =0x00;
		GetConfig[3] =0x04;
		GetConfig[4] =0x00;
		GetConfig[5] =0xa8;
		GetConfig[6] =0x55;
		GetConfig[7] =0xa4;
	
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Test for "Removable Medium Feature"
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 &&
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x03)
	{
		char GetConfig[8];
		
		GetConfig[0] =0x00;
		GetConfig[1] =0x03;
		GetConfig[2] =0x03;//reserved2,version4,persistant1,current1 (00000111 req) //03
		GetConfig[3] =0x04;
		GetConfig[4] =0x28;//loading mech3,reserved1,eject1,pvnt jmpr1,reserved1,lock1
		GetConfig[5] =0x00;
		GetConfig[6] =0x00;
		GetConfig[7] =0x00;
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
	
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Test for "Multi-Read Feature"
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x1d)
	{
		char GetConfig[4];

		GetConfig[0] =0x00;
		GetConfig[1] =0x1d;
		GetConfig[2] =0x00; //  bit(0) current =0, bit(1) persistant =0
		GetConfig[3] =0x00;
		
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Test for "CD Read Feature"
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x1e)
	{
		char GetConfig[8];
	
		GetConfig[0] =0x00;
		GetConfig[1] =0x1e;
		GetConfig[2] =0x07; // bit 0 current(1),bit 1 persistant (0), bit 5-2 version (0001)
		GetConfig[3] =0x04;
		GetConfig[4] =0x01; // bit 0 is for format 5h? of read toc CD-Text (1)
		GetConfig[5] =0x00;
		GetConfig[6] =0x00;
		GetConfig[7] =0x00;

		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Test for "DVD Read Feature"
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[3]==0x1f)
	{
		char GetConfig[4];
	
		GetConfig[0] =0x00;
		GetConfig[1] =0x1f;
		GetConfig[2] =0x00;//  bit(0) current =0, bit(1) persistant =0
		GetConfig[3] =0x04;
		
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
	// Command not supported
	else 
	{
		char GetConfig[4];
		
		GetConfig[0] =0x00;
		GetConfig[1] =0x00;
		GetConfig[2] =0x00;
		GetConfig[3] =0x00;
		
		Endpoint_Write_Stream_LE(&GetConfig, sizeof(GetConfig), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();

		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(GetConfig);
	}
}

/**
 * Command processing for SCSI Read TOC(0x43)
 */
static void SCSI_Command_Read_TOC(USB_ClassInfo_MS_Device_t* MSInterfaceInfo)
{
	uint32_t CD_Max_Size=VIRTUAL_MEMORY_BYTES;//ISOFileDetails(1);
	
	// TOC Read Responce data for single track cdrom/ISO file. LBA / Format 0000
	if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x00 && 
		MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00)
	{
		char TOCRet[20];
		
		//TOC Responce Header
		TOCRet[0] =0x00;		//toc data length	msb 					18 bytes follow
		TOCRet[1] =0x12;		//toc data length	lsb
		TOCRet[2] =0x01;		//first track number						1st track
		TOCRet[3] =0x01;		//last track number							same as first, only 1 track total
		//track 1 info
		TOCRet[4] =0x00;		//reserved
		TOCRet[5] =0x14;		//4bit adr  4bit control	CHANGED ADR TO 0 (NO SUB DATA) was 1 (all sub data)
		TOCRet[6] =0x01;		//tract number								1st track
		TOCRet[7] =0x00;		//reserved
		TOCRet[8] =0x00;		//byte1	msb start track lba					lba of start of track
		TOCRet[9] =0x00;		//byte2
		TOCRet[10]=0x00;
		TOCRet[11]=0x00;		//byte4 lsb 
		//lead out info
		TOCRet[12]=0x00;		//reserved
		TOCRet[13]=0x14;		//4bit adr  4bit control	CHANGED ADR TO 0 (NO SUB DATA) was 1 (all sub data)
		TOCRet[14]=0xaa;		//tract number								AA=lead out track
		TOCRet[15]=0x00;		//reserved
		TOCRet[16]=CD_Max_Size>>24;	//lead out msb
		TOCRet[17]=CD_Max_Size>>16;	//lead out
		TOCRet[18]=CD_Max_Size>>8;	//lead out
		TOCRet[19]=CD_Max_Size;		//lead out lsb
		
		if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[8]==0x04)
		{
			Endpoint_Write_Stream_LE(&TOCRet, 4, NO_STREAM_CALLBACK);
			Endpoint_ClearIN();

			// Succeed the command and update the bytes transferred counter 
			MSInterfaceInfo->State.CommandBlock.DataTransferLength -= 4;
		}
		else
		{
			Endpoint_Write_Stream_LE(&TOCRet, sizeof(TOCRet), NO_STREAM_CALLBACK);
			Endpoint_ClearIN();

			// Succeed the command and update the bytes transferred counter 
			MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(TOCRet);
		}
	}
	// TOC Read Responce data for single track cdrom/ISO files. MSF
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x00)
	{
		char TOCRet[20];
		
		//TOC Responce Header
		TOCRet[0] =0x00;		//toc data length	msb 					18 bytes follow
		TOCRet[1] =0x12;		//toc data length	lsb
		TOCRet[2] =0x01;		//first track number						1st track
		TOCRet[3] =0x01;		//last track number							same as first, only 1 track total
		//track 1 info
		TOCRet[4] =0x00;		//reserved
		TOCRet[5] =0x04;		//4bit adr  4bit control	CHANGED ADR TO 0 (NO SUB DATA) was 1 (all sub data)
		TOCRet[6] =0x01;		//tract number								1st track
		TOCRet[7] =0x00;		//reserved
		TOCRet[8] =0x00;		//byte1	msb start track lba					lba of start of track
		TOCRet[9] =0x00;		//byte2
		TOCRet[10]=0x00;
		TOCRet[11]=0x00;		//byte4 lsb 
		//lead out info
		TOCRet[12]=0x00;		//reserved
		TOCRet[13]=0x04;		//4bit adr  4bit control	CHANGED ADR TO 0 (NO SUB DATA) was 1 (all sub data)
		TOCRet[14]=0xaa;		//tract number								AA=lead out track
		TOCRet[15]=0x00;		//reserved
		TOCRet[16]=0x00;		//lead out msb
		TOCRet[17]=0x59;		//lead out
		TOCRet[18]=0x3b;		//lead out
		TOCRet[19]=0x4f;		//lead out lsb
		Endpoint_Write_Stream_LE(&TOCRet, sizeof(TOCRet), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(TOCRet);
	}
	// TOC Read Responce data for single track cdrom/ISO files. LBA / Format 0001
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x00 && 
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x01)
	{
		char TOCRet[12];
		
		//TOC Responce Header
		TOCRet[0] =0x00;			//toc data length	msb 					18 bytes follow
		TOCRet[1] =0x0a;			//toc data length	lsb
		TOCRet[2] =0x01;			//first track number						1st track
		TOCRet[3] =0x01;			//last track number							same as first, only 1 track total
		//track 1 info
		TOCRet[4] =0x00;			//reserved
		TOCRet[5] =0x04;			//4bit adr  4bit control	CHANGED ADR TO 0 (NO SUB DATA) was 1 (all sub data)
		TOCRet[6] =0x01;			//tract number								1st track
		TOCRet[7] =0x00;			//reserved

		TOCRet[8] =0x00;			//byte1	msb start track lba					lba of start of track
		TOCRet[9] =0x00;			//byte2
		TOCRet[10]=0x00;			//byte3
		TOCRet[11]=0x00;			//byte4 lsb 
		
		Endpoint_Write_Stream_LE(&TOCRet, sizeof(TOCRet), NO_STREAM_CALLBACK);
		Endpoint_ClearIN();
		
		// Succeed the command and update the bytes transferred counter 
		MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(TOCRet);
	}
	else if (MSInterfaceInfo->State.CommandBlock.SCSICommandData[1]==0x02 &&
			 MSInterfaceInfo->State.CommandBlock.SCSICommandData[2]==0x02)
	{
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
		               SCSI_ASENSE_INVALID_FIELD_IN_CDB,
		               SCSI_ASENSEQ_NO_QUALIFIER);

	}
	else
	{
		// Do nothing for exception case
	}
}

/**
 * Command processing for SCSI Mode Sense (0x55)
 */
static void SCSI_Command_Mode_Sense(USB_ClassInfo_MS_Device_t* MSInterfaceInfo)
{

}

/**
 * Command processing for SCSI Mode Sense(10) (0x5A)
 */
static void SCSI_Command_Mode_Sense_10(USB_ClassInfo_MS_Device_t* MSInterfaceInfo)
{
	char ModeSenseHeader[8];

	ModeSenseHeader[0]=0x00;
	ModeSenseHeader[1]=0x00;
	ModeSenseHeader[2]=0x00;
	ModeSenseHeader[3]=0x12;
	ModeSenseHeader[4]=0x00;
	ModeSenseHeader[5]=0x00;
	ModeSenseHeader[6]=0x00;
	ModeSenseHeader[7]=0x00;
	
	char ModeSense_10_2A[20];
	
	//Mode Sense 2A Code Page Return, Mech and Capabilities
	ModeSense_10_2A[0] =0x2a;
	ModeSense_10_2A[1] =0x12;
	ModeSense_10_2A[2] =0x00;
	ModeSense_10_2A[3] =0x00;
	ModeSense_10_2A[4] =0x70;
	ModeSense_10_2A[5] =0x00;
	ModeSense_10_2A[6] =0x00;
	ModeSense_10_2A[7] =0x00;
	ModeSense_10_2A[8] =0x04;
	ModeSense_10_2A[9] =0x20;
	ModeSense_10_2A[10]=0x01;
	ModeSense_10_2A[11]=0x00;
	ModeSense_10_2A[12]=0x00;
	ModeSense_10_2A[13]=0x00;
	ModeSense_10_2A[14]=0x04;
	ModeSense_10_2A[15]=0x20;
	ModeSense_10_2A[16]=0x02;
	ModeSense_10_2A[17]=0x00;
	ModeSense_10_2A[18]=0x00;
	ModeSense_10_2A[19]=0x00;
	
	Endpoint_Write_Stream_LE(&ModeSenseHeader, sizeof(ModeSenseHeader), NO_STREAM_CALLBACK);
	Endpoint_Write_Stream_LE(&ModeSense_10_2A, sizeof(ModeSense_10_2A), NO_STREAM_CALLBACK);
	Endpoint_ClearIN();

	// Succeed the command and update the bytes transferred counter 
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(ModeSense_10_2A)+sizeof(ModeSenseHeader);
}

/**
 *	Command processing for SCSI READ DISK INFO (0x51)
 */
static void SCSI_Command_Read_Disk_Info(USB_ClassInfo_MS_Device_t* MSInterfaceInfo)
{
	char ReadDiskInfo[32];
	
	//Mode Sense 2A Code Page Return, Mech and Capabilities
	ReadDiskInfo[0] =0x00;
	ReadDiskInfo[1] =0x20;
	ReadDiskInfo[2] =0x0e;
	ReadDiskInfo[3] =0x01;
	ReadDiskInfo[4] =0x01;
	ReadDiskInfo[5] =0x01;
	ReadDiskInfo[6] =0x01;
	ReadDiskInfo[7] =0x80;
	ReadDiskInfo[8] =0x00;
	ReadDiskInfo[9] =0x00;
	ReadDiskInfo[10]=0x00;
	ReadDiskInfo[11]=0x00;
	ReadDiskInfo[12]=0x00;
	ReadDiskInfo[13]=0x00;
	ReadDiskInfo[14]=0x00;
	ReadDiskInfo[15]=0x00;
	ReadDiskInfo[16]=0xff;
	ReadDiskInfo[17]=0xff;
	ReadDiskInfo[18]=0xff;
	ReadDiskInfo[19]=0xff;
	ReadDiskInfo[20]=0xff;
	ReadDiskInfo[21]=0xff;
	ReadDiskInfo[22]=0xff;
	ReadDiskInfo[23]=0xff;
	ReadDiskInfo[24]=0x00;
	ReadDiskInfo[25]=0x00;
	ReadDiskInfo[26]=0x00;
	ReadDiskInfo[27]=0x00;
	ReadDiskInfo[28]=0x00;
	ReadDiskInfo[29]=0x00;
	ReadDiskInfo[30]=0x00;
	ReadDiskInfo[31]=0x00;

	Endpoint_Write_Stream_LE(&ReadDiskInfo, sizeof(ReadDiskInfo), NO_STREAM_CALLBACK);
	Endpoint_ClearIN();

	// Succeed the command and update the bytes transferred counter 
	MSInterfaceInfo->State.CommandBlock.DataTransferLength -= sizeof(ReadDiskInfo);
}


/** 
 *	Developing...
 * 	Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board Dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the Dataflash in Dataflash page sized blocks.
 *
 *  \param[in] MSInterfaceInfo  Pointer to a structure containing a Mass Storage Class configuration and state
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
void DataflashManager_WriteBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                  const uint32_t BlockAddress,
                                  uint16_t TotalBlocks)
{
	//char vcSendBuffer[100];
	int iSectorIndex = 0;
	char cDummy;
	int i;

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
		return;
	
	while (TotalBlocks)
	{
		// Send ATMEGA32 to Serial Message
		//sprintf(vcSendBuffer, "W;%d;;",iSectorIndex + BlockAddress); 
		//Serial_TxString(vcSendBuffer);
	
		for (i = 0 ; i < 512 ; i++)
		{
			/* Check if the endpoint is currently empty */
			if (!(Endpoint_IsReadWriteAllowed()))
			{
				/* Clear the current endpoint bank */
				Endpoint_ClearOUT();
				
				/* Wait until the host has sent another packet */
				if (Endpoint_WaitUntilReady())
					return;
			}

			// Write to Serial(ATMEGA16u2 -> ATMEGA32)
			//Serial_TxByte(Endpoint_Read_Byte());
			cDummy = Endpoint_Read_Byte();
			
			/* Check if the current command is being aborted by the host */
			if (MSInterfaceInfo->State.IsMassStoreReset)
			{
				//Serial_TxByte(0x00);
				break;
			}
		}

		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
			return;			
	
		/* Decrement the blocks remaining counter and reset the sub block counter */
		TotalBlocks--;
		iSectorIndex++;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
		Endpoint_ClearOUT();
}

/** 
 *	Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board Dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] MSInterfaceInfo  Pointer to a structure containing a Mass Storage Class configuration and state
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */
char g_vcSectorBuffer[512];

void DataflashManager_ReadBlocks(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo,
                                 const uint32_t BlockAddress,
                                 uint16_t TotalBlocks)
{
	int iSectorIndex = 0;
	char cDummy;
	int i;
	int j;

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
		return;
	
	//Serial_TxString("D;0;RS;");
	
	while (TotalBlocks)
	{		
		// Send ATMEGA32 to Serial Message
		//sprintf(vcRecvBuffer, "R;%ld;;", iSectorIndex + BlockAddress); 
		//Serial_TxString(vcRecvBuffer);
		//sprintf(vcRecvBuffer, "%ld;;", iSectorIndex + BlockAddress); 
		
		//cli();
		//MyItoA(vcRecvBuffer, (int)(iSectorIndex + BlockAddress));
		//Serial_TxString("R;");
		//Serial_TxString(vcRecvBuffer);
		//Serial_TxString(";;");
	
		for (j = 0 ; j < 512 / sizeof(g_vcSectorBuffer); j++)
		{
			//for (i = 0 ; i < sizeof(vcRecvBuffer) ; i++)
			//{
			//	cDummy = Serial_RxByte();
			//	vcRecvBuffer[i] = cDummy;
			//}
			GetSectorData((int)iSectorIndex + BlockAddress);
			//GetSectorData(((int)100000000), g_vcSectorBuffer);
			
			/* Check if the current command is being aborted by the host */
			if (MSInterfaceInfo->State.IsMassStoreReset)
			{
				//Serial_TxString("D;0;Reset;");
				continue;
			}
	
			for (i = 0 ; i < sizeof(g_vcSectorBuffer) ; i++)
			{
				/* Check if the endpoint is currently full */
				if (!(Endpoint_IsReadWriteAllowed()))
				{
					/* Clear the endpoint bank to send its contents to the host */
					Endpoint_ClearIN();
					
					/* Wait until the endpoint is ready for more data */
					if (Endpoint_WaitUntilReady())
					{
						//Serial_TxString("D;0;Full;");
						break;
					}
				}
				
				// Read from Serial(ATMEGA32 -> ATMEGA16u2)
				Endpoint_Write_Byte(g_vcSectorBuffer[i]);

				/* Check if the current command is being aborted by the host */
				if (MSInterfaceInfo->State.IsMassStoreReset)
				{
					break;
				}
			}
		}
		//sei();
		
		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
		{
			//Serial_TxString("D;0;Reset;");
			return;
		}
		
		/* Decrement the blocks remaining counter */
		TotalBlocks--;
		iSectorIndex++;
	}

	//Serial_TxString("D;0;RE;");
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
		Endpoint_ClearIN();

	/* Deselect all Dataflash chips */
	//Dataflash_DeselectChip();
}
