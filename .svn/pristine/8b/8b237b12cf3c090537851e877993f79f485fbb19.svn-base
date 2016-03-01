/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/** @file bitflow.c
 *	@brief implementation of the wrapper functions to access the bitflow card.
 * 		
 *
 *  This contains the implementations of the bitflow wrapper functions
 *  for accessing the bitflow framegrabber card.
 *	
 *	@author	Rui Li
 *	@bug No know bugs
 */
#include "ise_config.h"
#if (BITFLOW_FOUND)
#include <stdio.h>
#include <conio.h>
#include "R2Api.h"
#include "BFApi.h"
#include "bitflow.h"
#include "debug.h"

// Allow a few repeating attempts on acquisition when overflow occured.
#define	ERR_LIMIT	3 


// These values work well for proton center's 4030CB -- maybe should move
// these to a configuration file
#define PCHZACTIVE 1024
#define PCVTACTIVE 768
#define PCTRIMX 8
#define PCTRIMY 5


//////////////////////////////////////////////////////////////////////////////
// SetCTabValue		--	Used by SetVideoTimingToGrabber
//////////////////////////////////////////////////////////////////////////////
// GCS Return 0 for failure
int SetCTabValue (RdRn hBoard, DWORD index, WORD mask)
{
    int i=0;
    int ok = 0;
    const int ctabLim = 0x2000;
    BFU32* data = (BFU32*) malloc (sizeof(BFU32) * ctabLim);
    R2RC ret;

    if(!data) return 0;
    memset(data, 0, ctabLim * sizeof(BFU32));
    ret = R2CTabRead (hBoard, 0, ctabLim, mask, data);
    if (ret == R2_OK)
    {
	while(i < ctabLim && data[i] != mask) i++;
	if(i < ctabLim)
	{
	    // zero out the existing entry
	    R2CTabFill(hBoard, i, 0x0001, mask, 0x0000);
	    // set the new value required
	    R2CTabFill(hBoard, index, 0x0001, mask, 0xFFFF);
	    ok = 1;
	}
    }
    if (data) free (data);
    return ok;
}

int SetVideoTimingToGrabber(RdRn hBoard, int hzActive, int vtActive, int trimX, int trimY, 
							char *errMsg)
{
    // initially was declared as a global variable
    // Rui changed it to a local variable as it is 
    // only used in this function.
    int gConTrim; 
    DWORD index;
    WORD mask;

    if(trimY < 0) trimY = 0;
    if(trimX < 0) trimX = 0;

    gConTrim = (trimX % 4);
    trimX /= 4;

    if (R2AqFrameSize (hBoard, hzActive, vtActive) != R2_OK)
    {
        printf ("Unable to set new frame size: %dx%d", hzActive, vtActive);
        return -1;
    }

    // first do the horizontal; note 1/4 pixel clock 
    index = 0x0800 + trimX;
    mask  = R2HCTabHStart;
    if (!SetCTabValue(hBoard,index, mask))
    {
        printf ("\nUnable to find Horizontal mask value\n");
        return -1;
    }

    index = 0x0800 + hzActive / 4 + trimX;
    mask = R2HCTabHStop + R2HCTabHStart;
    if (!SetCTabValue(hBoard,index, mask))
    {
        printf ("\nUnable to find Horizontal mask value\n");
        return -1;
    }

    // next do the vertical
    index = 0x1000 + trimY;
    mask = R2VCTabVStart;
    if(!SetCTabValue(hBoard,index, mask))
    {
        printf ("\nUnable to find Vertical mask value\n");
        return -1;
    }
    index = 0x1000 + vtActive + trimY;
    mask = R2VCTabVStop + R2VCTabVStart;
    if(!SetCTabValue(hBoard,index, mask))
    {
        printf ("\nUnable to find Vertical mask value\n");
        return -1;
    }
    return 0;	
}

Ise_Error bitflow_init (BitflowInfo* bf, unsigned int mode)
{
	//set it to null for clean start
	memset(bf, 0, sizeof(BitflowInfo));

	//in matrox_source, if the mode is fluoro, then it is
	//necessary to allocate resource to the application
	//identifiers. However, in bitflow, this is not nessary.
	//so we return ISE_SUCCESS irregardless of the mode
	return ISE_SUCCESS;
}

Ise_Error bitflow_open (BitflowInfo* bf, unsigned int idx, 
						unsigned int board_no, unsigned int mode,
						unsigned long fps)
{
    R2ENTRY entry;
    BFU32 modeGot;
    PBFU8 hostBuf0, hostBuf1;
	
    //find the board
    if (R2SysBoardFindByNum(board_no, &entry)) 
        return ISE_BITFLOW_FIND_BOARD_FAILED;

    //open the board
    if (R2BrdOpen(&entry, &bf->hBoard[idx], R2SysInitialize))
        return ISE_BITFLOW_BOARD_OPEN_FAILED,

    //request host based QTABs, this always works
    BFQTabModeRequest(bf->hBoard[idx], BFQTabModeHost, &modeGot);

    //obtain the image size information
    R2BrdInquire(bf->hBoard[idx], R2CamInqHostFrameSize, &bf->imageSize);
    R2BrdInquire(bf->hBoard[idx], R2CamInqXSize, &bf->sizeX);
    R2BrdInquire(bf->hBoard[idx], R2CamInqYSize, &bf->sizeY);
    R2BrdInquire(bf->hBoard[idx], R2CamInqPixBitDepth, &bf->bitDepth);

    //allocate host buffer, double buffer is used
    hostBuf0 = (PBFU8) malloc(bf->imageSize);
    if (hostBuf0 == NULL) 
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_MALLOC_FAILED;
    }
	
    hostBuf1 = (PBFU8) malloc(bf->imageSize);
    if (hostBuf1 == NULL)
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_MALLOC_FAILED;
    }

    bf->hostBuffer[idx][0] = hostBuf0;
    bf->hostBuffer[idx][1] = hostBuf1;
    bf->acqMode = 0;
    return ISE_SUCCESS;
}

Ise_Error bitflow_grab_setup (BitflowInfo *bf, int idx)
{
    PBFU8 hostBuf0, hostBuf1;
    int activeDimX = PCHZACTIVE;
    int activeDimY = PCVTACTIVE;
    int trimX = PCTRIMX;
    int trimY = PCTRIMY;

	// The current implementation only supports unsigned short
	// This limitation is from the implementation of the circular
	// buffer. A better way to do this is to obtain the information
	// from the board.
    if (bf->imageSize != bf->sizeX * bf->sizeY * bf->bitDepth / 8)
        return ISE_BITFLOW_BAD_IMAGE_SIZE;

    hostBuf0 = bf->hostBuffer[idx][0];
    hostBuf1 = bf->hostBuffer[idx][1];

    // Check to make sure there is memory being allocated. 
    // This is just a sanity check, the actual malloc is done
    // bitflow_init
    if (hostBuf0 == NULL) 
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_MALLOC_FAILED;
    }
    if (hostBuf1 == NULL) 
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_MALLOC_FAILED;
    }

    //create singal, used to tell when image is completely in memory	
    if (R2SignalCreate(bf->hBoard[idx], R2IntTypeQuadDone, &bf->Signal[idx]))
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_CREATE_SIG_FAILED;
    }

    //request host based QTABS
    BFQTabModeRequest(bf->hBoard[idx], BFQTabModeHost, &bf->gQTabMode[idx]);	

    // Set up for acquision to initial buffer
    if (R2AqSetup (bf->hBoard[idx], (PBFVOID)hostBuf0, bf->imageSize, 0, R2DMADataMem,
        R2LutBank0, R2Lut8Bit, R2QTabBank0, TRUE)) 
    {
        R2ErrorShow(bf->hBoard[idx]);
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_R2ACQ_SETUP_FAILED;
    }
    // Set up for acquision to additional buffer
    if (R2AqSetup (bf->hBoard[idx], (PBFVOID)hostBuf1, bf->imageSize, 0, R2DMADataMem,
        R2LutBank0, R2Lut8Bit, R2QTabBank1, FALSE)) 
    {
        R2ErrorShow(bf->hBoard[idx]);
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_R2ACQ_SETUP_FAILED;
    }

    // This is to make it work on 4030CB
    if (SetVideoTimingToGrabber(bf->hBoard[idx], activeDimX, activeDimY, trimX, trimY, 0))
        return ISE_BITFLOW_SET_VIDEO_TIMING_FAILED;

    bf->bank[idx] = 0;
    

    if (R2AqCommand(bf->hBoard[idx], R2ConGrab, R2ConWait, R2QTabBank0))
    {
        R2BrdClose(bf->hBoard[idx]);
        return ISE_BITFLOW_R2ACQ_CMD_FAILED;
    }
	
    // for host-based QTAB's, we don't need to set up next bank explicitly
    if (bf->gQTabMode[idx] == BFQTabModeBoard)
        R2AqNextBankSet(bf->hBoard[idx], R2QTabBank1);
    bf->acqMode = 1;

    return ISE_SUCCESS;
}

void bitflow_grab_image (unsigned short *img, BitflowInfo *bf, int idx)
{
        R2RC rv;
	BFU32 keepLooping = 0;
	BFU32 num;
	PBFU16 pNew;
	
	// wait for a frame done signal or 5000 msec, whichever comes first
	rv = R2SignalWait(bf->hBoard[idx], &bf->Signal[idx], INFINITE, &num);

        switch (rv) {
            case R2_OK:
                debug_printf("OK\n");
                break;
            case R2_SIGNAL_TIMEOUT:
                debug_printf("TIMEOUT\n");
                break;
            case R2_SIGNAL_CANCEL:
                debug_printf("CANCEL\n");
                break;
            case R2_BAD_SIGNAL:
                debug_printf("BAD\n");
                break;
            case R2_WAIT_FAILED:
                debug_printf("FAILED\n");
                break;
            default:
                debug_printf("nothing\n");
                break;
        }
	if (rv == R2_SIGNAL_TIMEOUT || rv != R2_OK)
	{
		printf("R2_SIGNAL_TIMEOUT, shutting down the acquisition process ...\n");
		bitflow_shutdown(bf, idx);	
		return;
	}

      debug_printf("grabbing ...\n");

	//set up to acquire to other bank -- not needed for host-based QTABs
	if (bf->gQTabMode[idx] == BFQTabModeBoard)
	{
		if (bf->bank[idx] = 0)
			R2AqNextBankSet(bf->hBoard[idx], R2QTabBank0);
		else
			R2AqNextBankSet(bf->hBoard[idx], R2QTabBank1);
	}

	//use the newest filled bank of data
	if (bf->bank[idx] == 0)
		pNew = (PBFU16) bf->hostBuffer[idx][0];
	else
		pNew = (PBFU16) bf->hostBuffer[idx][1];

	//copy to buff
	memcpy(img, pNew, bf->imageSize);
		
	//change bank
	bf->bank[idx] ^= 1;
}

Ise_Error bitflow_shutdown(BitflowInfo* bf, int num_panels)
{
    int idx;

    
    for (idx = 0; idx < num_panels; idx ++) {
        // has set up acquisition
        if (bf->acqMode == 1) {
	    // stop acquiring
	    if (R2AqCommand(bf->hBoard[idx], R2ConFreeze, R2ConWait, R2QTabBank0))
                return ISE_BITFLOW_FREEZE_FAILED;
    
            // clean up acquisition resources
            R2AqCleanUp(bf->hBoard[idx]);
        }

        // free buffers
        free(bf->hostBuffer[idx][0]);
        free(bf->hostBuffer[idx][1]);

        // close board
        R2BrdClose(bf->hBoard[idx]);
    }

    return ISE_SUCCESS;
}

int bitflow_probe(void)
{
    Ise_Error rc;
    BitflowInfo bitflow;
    unsigned int mode = ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO;
    if (bitflow_init (&bitflow, mode) != ISE_SUCCESS) 
        return 0;

    rc = bitflow_open (&bitflow, 0, 0, ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO, ISE_FRAMERATE_7_5_FPS);
    
    if (rc == ISE_SUCCESS) 
    {
        bitflow_clear_probe (&bitflow, 0);
        return 1;
    } 
    else 
        return 0;
}

void bitflow_clear_probe(BitflowInfo *bf, int idx)
{
	// free buffers
	free(bf->hostBuffer[idx][0]);
	free(bf->hostBuffer[idx][1]);

	// close board
	R2BrdClose(bf->hBoard[idx]);
}
#endif /* BITFLOW_FOUND */
