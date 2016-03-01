/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_tps_h_
#define _itk_tps_h_

#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPoint.h"
#include "itkPointSet.h"
#include "itkThinPlateSplineKernelTransform.h"

class Landmark_warp;

class TPS_parms {
public:
    char* reference;
    char* target;
    char* fixed;
    char* moving;
    char* warped;
    char* vf;
};

template<class T>
PLMREGISTER_API void do_tps (TPS_parms* parms,typename itk::Image<T,3>::Pointer img_fixed, typename itk::Image<T,3>::Pointer img_moving,T);
PLMREGISTER_API void itk_tps_warp (Landmark_warp *lw);

#endif
