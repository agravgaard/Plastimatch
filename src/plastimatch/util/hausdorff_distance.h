/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _hausdorff_distance_h_
#define _hausdorff_distance_h_

#include "plmutil_config.h"
#include "itk_image.h"

class Plm_image;
class Hausdorff_distance_private;

/*! \brief 
 * The Hausdorff class computes the worst-case distance between 
 * two regions.  There are many variants of the Hausdorff.
 * See for example:
 *
 * "A Modified Hausdorff Distance for Object Matching," 
 * MP Dubuisson and AK Jain, 
 * Proc. International Conference on Pattern Recognition, pp 566–568, 1994.
 * 
 * To understand the variants, we first define the directed Hausdorff 
 * and the boundary Hausdorff.
 * 
 * The directed Hausdorff measure from X to Y is defined as the 
 * maximum distance, for all points in X, to the closest point in Y.
 * Mathematically this is given as:
 * 
 * \f[
 *   \vec{d}_{H}(X,Y) = \max_{x \in X} { \min_{y \in Y} d (x,y) }
 * \f]
 * 
 * The (undirected) Hausdorff distance is the maximum of the two directed 
 * Hausdorff measures.
 * 
 * \f[
 *   d_H(X,Y) = \max \left\{ \vec{d}_{H}(X,Y), \vec{d}_{H}(Y,X) \right\}
 * \f]
 * 
 * The (directed or undirected) boundary Hausdorff is a 
 * Hausdorff distance which is computed on the boundary of a set 
 * rather than the set itself.   For example, the (undirected) 
 * boundary Hausdorff distance is given as:
 *
 * \f[
 *   d_H(\partial X,\partial Y)
 * \f]
 * 
 * The directed average Hausdorff measure is the average distance of 
 * a point in X to its closest point in Y.  That is:
 * 
 * \f[
 *   \vec{d}_{H,\mathrm{avg}}(X,Y) = \frac{1}{|X|} \sum_{x \in X} \min_{y \in Y} d (x,y)
 * \f]
 * 
 * The (undirected) average Hausdorff measure is the average 
 * of the two directed average Hausdorff measures:
 * 
 * \f[
 *   d_{H,\mathrm{avg}}(X,Y) = \frac{\vec{d}_{H,\mathrm{avg}}(X,Y) + \vec{d}_{H,\mathrm{avg}}(Y,X)}{2}
 * \f]
 * 
 * The (undirected) max average Hausdorff measure is the maximum 
 * of the two directed average Hausdorff measures:
 * 
 * \f[
 *   d_{H,\mathrm{m-avg}}(X,Y) = \max \left\{ \vec{d}_{H,\mathrm{avg}}(X,Y) , \vec{d}_{H,\mathrm{avg}}(Y,X) \right\}
 * \f]
 * 
 * The directed percent Hausdorff measure, for a percentile \f$r\f$, 
 * is the \f$r\f$th percentile distance over all distances from points 
 * in X to their closest point in Y.  For example, the directed 95% 
 * Hausdorff distance is the point in X with distance to its closest 
 * point in Y is greater or equal to exactly 95% of the other points in X.
 * In mathematical terms, denoting the 
 * \f$r\f$th percentile as 
 * \f$K_r\f$, this is given as:
 * 
 * \f[
 *   \vec{d}_{H,r}(X,Y) = K_{r}  \left( \min_{y \in Y} d (x,y) \right) \forall x \in X
 * \f]
 * 
 * The (undirected) percent Hausdorff measure is defined again with the mean:
 * 
 * \f[
 *   d_{H,r}(X,Y) = \frac{\vec{d}_{H,r}(X,Y) + \vec{d}_{H,r}(Y,X)}{2}
 * \f]
 * 
 * If the images do not have the same size and resolution, the compare 
 * image will be resampled onto the reference image geometry prior 
 * to comparison.  
 */
class PLMUTIL_API Hausdorff_distance {
public:
    Hausdorff_distance ();
    ~Hausdorff_distance ();
public:
    Hausdorff_distance_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  The image will be loaded
      from the specified filename. */
    void set_reference_image (const char* image_fn);
    /*! \brief Set the reference image as an ITK image. */
    void set_reference_image (const UCharImageType::Pointer image);
    /*! \brief Set the compare image.  The image will be loaded
      from the specified filename. */
    void set_compare_image (const char* image_fn);
    /*! \brief Set the compare image as an ITK image. */
    void set_compare_image (const UCharImageType::Pointer image);
    /*! \brief Set the fraction of voxels to include when computing 
      the percent hausdorff distance.  The input value should be 
      a number between 0 and 1.  The default value is 0.95. */
    void set_hausdorff_distance_fraction (
        float hausdorff_distance_fraction);
    /*! \brief Choose which distance map algorithm to use */
    void set_distance_map_algorithm (const std::string& dmap_alg);
    /*! \brief Choose the maximum distance that is returned when 
      computing the distance map */
    void set_maximum_distance (float maximum_distance);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute hausdorff distances */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the Hausdorff distance */
    float get_hausdorff ();
    /*! \brief Return the average Hausdorff distance */
    float get_avg_average_hausdorff ();
    /*! \brief Return the max average Hausdorff distance */
    float get_max_average_hausdorff ();
    /*! \brief Return the percent Hausdorff distance */
    float get_percent_hausdorff ();
    /*! \brief Return the boundary Hausdorff distance */
    float get_boundary_hausdorff ();
    /*! \brief Return the average boundary Hausdorff distance */
    float get_avg_average_boundary_hausdorff ();
    /*! \brief Return the max average boundary Hausdorff distance */
    float get_max_average_boundary_hausdorff ();
    /*! \brief Return the percent boundary Hausdorff distance */
    float get_percent_boundary_hausdorff ();
    /*! \brief Display debugging information to stdout */
    void debug ();
    ///@}

protected:
    void run_internal (
        UCharImageType::Pointer image1,
        UCharImageType::Pointer image2);
};

PLMUTIL_API
void do_hausdorff(
    UCharImageType::Pointer image_1, 
    UCharImageType::Pointer image_2);

#endif
