/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _fluoro_source_h_
#define _fluoro_source_h_

#include <string>

class Cbuf;

class Fluoro_source {
public:
    Fluoro_source ();
public:
    virtual unsigned long get_size_x (int x) = 0;
    virtual unsigned long get_size_y (int y) = 0;
    virtual const std::string get_type () = 0;
    virtual void start () = 0;
    virtual void grab_image (Frame* f) = 0;
    virtual void stop () = 0;
public:
    void set_cbuf (Cbuf * cbuf) {
        this->cbuf = cbuf;
    }
public:
    Cbuf *cbuf;
};

#endif
