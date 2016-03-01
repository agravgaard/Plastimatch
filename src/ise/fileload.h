/** @file fileload.h
 *  @brief Function prototypes for the loading images from file
 *
 *  This contains the prototypes for the functions that implement
 *  loading images from file
 *	
 *  @author Rui Li
 *  @bug    No know bugs
 */

/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __FILELOAD_H__
#define __FILELOAD_H__

#include "ise_error.h"
#include "ise_structs.h"

/** @brief Initialize the FileloadInfo struct
 *
 *  Reset the fileload information to NULL
 *
 *  @param fl pointer to the FileloadInfo struct.
 *  @param model integer to indicate whether the system is in the fluoro mode.
 *  @return The error code listed in Ise_Error.
 */
Ise_Error fileload_init (FileloadInfo* fl, unsigned int mode);

/** @brief Open the directory and obtain the information regarding
 *  
 *  Open the directory and obtain the information regarding. This function
 *  must succeed in order to start loading files into the system
 *
 *  @param fl pointer to the FileloadInfo struct.
 *  @param idx integer (0/1) for the corresponding board
 *  @return The error code listed in Ise_Error.
 */
Ise_Error fileload_open (FileloadInfo* fl);

/** @brief loading image files from the directory
 *
 *  load images from the directory and copy them into image buffer.
 *	
 *  @param img pointer to the image buffer.
 *  @param bf pointer to the FileflowInfo struct.
 *  @param idx integer (0/1) for the corresponding board
 *  @return Void
 */
Ise_Error fileload_load_image (unsigned short* img, FileloadInfo *fl, int idx);

/** @brief Stop the fileload and frees all associated resouces.
 * 
 *  @param bf pointer to the FileloadInfo struct
 *  @param idx integer (0/1) for the corresponding board
 *  @return Ise_Error code if failed, otherwise return ISE_SUCCESS.
 */
void fileload_shutdown(FileloadInfo* fl);

/*void fileload_browse (void);
int fileload_load_image (Frame* f, char* fn);
*/
#endif
