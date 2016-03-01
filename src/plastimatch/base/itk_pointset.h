/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_pointset_h_
#define _itk_pointset_h_

#include "plmbase_config.h"
#include "itkPointSet.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itk_point.h"

class Point;
class Xform;
template<class T> class Pointset;
typedef struct raw_pointset Raw_pointset;
typedef Pointset<Point> Unlabeled_pointset;


typedef itk::DefaultStaticMeshTraits< 
    float, 3, 3, float, float > FloatPointSetTraitsType;
typedef itk::PointSet< 
    FloatPoint3DType, 3, FloatPointSetTraitsType > FloatPointSetType;
typedef FloatPointSetType::PointIdentifier FloatPointIdType;

typedef itk::DefaultStaticMeshTraits< 
    double, 3, 3, double, double > DoublePointSetTraitsType;
typedef itk::PointSet< 
    DoublePoint3DType, 3, DoublePointSetTraitsType > DoublePointSetType;
typedef DoublePointSetType::PointIdentifier DoublePointIdType;
typedef itk::PointSet< short, 3 > ShortPointSetType;
typedef ShortPointSetType::PointsContainer ShortPointsContainer;

template<class T> void itk_pointset_load (T pointset, const char* fn);
template<class T> T itk_pointset_warp (T ps_in, Xform* xf);
template<class T> void itk_pointset_debug (T pointset);

PLMBASE_API FloatPointSetType::Pointer itk_float_pointset_from_raw_pointset (Raw_pointset *ps);
PLMBASE_API DoublePointSetType::Pointer itk_double_pointset_from_raw_pointset (Raw_pointset *ps);

template<class T> PLMBASE_API FloatPointSetType::Pointer
itk_float_pointset_from_pointset (const Pointset<T> *ps);
template<class T> PLMBASE_API DoublePointSetType::Pointer
itk_double_pointset_from_pointset (const Pointset<T>& ps);

PLMBASE_API Unlabeled_pointset* unlabeled_pointset_from_itk_float_pointset (FloatPointSetType::Pointer itk_ps);
PLMBASE_API Raw_pointset* raw_pointset_from_itk_float_pointset (FloatPointSetType::Pointer itk_ps);

#endif
