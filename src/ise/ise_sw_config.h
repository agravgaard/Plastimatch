/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ise_config_h__
#define __ise_config_h__

#define ISE_TRACKING_ENABLED 0
// #define ISE_TRACKING_ENABLED 1

//#define ISE_HAVE_HARDWARE   0
#define ISE_HAVE_HARDWARE   1

/* Number of frames for CBUF, for ISE */
//#define ISE_NUM_FRAMES           240
#define ISE_NUM_FRAMES           120

/* Number of CBUF frames for IGTALK2 */
#define IGTALK2_NUM_FRAMES       40


/* This is the original setup for image panels
 * 2 image panels were used
 */
/* #define ISE_CLIENT_IP_1     "192.168.1.2"
	#define ISE_SERVER_IP_1     "192.168.1.3"
	#define ISE_BOARD_1          0
	#define ISE_FLIP_1           2
	#define ISE_FRAMERATE_1      7.5

	#define ISE_CLIENT_IP_2     "192.168.1.4"
	#define ISE_SERVER_IP_2     "192.168.1.5"
	#define ISE_BOARD_2          1
	#define ISE_FLIP_2           3
	#define ISE_FRAMERATE_2      7.5
*/
/* The IP address is modified by RUI LI on 5/21
 * 1 image panel is used.
 */
	#define ISE_CLIENT_IP_1     "172.20.20.21"
	#define ISE_SERVER_IP_1     "172.20.20.20"
	#define ISE_BOARD_1          0
	#define ISE_FLIP_1           2
	#define ISE_FRAMERATE_1      7.5	
	#define ISE_CLIENT_IP_2     "172.20.20.20"
	#define ISE_SERVER_IP_2     "172.20.20.21"
	#define ISE_BOARD_2          1
	#define ISE_FLIP_2           3
	#define ISE_FRAMERATE_2      7.5
#endif



