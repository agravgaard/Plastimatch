/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ISE_ERROR_H__
#define __ISE_ERROR_H__

enum __Ise_Error {
    ISE_SUCCESS = 0,

    /* Program errors */
    ISE_ERR_MALLOC,
    ISE_ERR_INVALID_PARM,

    /* Matrox errors */
    ISE_MATROX_MAPPALLOC_FAILED,
    ISE_MATROX_MDIGALLOC_FAILED,
    ISE_MATROX_BAD_IMAGE_SIZE,

    /* Bitflow errors */
    ISE_BITFLOW_FIND_BOARD_FAILED,
    ISE_BITFLOW_BOARD_OPEN_FAILED,
    ISE_BITFLOW_R2ACQ_SETUP_FAILED,
    ISE_BITFLOW_SET_VIDEO_TIMING_FAILED,
    ISE_BITFLOW_BAD_IMAGE_SIZE,
    ISE_BITFLOW_MALLOC_FAILED,
    ISE_BITFLOW_CREATE_SIG_FAILED,
    ISE_BITFLOW_R2ACQ_CMD_FAILED,
    ISE_BITFLOW_FREEZE_FAILED,


    /* File loading errors */
    ISE_FILE_OPEN_FAILED,
    ISE_FILE_FIND_FAILED,
    ISE_FILE_LIST_INIT_FAILED,
    ISE_FILE_LIST_GROW_FAILED,
    ISE_FILE_BAD_IMAGE_SIZE,
    ISE_FILE_READ_ERROR,

    /* IGPAX errors */
    ISE_IGPAX_SPAWN_FAILURE,
    ISE_IGPAX_CREATE_PIPE_FAILURE,
    ISE_IGPAX_SERVER_WRITE_FAILED,
    ISE_IGPAX_SERVER_READ_FAILED,
    ISE_IGPAX_SERVER_INCORRECT_RESPONSE,
    ISE_IGPAX_QUEUE_OVERFLOW,
    ISE_IGPAX_QUEUE_USER_APC_FAILED,

};
typedef enum __Ise_Error Ise_Error;

#endif
