/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dlib_threads_h_
#define _dlib_threads_h_

#include "plmsys_config.h"

class Dlib_master_slave_private;
class Dlib_semaphore_private;
class Dlib_thread_function_private;

class PLMSYS_API Dlib_thread_function
{
public:
    Dlib_thread_function_private *d_ptr;
public:
    Dlib_thread_function (void (*thread_routine) (void *), void *arg);
    ~Dlib_thread_function ();
};

class PLMSYS_API Dlib_semaphore
{
public:
    Dlib_semaphore_private *d_ptr;
public:
    Dlib_semaphore (bool grabbed = false);
    ~Dlib_semaphore ();
public:
    void grab ();
    void release ();
};

class PLMSYS_API Dlib_master_slave
{
public:
    Dlib_master_slave_private *d_ptr;
public:
    Dlib_master_slave ();
    ~Dlib_master_slave ();
public:
    void master_grab_resource ();
    void master_release_resource ();
    void slave_grab_resource ();
    void slave_release_resource ();
};

#endif
