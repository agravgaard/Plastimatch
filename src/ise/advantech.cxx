/* -----------------------------------------------------------------------
See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
----------------------------------------------------------------------- */
#include "ise_config.h"
#include <string>
#include <stdio.h>
#include <windows.h>
#include "advantech.h"
#include "aqprintf.h"
#include "acquire_4030e_define.h"

/* Card is Advantech PCI-1760U */
/* USB box is Advantech USB-4761 */
#include "driver.h"  /* Advantech */
#include "os.h"      /* Advantech */

#if (ADVANTECH_FOUND)
static PT_DioWritePortByte ptDioWritePortByte;
static PT_DioReadPortByte ptDioReadPortByte;
#endif /* ADVANTECH_FOUND */

Advantech::Advantech ()
{
	this->have_device = false;
	this->device_number = 0;	
	m_bOpenSuccess = false;
}

int Advantech::Init()
{
	LRESULT rc;
	SHORT num_devices;

	//int initResult = 0; // 0 = No Error

	/* Find number of devices */
	rc = DRV_DeviceGetNumOfList(&num_devices);
	if (rc != SUCCESS) {
		aqprintf ("Error enumerating Advantech devices (1)\n");
		Advantech::print_error (rc);
		return ADVANTECH_DRV_GET_NUM_ERROR;
		//exit (-1);
	}
	aqprintf ("Advantech: %d devices found.\n", (int) num_devices);

	/* Retrieve device list */
	DEVLIST* device_list = (DEVLIST*) malloc (num_devices * sizeof(DEVLIST));
	SHORT num_out_entries;
	rc = DRV_DeviceGetList (device_list, num_devices, &num_out_entries);
	if (rc != SUCCESS) {
		aqprintf ("Error enumerating Advantech devices (2)\n");
		Advantech::print_error (rc);
		return ADVANTECH_DRV_GET_LIST_ERROR;
		//exit (-1);
	}

	/* Find the USB device */
	for (int i = 0; i < num_devices; i++) {
		std::string device_name = device_list[i].szDeviceName;
		std::string usb_device_name ("USB-4761");
		aqprintf ("Advantech device %2d: %d, %s\n", 
			i, device_list[i].dwDeviceNum, device_list[i].szDeviceName);
		if (!device_name.compare (0, usb_device_name.size(), usb_device_name))
		{
			if (!this->have_device) {
				aqprintf ("  >> Trying to connect to USB-4761 at device %d\n", 
					device_list[i].dwDeviceNum);
				this->have_device = true;
				this->device_number = device_list[i].dwDeviceNum;
			}
		}
	}

	if (!this->have_device) {
		aqprintf ("Sorry, no USB device found\n");
		return ADVANTECH_FIND_USB_ERROR;
		//exit (-1);
	}

	/* Open the device */
	rc = DRV_DeviceOpen (this->device_number, &this->driver_handle);
	if (rc != SUCCESS) {
		this->have_device = false;
		aqprintf ("Error opening Advantech device\n");
		Advantech::print_error (rc);
		if (rc == 8193) {
			aqprintf ("USB device not connected?\n");
		}
		return ADVANTECH_DRV_OPEN_ERROR;
		//exit (-1);
	}
	aqprintf ("Connected to USB-4761.\n");


	m_bOpenSuccess = true;
	return ADVANTECH_NO_ERROR;
}

Advantech::~Advantech ()
{
	if (m_bOpenSuccess)
	{
		if (this->have_device) {
			DRV_DeviceClose (&this->driver_handle);
		}
	}	
}

void Advantech::relay_close (int bit)
{
	if (!m_bOpenSuccess)
	{
		return;
	}


	LRESULT rc;
	PT_DioWriteBit ptDioWriteBit;
	ptDioWriteBit.port  = 0;
	ptDioWriteBit.bit   = bit;
	ptDioWriteBit.state = 1;
	rc = DRV_DioWriteBit(
		this->driver_handle, (LPT_DioWriteBit)&ptDioWriteBit);
}

void 
Advantech::relay_open (int bit)
{
	if (!m_bOpenSuccess)
	{
		return;
	}

	LRESULT rc;
	PT_DioWriteBit ptDioWriteBit;
	ptDioWriteBit.port  = 0;
	ptDioWriteBit.bit   = bit;
	ptDioWriteBit.state = 0;
	rc = DRV_DioWriteBit (
		this->driver_handle, (LPT_DioWriteBit) &ptDioWriteBit);
}

//bool Advantech::read_bit (int bit)
int Advantech::read_bit (int bit)
{
	if (!m_bOpenSuccess)
	{
		return -1;
	}

	int result;

	LRESULT rc;
	PT_DioReadBit ptDioReadBit;
	USHORT state = 0;
	ptDioReadBit.port  = 0;
	ptDioReadBit.bit   = bit;
	ptDioReadBit.state = &state;

	rc = DRV_DioReadBit (
		this->driver_handle, (LPT_DioReadBit) &ptDioReadBit);

	result = (int)state;

	if (rc != SUCCESS)
	{
		aqprintf ("Error reading bit on Advantech device\n");
		Advantech::print_error (rc);
		result = STATE_ERROR;
		//exit (-1);
	}
	return result;
}

void 
Advantech::print_error (LRESULT ErrorCode)
{
	char error_msg[80];
	DRV_GetErrorMessage (ErrorCode, error_msg);
	aqprintf ("Error %d: %s\n", ErrorCode, error_msg);
}
