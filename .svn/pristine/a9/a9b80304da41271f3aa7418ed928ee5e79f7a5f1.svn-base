/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _direction_cosines_h_
#define _direction_cosines_h_

#include "plmbase_config.h"
#include <string>
#include <stdio.h>

#include "plm_math.h"

#define DIRECTION_COSINES_IDENTITY_THRESH 1e-9
#define DIRECTION_COSINES_EQUALITY_THRESH 1e-9

namespace itk { template<class T, unsigned int NRows, unsigned int NColumns> class Matrix; }
typedef itk::Matrix < double, 3, 3 > DirectionType;

class Direction_cosines_private;

class PLMBASE_API Direction_cosines {
public:
    Direction_cosines_private *d_ptr;

public:
    Direction_cosines ();
    Direction_cosines (const float *dm);
    Direction_cosines (const DirectionType& itk_dc);
    ~Direction_cosines ();

public:
    operator const float* () const;
    operator float* ();
    bool operator==(const Direction_cosines& dc) const;
public:
    void set_identity ();

    /* Presets */
    void set_rotated_1 ();
    void set_rotated_2 ();
    void set_rotated_3 ();
    void set_skewed ();

    const float* get_matrix () const;
    float* get_matrix ();
    const float* get_inverse () const;
    void set (const float dc[]);
    void set (const DirectionType& itk_dc);
    bool set_from_string (std::string& str);
    bool is_identity ();
    std::string get_string () const;
protected:
    void solve_inverse ();
private:
    Direction_cosines (const Direction_cosines&);
    void operator= (const Direction_cosines&);
};

#endif
