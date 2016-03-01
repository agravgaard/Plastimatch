/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _aperture_h_
#define _aperture_h_

#include "plmbase_config.h"
#include "plm_image.h"
#include "plm_int.h"

class Aperture_private;

class PLMBASE_API Aperture {
public:
    SMART_POINTER_SUPPORT (Aperture);
public:
    Aperture_private *d_ptr;
public:
    Aperture ();
    ~Aperture ();
    Aperture (const Aperture::Pointer&);
public:
    /*! \name Inputs */
    ///@{

    /*! \brief Get the aperture dimension, in pixels */
    const int* get_dim () const;
    /*! \brief Get the i or j aperture dimension, in pixels */
    int get_dim (int dim) const;
    /*! \brief Set the aperture dimension, in pixels */
    void set_dim (const int* dim);

    /*! \brief Get the aperture center, in pixels */
    const double* get_center () const;
    /*! \brief Get the aperture center in the i or j dimension, in pixels */
    double get_center (int dim) const;

    /*! \brief Set the aperture center, in pixels */
    void set_center (const float* center);
    void set_center (const double* center);

    /*! \brief Set the aperture origin, in mm */
    void set_origin (const float* center);
    void set_origin (const double* center);

    /*! \brief Get the aperture offset: the distance from the
      beam source to closest point on the aperture plane */
    double get_distance () const;
    /*! \brief Get the aperture offset: the distance from the
      beam source to closest point on the aperture plane */
    void set_distance (double distance);

    /*! \brief Get the aperture spacing: the distance between 
      sampling points in the aperture plane, in mm */
    const double* get_spacing () const;
    /*! \brief Get the aperture spacing in the i or j dimension, in mm */
    double get_spacing (int dim) const;

    /*! \brief Get the aperture spacing: the distance between 
      sampling points in the aperture plane */
    void set_spacing (const float* spacing);
    void set_spacing (const double* spacing);

    /*! \brief Get the aperture vup vector, which is the vector 
      that orients the top of the aperture in room coordinates */
    void set_vup (const float* vup);

    /*! \brief Allocate aperture and range compensator images */
    void allocate_aperture_images ();

    /*! \brief Test if the aperture has a bitmap image describing shape */
    bool have_aperture_image ();
    /*! \brief Get the aperture image as Plm_image */
    Plm_image::Pointer& get_aperture_image ();
    /*! \brief Get the aperture image as Volume */
    Volume::Pointer& get_aperture_volume ();
    /*! \brief Load the aperture image from a file */
    void set_aperture_image (const char *ap_filename);

    /*! \brief Load the aperture volume from a file */
    void set_aperture_volume (Volume::Pointer ap);

    /*! \brief Test if the aperture has a float image describing 
      range compensator thicknesses */
    bool have_range_compensator_image ();
    /*! \brief Get the range_compensator image as Plm_image */
    Plm_image::Pointer& get_range_compensator_image ();
    /*! \brief Get the range_compensator image as Volume */
    Volume::Pointer& get_range_compensator_volume ();

    /*! \brief Load the range_compensator image from a file */
    void set_range_compensator_image (const char *rc_filename);

    /*! \brief Load the range_compensator volume from a file */
    void set_range_compensator_volume (Volume::Pointer ap);

    /*! \brief Expand aperture and smear compensator.  The smearing 
      parameters is defined as mm around the target in the beam eye view frame
	  at the target minimal depth. */
    void apply_smearing_to_aperture (float smearing, float target_depth);
	void apply_smearing_to_range_compensator (float smearing, float target_depth);
    ///@}

public:
    double vup[3];        /* orientation */
    double ic_room[3];    /* loc of center (room coords) */
    double ul_room[3];    /* loc of upper left corder (room coords) */
    double incr_r[3];     /* row increment vector */
    double incr_c[3];     /* col increment vector */
    double nrm[3];        /* unit vec: normal */
    double pdn[3];        /* unit vec: down */
    double prt[3];        /* unit vec: right */
    double tmp[3];
};

#endif

