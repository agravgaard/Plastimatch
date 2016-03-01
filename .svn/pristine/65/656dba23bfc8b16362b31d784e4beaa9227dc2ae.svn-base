/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <stdio.h>
#include "vnl/vnl_random.h"

#include "itk_pointset.h"
#include "logfile.h"
#include "rt_study.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "rtss.h"
#include "simplify_points.h"

/* -----------------------------------------------------------------------
    Resorting method for the simplified vector of points
   ----------------------------------------------------------------------- */
int compare (const void * a, const void * b)
{
  return ( *(int*)a < *(int*)b );
}

/* -----------------------------------------------------------------------
    Actual function that simplifies the contours
   ----------------------------------------------------------------------- */
void
do_simplify (Rt_study *rtds, float percentage)
{
    int num_structures=0;
    int first_index_to_remove=0;
    Rtss_roi *curr_struct;
    Rtss_contour *curr_polyline;
    
    vnl_random gnr;

    lprintf ("Hello from simplify_points! \n"
        "You are going to delete %f percent of points from your dataset\n",
        percentage);

    /* Check file_type */
//    if (file_type != PLM_FILE_FMT_DICOM_RTSS) {
//      printf("Error: the input file is not a dicom RT struct!");
//      exit(-1);
//    }

    Rtss *rtss_ss = rtds->get_rtss()->get_structure_set_raw ();
    num_structures = rtss_ss->num_structures;

    for(int j=0;j<num_structures;j++){
        curr_struct=rtss_ss->slist[j];
        for(size_t k=0;k<curr_struct->num_contours;k++){
            int *index, *ordered_index;
            gnr.restart();
            curr_polyline=curr_struct->pslist[k];
            ShortPointSetType::PointType curr_point;
            ShortPointsContainer::Pointer points = ShortPointsContainer::New();
            ShortPointsContainer::Pointer shuffled_points = ShortPointsContainer::New();
            //index = (int*) malloc (sizeof (int) * curr_polyline->num_vertices);
            //ordered_index = (int*) malloc (sizeof (int) * curr_polyline->num_vertices);
            index = new int[curr_polyline->num_vertices];
            ordered_index = new int[curr_polyline->num_vertices];
            //extract vertices of the current contour and extract random indices
            for(int j=0;j<curr_polyline->num_vertices;j++){
                curr_point[0]=curr_polyline->x[j];
                curr_point[1]=curr_polyline->y[j];
                curr_point[2]=curr_polyline->z[j];
                points->InsertElement( j , curr_point );
                index[j]=gnr.drand64()*curr_polyline->num_vertices+0;
            }
            first_index_to_remove= int(double(curr_polyline->num_vertices) * ((100.0-percentage)/100.0));
            //removes the points according to the user-defined percentage               
            for(int pointId=0; pointId<first_index_to_remove; pointId++){
                ordered_index[pointId]=index[pointId];
            }
            //resorting of the points
            //bubble_sort(ordered_index,first_index_to_remove);
            qsort(ordered_index,first_index_to_remove,sizeof(int),compare);
                
            Rtss_contour *new_polyline=new Rtss_contour();
            new_polyline->num_vertices=first_index_to_remove;
            new_polyline->slice_no=curr_polyline->slice_no;
            new_polyline->ct_slice_uid=curr_polyline->ct_slice_uid;
            new_polyline->x = new float[first_index_to_remove+1];
            new_polyline->y = new float[first_index_to_remove+1];
            new_polyline->z = new float[first_index_to_remove+1];
            //get the final points
            for(int pointId=0; pointId<first_index_to_remove; pointId++){
                curr_point=points->GetElement(ordered_index[pointId]);
                new_polyline->x[pointId]=curr_point[0];
                new_polyline->y[pointId]=curr_point[1];
                new_polyline->z[pointId]=curr_point[2];
            }
            curr_point=points->GetElement(ordered_index[0]);
            new_polyline->x[first_index_to_remove]=curr_point[0];
            new_polyline->y[first_index_to_remove]=curr_point[1];
            new_polyline->z[first_index_to_remove]=curr_point[2];
            curr_struct->pslist[k]=new_polyline;
            free (index);
            free (ordered_index);
        }
    }
}
