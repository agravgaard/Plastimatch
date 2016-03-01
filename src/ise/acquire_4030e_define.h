#pragma once

enum Generator_state {
	WAITING,
	EXPOSE_REQUEST,
	EXPOSING
};

enum CommandToChild {	
	PCOMMAND_KILL,	
	PCOMMAND_RESTART,	
	PCOMMAND_SHOWDLG,
	//PCOMMAND_ACTIVATE,
	PCOMMAND_UNLOCKFORPREPARE,
	PCOMMAND_CANCELACQ,
	PCOMMAND_DUMMY
};


enum AdvantechError {
	ADVANTECH_NO_ERROR = 0,
	ADVANTECH_DRV_GET_NUM_ERROR,
	ADVANTECH_DRV_GET_LIST_ERROR,
	ADVANTECH_FIND_USB_ERROR,
	ADVANTECH_DRV_OPEN_ERROR 	
};



typedef unsigned short USHORT;

#define MAX_CHECK_LINK_RETRY 3
#define RESTART_NEEDED -101
#define EXTERNAL_STATUS_CHANGE -102
#define DELAY_FOR_CHILD_RESPONSE 300

#define MAX_LINE_LENGTH 1024

#define DEFAULT_LOGFILE_PATH "C:\\"
#define MAX_PANEL_NUM_YK 2


enum PSTAT{	
	NOT_OPENNED,	
	OPENNED, //after openning, go to select receptor, vip_io_enable(active)
	PANEL_ACTIVE,	
	READY_FOR_PULSE,//print "ERADY for X-ray and go to wait-on-num-pulses
	PULSE_CHANGE_DETECTED, //beam signal detected
	COMPLETE_SIGNAL_DETECTED,
	IMAGE_ACQUSITION_DONE,
	STANDBY_CALLED,
	STANDBY_SIGNAL_DETECTED,
	ACQUIRING_DARK_IMAGE,
	DUMMY
};

enum Label_style {
	LABEL_NOT_READY,
	LABEL_ACQUIRING, //Yellow
	LABEL_PREPARING, //Orange
	LABEL_STANDBY, //Orange
	LABEL_READY
};


struct BADPIXELMAP{
	int BadPixX;
	int BadPixY;
	int ReplPixX;
	int ReplPixY;
};