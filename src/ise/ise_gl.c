/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "ise.h"
#include "ise_globals.h"
#include "ise_gdi.h"
#include "ise_gl.h"
#include "ise_gl_shader.h"
#include "ise_framework.h"
#include "ise_config.h"
#include "frame.h"
#include "debug.h"

/* ---------------------------------------------------------------------------- *
    Global variables
 * ---------------------------------------------------------------------------- */
#define HITEXSIZE1 2048
#define HITEXSIZE2 1536
#define HITEXSIZE2a 1024
#define HITEXSIZE2b 512

#define LOTEXSIZE1 1024
#define LOTEXSIZE2 768
#define LOTEXSIZE2a 512
#define LOTEXSIZE2b 256

GLuint base;

/* ---------------------------------------------------------------------------- *
    Function declarations
 * ---------------------------------------------------------------------------- */
void gl_overlay_tracking_rectangle (int idx, long x, long y);
void gl_overlay_text (int idx);
static void set_default_zooms (int idx);
void gl_overlay_findtrack_rectangle (int idx);
void gl_overlay_crosshair (int idx);

/* ---------------------------------------------------------------------------- *
    Functions
 * ---------------------------------------------------------------------------- */
/* GCS: I'm not sure why this function is necessary */
static int
MySetPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd = { 
	sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
	1,                                // version number 
	PFD_DRAW_TO_WINDOW |              // support window 
	PFD_SUPPORT_OPENGL |              // support OpenGL 
	PFD_DOUBLEBUFFER,                 // double buffered 
	PFD_TYPE_RGBA,                    // RGBA type 
	24,                               // 24-bit color depth 
	0, 0, 0, 0, 0, 0,                 // color bits ignored 
	0,                                // no alpha buffer 
	0,                                // shift bit ignored 
	0,                                // no accumulation buffer 
	0, 0, 0, 0,                       // accum bits ignored 
	32,                               // 32-bit z-buffer     
	0,                                // no stencil buffer 
	0,                                // no auxiliary buffer 
	PFD_MAIN_PLANE,                   // main layer 
	0,                                // reserved 
	0, 0, 0                           // layer masks ignored 
    };
    
    int  iPixelFormat; 

    // get the device context's best, available pixel format match 
    if((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0)
    {
	MessageBox(NULL, "ChoosePixelFormat Failed", NULL, MB_OK);
	return 0;
    }
	
    // make that match the device context's current pixel format 
    if(SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE)
    {
	MessageBox(NULL, "SetPixelFormat Failed", NULL, MB_OK);
	return 0;
    }

    return 1;
}

void
debug_modelview_matrix (void)
{
    GLdouble modelview[16];
    glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
    debug_printf ("MODELVIEW = \n"
		  "%g %g %g %g\n"
		  "%g %g %g %g\n"
		  "%g %g %g %g\n"
		  "%g %g %g %g\n",
		  modelview[0], modelview[4], modelview[8], modelview[12], 
		  modelview[1], modelview[5], modelview[9], modelview[13], 
		  modelview[2], modelview[6], modelview[10], modelview[14], 
		  modelview[3], modelview[7], modelview[11], modelview[15]);
}

void
gl_get_pos (int idx, int x, int y, float* glpos_x, float* glpos_y)
{
    int rc;
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    RECT rect;

    rc = wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);
    get_picture_window_rect (idx, &rect);
    x = x - rect.left;
    y = y - rect.top;

    /* GCS FIX: This would work, except that the matrices at the top of the 
	stack are not the right ones. */
    glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev (GL_PROJECTION_MATRIX, projection);
    glGetIntegerv (GL_VIEWPORT, viewport);

    winX = (float) x;
    winY = (float) viewport[3] - (float) y;
    glReadPixels (x, (int) winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject (winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    *glpos_x = (float) posX;
    *glpos_y = (float) posY;
}

void
gl_get_image_pos (int idx, int x, int y, int* im_x, int* im_y)
{
    float glpos_x, glpos_y;
    int impos_x, impos_y;

    gl_get_pos (idx, x, y, &glpos_x, &glpos_y);

    impos_x = (int) ((((glpos_x + 1.0) / 2.0) * HIRES_IMAGE_WIDTH) + 0.5);
    impos_y = (int) ((((glpos_y - 1.0) / -2.0) * HIRES_IMAGE_HEIGHT) + 0.5);

    *im_x = impos_x;
    *im_y = impos_y;
}

void
gl_set_findtrack_overlay_pos (int idx, int x, int y)
{
    globals.win[idx].findtrack_overlay_x = (float) x;
    globals.win[idx].findtrack_overlay_y = (float) y;
}

void
gl_zoom_at_pos (int idx, int x, int y)
{
    float posX, posY;

    gl_get_pos (idx, x, y, &posX, &posY);

    /* Now that I have the position, I need to zoom here */
    set_default_zooms (idx);
    if (!globals.win[idx].is_zoomed) {
	globals.win[idx].zoomx *= 2.0;
	globals.win[idx].zoomy *= 2.0;
	globals.win[idx].panx = -posX;
	globals.win[idx].pany = -posY;
    }
    globals.win[idx].is_zoomed = !globals.win[idx].is_zoomed;
}

/* The nehe functions come from here: http://nehe.gamedev.net/
   LICENSE: "What you do with the code is up to you."
            (see http://nehe.gamedev.net/lesson.asp?index=01)
*/
GLvoid
gl_nehe_printf (const char *fmt, ...)
{
    char text[256];
    va_list ap;

    if (fmt == NULL)
	return;

    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);

    glPushAttrib(GL_LIST_BIT);
    glListBase(base - 32);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
    glPopAttrib();
}

GLvoid
gl_nehe_build_font (HDC hdc)
{
    HFONT font;
    HFONT oldfont;

    base = glGenLists(96);
    font = CreateFont (-16,				// Height Of Font
			0,				// Width Of Font
			0,				// Angle Of Escapement
			0,				// Orientation Angle
			FW_BOLD,			// Font Weight
			FALSE,				// Italic
			FALSE,				// Underline
			FALSE,				// Strikeout
			ANSI_CHARSET,			// Character Set Identifier
			OUT_TT_PRECIS,			// Output Precision
			CLIP_DEFAULT_PRECIS,		// Clipping Precision
			ANTIALIASED_QUALITY,		// Output Quality
			FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
			"Courier New");			// Font Name
    oldfont = (HFONT) SelectObject (hdc, font);         // Selects The Font We Want
    wglUseFontBitmaps (hdc, 32, 96, base);		// Builds 96 Characters Starting At Character 32
    SelectObject (hdc, oldfont);
    DeleteObject (font);
}

void
gl_init_overlay_text (void)
{
    int idx;
    for (idx = 0; idx < globals.num_panels; idx++) {
	wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);
	gl_nehe_build_font (globals.win[idx].hpdc);
    }
}

void
init_gl (void)
{
    int idx;
    int imager_no = 0;
    int rc;

    for (idx = 0; idx < globals.num_panels; idx++) {
	MySetPixelFormat(globals.win[idx].hpdc);
	globals.win[idx].hglrc = wglCreateContext(globals.win[idx].hpdc);
	rc = wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);
	resize_gl_window (idx);
	glGenTextures(1, &globals.win[idx].texture_name);
	glBindTexture(GL_TEXTURE_2D, globals.win[idx].texture_name);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (globals.have_bitflow_hardware)
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16, LOTEXSIZE1, LOTEXSIZE2a, 
			0, GL_LUMINANCE, GL_UNSIGNED_SHORT,
			0);
    else
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16, HITEXSIZE1, HITEXSIZE2a, 
			0, GL_LUMINANCE, GL_UNSIGNED_SHORT,
			0);

	globals.win[idx].si = shader_init();
    }
    gl_init_overlay_text ();
}

void
gl_update_lut (int idx, unsigned short bot, unsigned short top)
{
    shader_update_lut (globals.win[idx].si, bot, top);
}

static void 
set_default_zooms (int idx)
{
    RECT rect;
    float w1, h1;
    float xpct, ypct;

    get_picture_window_rect (idx, &rect);

    w1 = (float) (rect.right - rect.left + 1);
    h1 = (float) (rect.bottom - rect.top + 1);
    xpct = w1 / 2048;
    ypct = h1 / 1536;
    
    /* This zooming works for "dips style" */
    if (xpct > ypct) {
	globals.win[idx].zoomx = 1.0;
	globals.win[idx].zoomy = xpct / ypct;
    } else {
	globals.win[idx].zoomx = ypct / xpct;
	globals.win[idx].zoomy = 1.0;
    }

    globals.win[idx].panx = 0.0;
    globals.win[idx].pany = 0.0;
}

void 
resize_gl_window (int idx)
{
    RECT rect;

    set_default_zooms (idx);

    wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);

    get_picture_window_rect (idx, &rect);
    glViewport (0,0,rect.right-rect.left+1,rect.bottom-rect.top+1);

    /* In this code, GL_PROJECTION (i.e. clipping planes) is always
       set from -1 to 1.  The GL_MODELVIEW is modified to zoom & pan. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* Non-power-of-two textures are dog-slow.  This is a tiled version, which 
   uses two textures */
void
blt_frame_gl (int idx, Frame* frame, void* buf, int image_source)
{
    int rc;
    unsigned short* texture = (unsigned short*) buf;
    float zoomx = globals.win[idx].zoomx;
    float zoomy = globals.win[idx].zoomy;
    float panx = globals.win[idx].panx;
    float pany = globals.win[idx].pany;
    unsigned long texSizeX, texSizeY1, texSizeY2;
    
    if (image_source == ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO)
    {
        texSizeX = LOTEXSIZE1;
        texSizeY1 = LOTEXSIZE2a;
        texSizeY2 = LOTEXSIZE2b;
    }
    else
    {
        texSizeX = HITEXSIZE1;
        texSizeY1 = HITEXSIZE2a;
        texSizeY2 = HITEXSIZE2b;
    }

    rc = wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);
    glMatrixMode(GL_MODELVIEW);

    shader_apply (globals.win[idx].si);

    // ?? try this...
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glDisable(GL_BLEND);
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, globals.win[idx].texture_name);

    //---------- Part 2a (top)
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texSizeX, texSizeY1, 
        GL_LUMINANCE, GL_UNSIGNED_SHORT, 
        texture);
    
    glPushMatrix();

      glLoadIdentity();
      glScalef (zoomx,zoomy,1.0f);
      glTranslatef (panx,pany,0.0f);

      //debug_modelview_matrix ();

      glBegin(GL_POLYGON);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, 1.0f, 0.0f);		// upper left
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-1.0f, -1.0f/3.0f, 0.0f);	// lower left
	glTexCoord2f(1.0f, 1.0f);glVertex3f(1.0f, -1.0f/3.0f, 0.0f);	// lower right
	glTexCoord2f(1.0f, 0.0f);glVertex3f(1.0f, 1.0f, 0.0f);		// upper right
      glEnd();
    glPopMatrix();

    //---------- Part 2b (bottom)
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texSizeX, texSizeY2, 
			GL_LUMINANCE, GL_UNSIGNED_SHORT, 
			&texture[texSizeX*texSizeY1]);
    glPushMatrix();
      glLoadIdentity();
      glScalef (zoomx,zoomy,1.0f);
      glTranslatef (panx,pany,0.0f);
      glBegin(GL_POLYGON);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-1.0f, -1.0f/3.0f, 0.0f);	// upper left
	glTexCoord2f(0.0f, 0.5f);glVertex3f(-1.0f, -1.0f, 0.0f);	// lower left
	glTexCoord2f(1.0f, 0.5f);glVertex3f(1.0f, -1.0f, 0.0f);		// lower right
	glTexCoord2f(1.0f, 0.0f);glVertex3f(1.0f, -1.0f/3.0f, 0.0f);	// upper right
      glEnd();
    glPopMatrix();
    //glFlush();
    //glDisable(GL_TEXTURE_2D);

    shader_disable (globals.win[idx].si);

#if (ISE_TRACKING_ENABLED)
    gl_overlay_findtrack_rectangle (idx);
    gl_overlay_tracking_rectangle (idx, frame->clip_x, frame->clip_y);
#endif

    /* Rubber banding overlay */

    /* Text overlay */
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    gl_overlay_text (idx);

    /* Crosshair overlay */
    gl_overlay_crosshair (idx);

    //glDisable(GL_TEXTURE_2D);
    //glDisable(GL_BLEND);

    glFlush();
    SwapBuffers (globals.win[idx].hpdc);
}

void
black_frame_gl (int idx)
{
    int rc;

    rc = wglMakeCurrent (globals.win[idx].hpdc, globals.win[idx].hglrc);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFlush();
    SwapBuffers (globals.win[idx].hpdc);
}

void
gl_overlay_text (int idx)
{
    int width = 640;
    int height = 480;
    GLfloat white[3] = { 1.0, 1.0, 1.0 }; 
    GLfloat red[3] = { 1.0, 0.0, 0.0 }; 

    if (globals.is_writing) {
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glColor3f (1.0f,0.0f,0.0f);
	glRasterPos2f (-.9f, -.9f);
	gl_nehe_printf ("Saving");
    }

    if (idx == 0) {
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glColor3f (0.0f,0.65f,0.0f);
	glRasterPos2f (0.0f, -.9f);
	gl_nehe_printf ("INF");
	glRasterPos2f (0.0f, +.9f);
	gl_nehe_printf ("SUP");
	glRasterPos2f (-.9f, 0.0f);
	gl_nehe_printf ("POS");
	glRasterPos2f (+.9f, 0.0f);
	gl_nehe_printf ("ANT");
    } else if (idx == 1) {
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glColor3f (0.0f,0.65f,0.0f);
	glRasterPos2f (0.0f, -.9f);
	gl_nehe_printf ("INF");
	glRasterPos2f (0.0f, +.9f);
	gl_nehe_printf ("SUP");
	glRasterPos2f (-.9f, 0.0f);
	gl_nehe_printf ("RGT");
	glRasterPos2f (+.9f, 0.0f);
	gl_nehe_printf ("LFT");
    }
}

void
gl_image_to_video (float* vx, float* vy, int ix, int iy)
{
    *vx = (2.0f*((float)ix/(float)HIRES_IMAGE_WIDTH)) - 1.0f;
    *vy = (-2.0f*((float)iy/(float)HIRES_IMAGE_HEIGHT)) + 1.0f;
}

void
gl_video_to_image (float* ix, float* iy, float vx, float vy)
{
    *ix = ((vx+1.0f)/2.0f) * (float) HIRES_IMAGE_WIDTH;
    *iy = ((1.0f-vy)/2.0f) * (float) HIRES_IMAGE_HEIGHT;
}

void
gl_client_to_image (int idx, float* ix, float* iy, int cx, int cy)
{
    float vx, vy;

    gl_client_to_video (idx, &vx, &vy, cx, cy);
    gl_video_to_image (ix, iy, vx, vy);
}

void
gl_client_to_video (int idx, float* vx, float* vy, int cx, int cy)
{
    RECT rect;
    float zoomx = globals.win[idx].zoomx;
    float zoomy = globals.win[idx].zoomy;
    float panx = globals.win[idx].panx;
    float pany = globals.win[idx].pany;

    get_picture_window_rect (idx, &rect);

    *vx = -panx + (2.0f*(cx - rect.left)/(rect.right - rect.left) - 1.0f)/zoomx;
    *vy = -pany + (-2.0f*(cy - rect.top)/(rect.bottom - rect.top) + 1.0f)/zoomy;
}

void
gl_draw_rectangle (int idx, float vx1, float vx2, float vy1, float vy2)
{
    float m_r, m_g, m_b, m_a;
    float zoomx = globals.win[idx].zoomx;
    float zoomy = globals.win[idx].zoomy;
    float panx = globals.win[idx].panx;
    float pany = globals.win[idx].pany;

    /* Set the colors */
    m_r = 1; m_g = 0; m_b = 0; m_a = 1;

    glPushMatrix();
      glLoadIdentity();
      glScalef (zoomx,zoomy,1.0f);
      glTranslatef (panx,pany,+0.05f);
      glColor4f(m_r, m_g, m_b, m_a);
      glLineWidth(1.0);
      glBegin(GL_LINE_LOOP);
        glVertex3f(vx1, vy1, 0.0);    // upper left
	glVertex3f(vx2, vy1, 0.0);    // upper right
	glVertex3f(vx2, vy2, 0.0);    // lower right
	glVertex3f(vx1, vy2, 0.0);    // lower left
      glEnd();
    glPopMatrix();
}

void
gl_draw_line (int idx, float vx1, float vx2, float vy1, float vy2)
{
    float m_r, m_g, m_b, m_a;
    float zoomx = globals.win[idx].zoomx;
    float zoomy = globals.win[idx].zoomy;
    float panx = globals.win[idx].panx;
    float pany = globals.win[idx].pany;

    /* Set the colors */
    m_r = 0; m_g = 0; m_b = 1; m_a = 1;

    glPushMatrix();
      glLoadIdentity();
      glScalef (zoomx,zoomy,1.0f);
      glTranslatef (panx,pany,+0.05f);
      glColor4f(m_r, m_g, m_b, m_a);
      glLineWidth(1.0);
      glBegin(GL_LINES);
        glVertex3f(vx1, vy1, 0.0);
	glVertex3f(vx2, vy2, 0.0);
      glEnd();
    glPopMatrix();
}

void
gl_overlay_findtrack_rectangle (int idx)
{
    int x = (int) globals.win[idx].findtrack_overlay_x;
    int y = (int) globals.win[idx].findtrack_overlay_y;
    float vx1, vy1;
    float vx2, vy2;
    const float sx = .015f;
    const float sy = .02f;

    if (!globals.win[idx].findtrack_overlay_flag) return;

    gl_client_to_video (idx, &vx1, &vy1, x, y);
    vx2 = vx1 + sx;
    vy2 = vy1 + sy;
    vx1 = vx1 - sx;
    vy1 = vy1 - sy;
    gl_draw_rectangle (idx, vx1, vx2, vy1, vy2);
}

void
gl_overlay_tracking_rectangle (int idx, long x, long y)
{
    float vx1, vy1;
    float vx2, vy2;
    const float sx = .015f;
    const float sy = .02f;

    gl_image_to_video (&vx1, &vy1, x, y);
    vx2 = vx1 + sx;
    vy2 = vy1 + sy;
    vx1 = vx1 - sx;
    vy1 = vy1 - sy;
    gl_draw_rectangle (idx, vx1, vx2, vy1, vy2);
}

void
gl_overlay_crosshair (int idx)
{
    float vx, vy;
    int cx0 = 1018;
    int cy0 = 771;
    int cx1 = 1039;
    int cy1 = 754;

    cx0 = 2047 - cx0;
    cx1 = 2047 - cx1;

    if (idx == 0) {
	gl_image_to_video (&vx, &vy, cx0, cy0);
    } else {
	gl_image_to_video (&vx, &vy, cx1, cy1);
    }
    gl_draw_line (idx, -1.0f, 1.0f, (float)vy, (float)vy);
    gl_draw_line (idx, (float)vx, (float)vx, -1.0f, 1.0f);
}
