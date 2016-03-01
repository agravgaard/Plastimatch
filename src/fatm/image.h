/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef IMAGE_H
#define IMAGE_H

/* ABOUT THE DIMENSIONS: SLOW MOVING INDICES FIRST!
   For col-major code, this means:
     pmin[0] = cmin,  pmin[1] = rmin.
     dims[0] = ncol,  dims[1] = nrow.
   For row-major code, this means:
     pmin[0] = rmin,  pmin[1] = cmin.
     dims[0] = nrow,  dims[1] = ncol.
*/
/*typedef struct Image_Rect_struct Image_Rect;
struct Image_Rect_struct {
    int pmin[2];
    int dims[2];
};

typedef struct Image_struct Image;
struct Image_struct {
    int dims[2];
    int type;
    void* data;
};
*/

class Image_Rect {

 public:
    int pmin[2];
    int dims[2];
//    unsigned short *data;

 public:
    Image_Rect ();
    ~Image_Rect ();
    void set_dims (int dimens[2]);
};

class Image {

 public:
    int dims[2];
    int type;
    void *data;

 public:
    Image ();
    ~Image ();
};

inline int
image_bytes (Image* image)
{
    return image->dims[0] * image->dims[1] * sizeof(double);
}

inline int
image_size (Image* image)
{
    return image->dims[0] * image->dims[1];
}

inline double*
image_data (Image* image)
{
    return (double*) image->data;
}

inline int
image_index_pt (const int dims[2], const int pt[2])
{
    return (pt[0]*dims[1]+pt[1]);
}

inline int
image_index (const int dims[2], const int p0, const int p1)
{
    return (p0*dims[1]+p1);
}

inline int
image_rect_size (Image_Rect* rect)
{
    return rect->dims[0] * rect->dims[1];
}

int image_bytes (Image* image);
double* image_data (Image* image);
void image_init (Image* image);
void image_malloc (Image* image, int dims[2]);
void image_malloc_rand (Image* image, int dims[2]);
void image_write (Image* image, const char* fn);
void image_free (Image* image);
double image_double (Image* image);

#endif
