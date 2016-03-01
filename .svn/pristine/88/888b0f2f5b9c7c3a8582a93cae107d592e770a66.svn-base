/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>

#include "xpm_p.h"
#include "xpm.h"

/* -----------
     Canvas
   ----------- */
Xpm_canvas::Xpm_canvas (int width, int height, int cpp)
{
    d_ptr = new Xpm_canvas_private;

    d_ptr->width = width;
    d_ptr->height = height;
    d_ptr->num_pix = width * height;
    d_ptr->num_colors = 0;
    d_ptr->cpp = cpp;

    // Allocate memory for pixel data
    d_ptr->img = (char*)malloc (width*height*sizeof(char));
}

Xpm_canvas::~Xpm_canvas ()
{
    free (d_ptr->img);
    free (d_ptr->color_code);
    free (d_ptr->colors);
}

void
Xpm_canvas::prime (char color_code)
{
    int i;
    char* img = d_ptr->img;

    for (i=0; i<d_ptr->num_pix; i++)
        img[i] = color_code;
}

void
Xpm_canvas::add_color (char color_code, int color)
{
    // Increase memory usage as necessary
    if (!d_ptr->num_colors) {
        d_ptr->num_colors++;
        d_ptr->colors = (int*)malloc (sizeof(int));
        d_ptr->color_code = (char*)malloc (sizeof(char));
    } else {
        d_ptr->num_colors++;
        d_ptr->colors = (int*)realloc (d_ptr->colors, d_ptr->num_colors * sizeof(int));
        d_ptr->color_code = (char*)realloc (d_ptr->color_code, d_ptr->num_colors * sizeof(char));
    }

    // Insert the color
    d_ptr->colors[d_ptr->num_colors - 1] = color;
    d_ptr->color_code[d_ptr->num_colors - 1] = color_code;
}

// Returns 0 on success
// -- " -- 1 on failure
// 
// I don't plan on ever needing this...
// but perhaps you will, so here it is.
int
Xpm_canvas::remove_color (char color_code)
{
    int i;
    char* code = d_ptr->color_code;

    // Search for the color code and remove it
    for(i=0; i<d_ptr->num_colors; i++)
    {
        if (code[i] == color_code) {
            
            // Decrement palette
            d_ptr->num_colors--;

            // Did we remove the last color?
            if (!d_ptr->num_colors){
                // We have removed all the colors
                free (d_ptr->colors);
                free (d_ptr->color_code);
            } else {
                d_ptr->colors = (int*)realloc (d_ptr->colors, d_ptr->num_colors * sizeof(int));
                d_ptr->color_code = (char*)realloc (d_ptr->color_code, d_ptr->num_colors * sizeof(char));
            }

        } else {
            // color code not registered
            return 1;
        }
    }
    return 0;
}

int
Xpm_canvas::draw (Xpm_brush* brush)
{
    int i, j;
    int x1,x2,y1,y2;

    // which brush, son?
    switch (brush->get_type()) {
    case XPM_BOX:
        // define bounds
        x1 = brush->get_x();
        x2 = brush->get_x() + brush->get_width();
        y1 = brush->get_y();
        y2 = brush->get_y() + brush->get_height();

        // bound checking
        if ( (x1 < 0) || (x2 > d_ptr->width) )
            return 1;

        if ( (y1 < 0) || (y2 > d_ptr->height) )
            return 1;

        // draw the box
        for (j=y1; j<y2; j++)
            for (i=x1; i<x2; i++)
            d_ptr->img[j * d_ptr->width + i] = brush->get_color();
        break;
    case XPM_CIRCLE:
        /* not implemented */
        break;
    }

    return 0;
}

void
Xpm_canvas::write (char* xpm_file)
{
    FILE *fp;
    int i,j,p;

    char* img = d_ptr->img;

    // Write the XPM file to disk
    if ( !(fp = fopen(xpm_file, "w")) )
        fprintf(stderr, "Error: Cannot write open XPM file for writing\n");

    // Construct the XPM header
    fprintf(fp, "/* XPM */\n");
    fprintf(fp, "static char * plm_xpm[] = {\n");
    fprintf(fp, "/* width  height  colors  cpp */\n");
    fprintf(fp, "\"%i %i %i %i\",\n\n", d_ptr->width, d_ptr->height, d_ptr->num_colors, d_ptr->cpp);

    // Construct Palette
    fprintf(fp, "/* color codes */\n");
    for (i=0; i<d_ptr->num_colors; i++)
        fprintf(fp, "\"%c c #%.6x\",\n", d_ptr->color_code[i], d_ptr->colors[i]);

    // Write Pixel Data
    fprintf(fp, "\n/* Pixel Data */\n");

    p=0;
    for (j=0; j<d_ptr->height; j++) {
        fprintf(fp, "\"");

        for (i=0; i<d_ptr->width; i++) {
            fprintf(fp, "%c",img[p++]);
        }
        
        fprintf(fp, "\",\n");
    }

    fprintf(fp, "};");

    // Done like dinner.
    fclose(fp);
}


/* -----------
      Brush 
   ----------- */
Xpm_brush::Xpm_brush ()
{
    d_ptr = new Xpm_brush_private;
}

Xpm_brush::~Xpm_brush ()
{
    delete d_ptr;
}

void
Xpm_brush::set_type (xpm_brushes type)
{
    d_ptr->type = type;
}

void
Xpm_brush::set_color (char color)
{
    d_ptr->color = color;
}

void
Xpm_brush::set_pos (int x, int y)
{
    d_ptr->x_pos = x;
    d_ptr->y_pos = y;
}

void
Xpm_brush::set_width (int width)
{
    d_ptr->width = width;
}

void
Xpm_brush::set_height (int height)
{
    d_ptr->height = height;
}

void
Xpm_brush::set_x (int x)
{
    d_ptr->x_pos = x;
}

void
Xpm_brush::set_y (int y)
{
    d_ptr->y_pos = y;
}

char
Xpm_brush::get_color ()
{
    return d_ptr->color;
}

xpm_brushes
Xpm_brush::get_type ()
{
    return d_ptr->type;
}

int
Xpm_brush::get_width ()
{
    return d_ptr->width;
}

int
Xpm_brush::get_height ()
{
    return d_ptr->height;
}

int
Xpm_brush::get_x ()
{
    return d_ptr->x_pos;
}

int
Xpm_brush::get_y ()
{
    return d_ptr->y_pos;
}

void
Xpm_brush::inc_x (int dx)
{
    d_ptr->x_pos += dx;
}

void
Xpm_brush::inc_y (int dy)
{
    d_ptr->y_pos += dy;
}

