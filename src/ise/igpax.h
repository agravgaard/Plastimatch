/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __igpax_h__
#define __igpax_h__

#define IGPAX_SUCCESS			    0
#define IGPAX_NO_RADIOGRAPH_AUTOSENSED	    1
#define IGPAX_VIP_GET_IMAGE_FAILED	    2
#define IGPAX_VIP_SW_VALID_XRAYS_0	    3
#define IGPAX_VIP_SW_VALID_XRAYS_1	    4
#define IGPAX_VIP_OPEN_LINK		    5
#define IGPAX_VIP_RESET_STATE		    6
#define IGPAX_VIP_SELECT_MODE		    7
#define IGPAX_VIP_SW_PREPARE_0		    8
#define IGPAX_VIP_SW_PREPARE_1		    9
#define IGPAX_VIP_OFFSET_CAL		    10
#define IGPAX_VIP_GAIN_CAL		    11
#define IGPAX_UNCLASSIFIED		    105

#define IGPAXCMD_TEST_PIPES		    'a'
#define IGPAXCMD_CLEAR_CORRECTIONS	    'c'
#define IGPAXCMD_SET_1FPS		    'f'
#define IGPAXCMD_START_GRABBING		    'g'
#define IGPAXCMD_SET_7_5FPS		    'h'
#define IGPAXCMD_INIT			    'i'
#define IGPAXCMD_IGTALK2_CORRECTIONS	    'l'
#define IGPAXCMD_GET_IMAGE		    'm'
#define IGPAXCMD_OFFSET_CAL		    'o'
#define IGPAXCMD_QUIT			    'q'
#define IGPAXCMD_MODE_0			    '0'
#define IGPAXCMD_MODE_1			    '1'
#define IGPAXCMD_MODE_2			    '2'


#endif
