/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _advantech_h_
#define _advantech_h_

#include "ise_config.h"
#include <windows.h>


class Advantech {

public:
	enum STATERETURN{
		STATE_0 = 0,
		STATE_1,
		STATE_ERROR //2
	};
	
public:
    Advantech ();
    ~Advantech ();
public:
    bool have_device;
    ULONG device_number;
    LONG driver_handle;
	bool m_bOpenSuccess;
public:
	int Advantech::Init();
    void relay_open (int bit);
    void relay_close (int bit);
    static void print_error (LRESULT ErrorCode);
    int read_bit (int bit);
};

#endif
