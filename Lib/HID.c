/*
Copyright (c) 2014 NicoHood
See the readme for credit to other people.

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

#include "HID.h"

/** LUFA HID Class driver interface configuration and state information. This structure is
*  passed to all HID Class driver functions, so that multiple instances of the same class
*  within a device can be differentiated from one another.
*/
USB_ClassInfo_HID_Device_t Device_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber = INTERFACE_ID_HID,
		.ReportINEndpoint =
		{
			.Address = HID_IN_EPADDR,
			.Size = HID_EPSIZE,
			.Banks = 1,
		},
		.PrevReportINBuffer = NULL, // we dont cache anything
		.PrevReportINBufferSize = sizeof(HID_HIDReport_Data_t),
	},
};


void resetNHPbuffer(void){
	ram.NHP.readlength = 0;
	ram.NHP.mBlocks = 0;
}


void clearHIDReports(void){
	// dont do anything if the main flag is empty
	if (ram.HID.isEmpty[HID_REPORTID_NotAReport]) return;

	// check if every report is empty or not
	for (int i = 1; i < HID_REPORTID_LastNotAReport; i++){

		if (!ram.HID.isEmpty[i])
			clearHIDReport(i);
	}
	// clear the flag that >0 reports were set
	ram.HID.isEmpty[HID_REPORTID_NotAReport] = true;
}

void flushHID(void){
	// try to send until its done
	while (ram.HID.ID && ram.HID.length == ram.HID.recvlength)
		HID_Device_USBTask(&Device_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
*
*  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
*  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
*  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
*  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
*  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
*
*  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
*/
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
	uint8_t* const ReportID,
	const uint8_t ReportType,
	void* ReportData,
	uint16_t* const ReportSize)
{
	// only send report if there is actually a new report
	if (ram.HID.ID && ram.HID.length == ram.HID.recvlength){
		// set a general and specific flag that a report was made, ignore rawHID
		if (ram.HID.ID != HID_REPORTID_RawKeyboardReport){
			ram.HID.isEmpty[HID_REPORTID_NotAReport] = true;
			ram.HID.isEmpty[ram.HID.ID] = true;
		}

		//write report and reset ID
		memcpy(ReportData, ram.HID.buffer, ram.HID.length);
		*ReportID = ram.HID.ID;
		*ReportSize = ram.HID.length;
		ram.HID.ID = 0;
		ram.HID.recvlength = 0; //just to be sure if you call HID_Task by accident again
		ram.HID.length = 0; //just to be sure if you call HID_Task by accident again

		// always return true, because we cannot compare with >1 report due to ram limit
		// this will forcewrite the report every time
		return true;
	}
	else return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
*
*  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
*  \param[in] ReportID    Report ID of the received report from the host
*  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
*  \param[in] ReportData  Pointer to a buffer where the received report has been stored
*  \param[in] ReportSize  Size in bytes of the received HID report
*/
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
	const uint8_t ReportID,
	const uint8_t ReportType,
	const void* ReportData,
	const uint16_t ReportSize)
{
	// Unused in this demo, since there are no Host->Device reports
	//	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (ReportID == HID_REPORTID_RawKeyboardReport){
		//LEDs_SetAllLEDs(LEDS_ALL_LEDS);
		//while (1); //TODO remove <--

		// Turn on RX LED
		LEDs_TurnOnLEDs(LEDMASK_RX);
		ram.PulseMSRemaining.RxLEDPulse = TX_RX_LED_PULSE_MS;

		// Send bytes
		Serial_SendData(ReportData, ReportSize);
	}
}


void clearHIDReport(uint8_t ID){
	// RAW HID cannot be cleared
	if (ID == HID_REPORTID_RawKeyboardReport) return;

	// we have a pending HID report, flush it first
	flushHID();

	// get length of the report if its a valid report
	uint8_t length = getHIDReportLength(ID);
	if (!length) return;

	// save new values and prepare for sending
	ram.HID.length = ram.HID.recvlength = length;
	ram.HID.ID = ID;
	memset(&ram.HID.buffer, 0x00, length);

	// flush HID
	flushHID();

	// save new empty state
	ram.HID.isEmpty[ID] = true;
}

uint8_t getHIDReportLength(uint8_t ID){
	// Get the length of the report
	switch (ram.HID.ID){
	case HID_REPORTID_MouseReport:
		return sizeof(HID_MouseReport_Data_t);
		break;

	case HID_REPORTID_KeyboardReport:
		return sizeof(HID_KeyboardReport_Data_t);
		break;

	case HID_REPORTID_RawKeyboardReport:
		return sizeof(HID_RawKeyboardReport_Data_t);
		break;

	case HID_REPORTID_MediaReport:
		return sizeof(HID_MediaReport_Data_t);
		break;

	case HID_REPORTID_SystemReport:
		return sizeof(HID_SystemReport_Data_t);
		break;

	case HID_REPORTID_Gamepad1Report:
	case HID_REPORTID_Gamepad2Report:
		return sizeof(HID_GamepadReport_Data_t);
		break;

	case HID_REPORTID_Joystick1Report:
	case HID_REPORTID_Joystick2Report:
		return sizeof(HID_JoystickReport_Data_t);
		break;

	default:
		// error, write down this wrong ID report
		return 0;
		break;
	} //end switch
	return 0;
}