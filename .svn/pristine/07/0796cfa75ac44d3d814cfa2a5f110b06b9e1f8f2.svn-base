/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/** @file bitflow.h
 *	@brief Function prototypes for the bitflow wrapper functions
 *
 *  This contains the prototypes for the bitflow wrapper functions
 *  for accessing the bitflow framegrabber card.
 *	
 *	@author	Rui Li
 *	@bug No know bugs
 */
#ifndef __BITFLOW_H__
#define __BITFLOW_H__

#include "ise_config.h"
#include "ise_error.h"
#include "ise_structs.h"

#if (BITFLOW_FOUND)

#if defined __cplusplus
extern "C" {
#endif

/** @brief Initialize the BitflowInfo struct
 *
 *	Reset the bitflow information to NULL
 *
 *	@param bt pointer to the BitflowInfo struct.
 *	@param model integer to indicate whether the system is in the fluoro mode.
 *	@return The error code listed in Ise_Error.
 */
Ise_Error bitflow_init (BitflowInfo* bf, unsigned int mode);

/** @brief Open the bitflow card
 *
 *	Opens the board for grabbing. This function must return 
 *	succesfully before any other R2 board SDK functions are called
 *
 *	@param bt pointer to the BitflowInfo struct.
 *	@param idx 
 *	@param board_no
 *	@param mode
 *	@param fps user set capturing device frame rate. However, bitflow has no 
 *			   supporting SDK function for this feature.
 *	@return The error code listed in Ise_Error.
 */
Ise_Error bitflow_open (BitflowInfo* bf, unsigned int idx, 
			unsigned int board_no, unsigned int mode,
			unsigned long fps);

/** @brief Set up the bitflow card for grabbing image
 *
 *	Set up the bitflow card for grabbing image after it has been opened
 *	
 *	@param bf pointer to the BitflowInfo struct.
 *	@param idx 
 *	@return The error code listed in Ise_Error.
 */

Ise_Error bitflow_grab_setup (BitflowInfo *bf, int idx);

/** @brief Grab image from the panel
 *
 *	Grab image from the panel and copy them into image buffer.
 *	
 *  @param img pointer to the image buffer.
 *	@param bf pointer to the BitflowInfo struct.
 *	@param idx integer (0/1) for the corresponding board
 *	@return Void
 */
void bitflow_grab_image (unsigned short* img, BitflowInfo *bf, int idx);

/** @brief Closes the board(s) and frees all associated resouces.
 * 
 *  Closes all boards that are open and frees all associated resouces.
 *  In bitflow, this needs to be in the same thread where the corresponding
 *  board open call is made.
 *
 *  @param bf pointer to the BitflowInfo struct
 *  @param numPanels total number of boards opened
 *  @return Ise_Error code if failed, otherwise return ISE_SUCCESS.
 */
Ise_Error bitflow_shutdown(BitflowInfo* bf, int numPanels);


/** @brief Test whether bitflow hardware exists and functions
 *  
 *  Test whether bitflow hardware exists and functions. Return 1 if the
 *  succeeds and 0 if it fails
 *
 *  @param Void
 *  @return 1 for sucesss and 0 for failure.
 */
int bitflow_probe(void);

/** @brief Close the opened board that is not in acquisition mode
 *  
 *  Close the opened board that is not in acquisition mode
 *
 *  @param bf pointer to the BitflowInfo struct
 *	@param idx integer (0/1) for the corresponding board
 *	@ retrun Void.
 */
void bitflow_clear_probe(BitflowInfo* bf, int idx);

#if defined __cplusplus
}
#endif

#endif
#endif
