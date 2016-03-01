/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_studyset_h_
#define _xio_studyset_h_

#include "plmbase_config.h"
#include <string>
#include <vector>

enum Xio_version {
    XIO_VERSION_UNKNOWN,
    XIO_VERSION_4_2_1,         /* MGH proton Xio */
    XIO_VERSION_4_33_02,       /* Older MGH photon Xio */
    XIO_VERSION_4_5_0,         /* Current MGH photon Xio */
};

class PLMBASE_API Xio_studyset_slice
{
public:
    std::string name;
    float location;

    std::string filename_scan;
    std::string filename_contours;
    
public:
    Xio_studyset_slice (std::string slice_filename_scan, const float slice_location);
    ~Xio_studyset_slice ();

    bool operator < (const Xio_studyset_slice &cmp) const
    {
        return location < cmp.location;
    }
};

class PLMBASE_API Xio_studyset
{
public:
    std::string studyset_dir;
    int number_slices;
    float thickness;
    float ct_pixel_spacing[2];
    std::vector<Xio_studyset_slice> slices;
    
public:
    Xio_studyset (const std::string& studyset_dir);
    ~Xio_studyset ();

private:
    int gcd(int, int);
};

#endif
