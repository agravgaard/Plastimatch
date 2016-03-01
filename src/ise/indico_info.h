/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __indico_info_h__
#define __indico_info_h__

#define INDICO_SHMEM_STRING "ISE_INDICO_SHMEM"

#define INDICO_SHMEM_XRAY_OFF		0
#define INDICO_SHMEM_XRAY_SPINNING_UP	1
#define INDICO_SHMEM_XRAY_PREPARED	2
#define INDICO_SHMEM_XRAY_ON		3


typedef struct indico_shmem_struct Indico_Shmem;
struct indico_shmem_struct {
    int rad_state[2];
};

typedef struct indico_info_struct Indico_Info;
struct indico_info_struct {
#ifdef _WIN32
    HANDLE h_shmem;
#endif
    Indico_Shmem* shmem;
};

void init_indico_shmem (Indico_Info* ii);

#endif
