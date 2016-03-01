/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __ise_structs_h__
#define __ise_structs_h__

#include "ise_config.h"
#ifdef _WIN32
#include <windows.h>
#endif
#if (MIL_FOUND)
#include <mil.h>
#endif
#if (BITFLOW_FOUND)
#include "R2Api.h"
#include "BFApi.h"
#endif

#ifndef _MAX_PATH
#ifdef _MAX_PATH
#define _MAX_PATH PATH_MAX
#else
#define _MAX_PATH 255
#endif
#endif

/* -------------------------------------------------------------------------*
    Various definitions
 * -------------------------------------------------------------------------*/
#ifndef MAXGREY
#define MAXGREY 16384            // 14-bit
#endif

/* These must match the command processor defines */
#define IRISGRAB_COMMAND_PROCESSOR_LORES_FLUORO 0
#define IRISGRAB_COMMAND_PROCESSOR_HIRES_FLUORO 1
#define IRISGRAB_COMMAND_PROCESSOR_HIRES_RADIO 2

#define ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO 0
#define ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO 1
#define ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO 100
#define ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO 101
#define ISE_IMAGE_SOURCE_INTERNAL_FLUORO 200
#define ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO 300
#define ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO 301
#define ISE_IMAGE_SOURCE_FILE_LORES_FLUORO 400
#define ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO 401

/* Frames per second */
enum __Framerate {
    ISE_FRAMERATE_1_FPS,
    ISE_FRAMERATE_7_5_FPS
};
typedef enum __Framerate Framerate;

#define IS_SIMULATED_FLUORO(a) \
    ((a)==ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_FILE_LORES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO)

#define IS_REAL_FLUORO(a) \
    ((a)==ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO \
     || (a)==ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO)


#define LORES_IMAGE_WIDTH (1024)
#define LORES_IMAGE_HEIGHT (768)
#define HIRES_IMAGE_WIDTH (2048)
#define HIRES_IMAGE_HEIGHT (1536)

/* Bitmasks form frame locks */
#define FRAME_UNLOCKED       0x00
#define FRAME_DISPLAY_LOCK   0x01
#define FRAME_WRITE_LOCK     0x02

#define INTERNAL_GRAB_BEGIN ((Frame*) -1)
#define INTERNAL_GRAB_END ((Frame*) 0)

#define IGPAX_CMD_QUEUE_SIZE 20


/* -------------------------------------------------------------------------*
    The data structures themselves
 * -------------------------------------------------------------------------*/
/* Forward declarations */
typedef struct __OntrakThreadData OntrakThreadData;
typedef struct __OntrakData OntrakData;
typedef struct __FileWrite FileWrite;

typedef struct __IgpaxInfo IgpaxInfo;
struct __IgpaxInfo {
#ifdef _WIN32
    HANDLE hthread;
    HANDLE hevent;
    CRITICAL_SECTION crit_section;
#endif
    int pipe_to_igpax[2];
    int pipe_from_igpax[2];
    int pid;
    char cmd_queue[IGPAX_CMD_QUEUE_SIZE];
    int cmd_queue_len;
    int cmd_queue_err;
    char ip_address_client[20];
    char ip_address_server[20];
};

typedef struct __ThreadData {
    struct __IseFramework *ig;
    unsigned long imager_no;
    int done;
    void (*notify_routine) (int);
} ThreadData;

struct __Frame {
    struct __Frame* prev;
    struct __Frame* next;
    unsigned short* img;
	
    unsigned long id;
    double timestamp;
    int writable;
    int written;
    int write_lock;
    int display_lock;
    int indico_state;

    Autosense autosense;

    long clip_x;
    long clip_y;
};
typedef struct __Frame Frame;

typedef struct __FrameQueue {
    unsigned long queue_len;
    Frame* head;
    Frame* tail;
} FrameQueue;

struct __CBuf {
    Frame* frames;
    unsigned long num_frames;
    unsigned long writable;
    unsigned long waiting_unwritten;
    unsigned long dropped;
    FrameQueue empty;
    FrameQueue waiting;
    Frame* write_ptr;
    Frame* display_ptr;
    Frame* internal_grab_ptr;
#ifdef _WIN32
    CRITICAL_SECTION cs;
#endif
};
typedef struct __CBuf CBuf;

typedef struct __TrackerInfo {
    long m_curr_x;
    long m_curr_y;
    long* m_score;
    int m_score_size;
    int m_search_w, m_search_h;
    short* m_template;
    int m_template_w, m_template_h;
    int m_template_s1, m_template_s2;
} TrackerInfo;

typedef struct __Panel Panel;
struct __Panel {

    /* How many fps? */
    Framerate framerate;

    /* How to rotate image? */
    unsigned int rotate_flag;

    /* Marker tracking */
    unsigned int have_tracker;
    unsigned int now_tracking;
    TrackerInfo tracker_info;
};

#if (MIL_FOUND)
typedef struct __MatroxInfo MatroxInfo;
struct __MatroxInfo {
    MIL_ID milapp;
    MIL_ID milsys[2];
    MIL_ID mildig[2];
    MIL_ID milimg[2][2];
    int active[2];	    /* index of frame being grabbed */
    int next_active;
    unsigned long mtx_size_x;
    unsigned long mtx_size_y;
};
#endif

#if (BITFLOW_FOUND)
typedef struct __BitflowInfo BitflowInfo;
struct __BitflowInfo {
    //handle to the bitflow board
    RdRn hBoard[2];

    //Signal -- tell when image is completely in memory
    R2SIGNAL Signal[2];

    //host based QTABS
    BFU32 gQTabMode[2];

    //set up double buffer for grabbing image
    PBFU8 hostBuffer[2][2];
    //bank -- tell which bank to use
    BFU32 bank[2];
    //size inforation
    BFU32 imageSize;
    BFU32 sizeX;
    BFU32 sizeY;
    BFU32 bitDepth;
    // 0-- not in continuous mode
    // 1-- in continuous mode
    BOOL acqMode;
};
#endif

typedef struct __FileloadInfo FileloadInfo;
struct __FileloadInfo {

    // size inforation
    // assume all the images under the same directory
    // have the same size.
    unsigned long sizeX;
    unsigned long sizeY;

    // total number of images in the current
    // directory starting from the user chosen
    // first frame
    unsigned long nImages[2]; 

    // maximum number of images
    // initialized to MAX_PATH = 260
    unsigned long maxNImages;
    
    // current image to be loaded;
    unsigned long curIdx[2];
    
    // image captured from panel 0 and 1
    char** imFileList[2];

    // number of panels
    int nPanels;
};

struct __OntrakThreadData {
    OntrakData* od;
};

struct __OntrakData {
    void* ontrak_device;
    int gate_beam;
    int bright_frame;

#ifdef _WIN32
    HANDLE semaphore;
    HANDLE thread;
#endif
    OntrakThreadData thread_data;
};

typedef struct __IseFramework IseFramework;
struct __IseFramework {

    /* Imaging systems */
    int num_idx;
    Panel panel[2];

    /* Image panels */
    unsigned long image_source;
#if (MIL_FOUND)
    MatroxInfo matrox;
#endif
#if (BITFLOW_FOUND)
    BitflowInfo bitflow;
#endif

    /* File loader */
    FileloadInfo fileload;

    /* Relay box */
    OntrakData* od;

    /* File writer subsystem */
    FileWrite* fw;
    unsigned int write_flag;
    unsigned int write_dark_flag;

    /* All images must have same size (for now) */
    unsigned long size_x;
    unsigned long size_y;

    /* Circular buffers */
    CBuf cbuf[2];

    /* Ethernet interface to command processors */
    IgpaxInfo igpax[2];

    /* Thread data */
#ifdef _WIN32
    HANDLE grab_thread[2];
#endif
    ThreadData grab_thread_data[2];
};

typedef struct __FWThreadData {
    struct __FileWrite *fw;
} FWThreadData;

struct __FileWrite {
    IseFramework* ig;
    int imgsize;
    int end;
    char output_dir_1[_MAX_PATH];
    char output_dir_2[_MAX_PATH];

#ifdef _WIN32
    HANDLE threads[2];
#endif
    FWThreadData thread_data[2];
};

#endif
