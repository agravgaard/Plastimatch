/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "config.h"
#include "image.h"

Image_Rect::Image_Rect () {
    dims[0] = 0;
    dims[1] = 1;
}

Image_Rect::~Image_Rect () {
}

void
Image_Rect::set_dims (int dimens[2]) {
    this->dims[0] = dimens[0];
    this->dims[1] = dimens[1];
}

Image::Image () {
}

Image::~Image () {
}

/* From K&R book, generates pseudo-random number on 0..32767 */
static unsigned int 
rand_kr (void)
{
    static unsigned long int next = 1;
    next = next * 1103515245 + 12345; 
    return (unsigned int) (next / 65536) % 32768; 
}

void
image_init (Image* image)
{
    image->dims[0] = 0;
    image->dims[1] = 0;
    image->data = 0;
}

void
image_malloc (Image* image, int dims[2])
{
    image->dims[0] = dims[0];
    image->dims[1] = dims[1];
    image->data = malloc(image_bytes(image));
}

void
image_malloc_rand (Image* image, int dims[2])
{
    int i;
    image_malloc (image, dims);
    for (i = 0; i < image_size(image); i++) {
        image_data(image)[i] = rand_kr() / 32768.0;
    }
}

void
image_write (Image* image, const char* fn)
{
    FILE* fp;
    int i;

    fp = fopen (fn, "wb");
    if (!fp) return;
    fprintf (fp,
             "Pf\n"
             "%d %d\n"
             "-1\n",
             image->dims[0], image->dims[1]);
    for (i = 0; i < image_size(image); i++) {
        float fv = (float) image_data(image)[i];
        fwrite (&fv, sizeof(float), 1, fp);
    }
    fclose (fp);
}

void
image_read (Image* image, char* fn)
{
    FILE* fp;
    int i;
    char buf[1024];

    image_init (image);  /* Leaks if image previously used */
    fp = fopen (fn, "rb");
    if (!fp) return;

    if (fgets (buf, 1024, fp) == NULL) {
        printf ("Error reading pfm file\n");
        return;
    }
    if (strcmp (buf, "Pf\n")) {
        fclose (fp);
        printf ("Error reading pfm file\n");
        return;
    }

    if (fgets (buf, 1024, fp) == NULL) {
        printf ("Error reading pfm file\n");
        return;
    }
    if (sscanf (buf, "%d %d", &image->dims[0], &image->dims[1]) != 2) {
        fclose (fp);
        printf ("Error reading pfm file\n");
        return;
    }

    if (fgets (buf, 1024, fp) == NULL) {
        printf ("Error reading pfm file\n");
        return;
    }
    if (strcmp (buf, "-1\n")) {
        fclose (fp);
        printf ("Error reading pfm file\n");
        return;
    }
    image_malloc (image, image->dims);
    for (i = 0; i < image_size(image); i++) {
        float fv;
        fread (&fv, sizeof(float), 1, fp);
        image_data(image)[i] = (double) fv;
    }
    fclose (fp);
}

void
image_free (Image* image)
{
    if (image->data) free (image->data);
}

#if defined (commentout)
double
image_double (Image* image)
{
    double sig[(image->dims[0])*(image->dims[1])];
    image->data = sig;
}
#endif
