//
#include "oraInvisibleXOpenGLRenderWindow.h"

#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>
#include <vtkCommand.h>
#include <vtkgl.h>

#include <GL/glx.h>
#if defined( VTK_OPENGL_HAS_OSMESA )
#include <GL/osmesa.h>
#endif

#include <X11/Xlib.h>

vtkCxxRevisionMacro(InvisibleXOpenGLRenderWindow, "1.0");
vtkStandardNewMacro(InvisibleXOpenGLRenderWindow);

/** Re-definition of the internal helper class of vtkXOpenGLRenderWindow. **/
class vtkXOpenGLRenderWindowInternal
{
  friend class InvisibleXOpenGLRenderWindow;
private:
  vtkXOpenGLRenderWindowInternal(vtkRenderWindow*);

  GLXContext ContextId;

  // so we basically have 4 methods here for handling drawables
  // how about abstracting this a bit?

  // support for Pixmap based offscreen rendering
  Pixmap pixmap;
  GLXContext PixmapContextId;
  Window PixmapWindowId;

  // support for Pbuffer based offscreen rendering
  GLXContext PbufferContextId;
#ifndef VTK_IMPLEMENT_MESA_CXX
  vtkglX::GLXPbuffer Pbuffer;
#endif

  // store previous settings of on screen window
  int ScreenDoubleBuffer;
  int ScreenMapped;

#if defined( VTK_OPENGL_HAS_OSMESA )
  // OffScreen stuff
  OSMesaContext OffScreenContextId;
  void *OffScreenWindow;
#endif
};

InvisibleXOpenGLRenderWindow::InvisibleXOpenGLRenderWindow()
 : Superclass()
{

}

InvisibleXOpenGLRenderWindow::~InvisibleXOpenGLRenderWindow()
{

}

void InvisibleXOpenGLRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void InvisibleXOpenGLRenderWindow::CreateAWindow()
{
  XVisualInfo *v, matcher;
  XSetWindowAttributes attr;
  int x, y, width, height, nItems;
  XWindowAttributes winattr;
  XSizeHints xsh;

  xsh.flags = USSize;
  if ((this->Position[0] >= 0) && (this->Position[1] >= 0))
  {
    xsh.flags |= USPosition;
    xsh.x = static_cast<int> (this->Position[0]);
    xsh.y = static_cast<int> (this->Position[1]);
  }

  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  height = ((this->Size[1] > 0) ? this->Size[1] : 300);

  xsh.width = width;
  xsh.height = height;

  // get the default display connection
  if (!this->DisplayId)
  {
    this->DisplayId = XOpenDisplay(static_cast<char *> (NULL));
    if (this->DisplayId == NULL)
    {
      vtkErrorMacro(<< "bad X server connection. DISPLAY="
          << vtksys::SystemTools::GetEnv("DISPLAY") << "\n");
    }
    this->OwnDisplay = 1;
  }

  attr.override_redirect = False;
  if (this->Borders == 0.0)
  {
    attr.override_redirect = True;
  }

  // create our own window ?
  this->OwnWindow = 0;
  if (!this->WindowId)
  {
    v = this->GetDesiredVisualInfo();
    this->ColorMap = XCreateColormap(this->DisplayId, XRootWindow(
        this->DisplayId, v->screen), v->visual, AllocNone);

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = this->ColorMap;
    attr.event_mask = StructureNotifyMask | ExposureMask;

    // get a default parent if one has not been set.
    if (!this->ParentId)
    {
      this->ParentId = XRootWindow(this->DisplayId, v->screen);
    }
    this->WindowId = XCreateWindow(this->DisplayId, this->ParentId, x, y,
        static_cast<unsigned int> (width), static_cast<unsigned int> (height),
        0, v->depth, InputOutput, v->visual, CWBackPixel | CWBorderPixel
            | CWColormap | CWOverrideRedirect | CWEventMask, &attr);
    XStoreName(this->DisplayId, this->WindowId, this->WindowName);
    XSetNormalHints(this->DisplayId, this->WindowId, &xsh);
    this->OwnWindow = 1;
  }
  else
  {
    XChangeWindowAttributes(this->DisplayId, this->WindowId,
        CWOverrideRedirect, &attr);
    XGetWindowAttributes(this->DisplayId, this->WindowId, &winattr);
    matcher.visualid = XVisualIDFromVisual(winattr.visual);
    matcher.screen = XDefaultScreen(DisplayId);
    v = XGetVisualInfo(this->DisplayId, VisualIDMask | VisualScreenMask,
        &matcher, &nItems);
  }

  if (this->OwnWindow)
  {
    // RESIZE THE WINDOW TO THE DESIRED SIZE
    vtkDebugMacro(<< "Resizing the xwindow\n");
    XResizeWindow(
        this->DisplayId,
        this->WindowId,
        ((this->Size[0] > 0) ? static_cast<unsigned int> (this->Size[0]) : 300),
        ((this->Size[1] > 0) ? static_cast<unsigned int> (this->Size[1]) : 300));
    XSync(this->DisplayId, False);
  }

  // is GLX extension is supported?
  if (!glXQueryExtension(this->DisplayId, NULL, NULL))
  {
    vtkErrorMacro("GLX not found.  Aborting.");
    if (this->HasObserver(vtkCommand::ExitEvent))
    {
      this->InvokeEvent(vtkCommand::ExitEvent, NULL);
      return;
    }
    else
    {
      abort();
    }
  }

  if (!this->Internal->ContextId)
  {
    this->Internal->ContextId
        = glXCreateContext(this->DisplayId, v, 0, GL_TRUE);
  }

  if (!this->Internal->ContextId)
  {
    vtkErrorMacro("Cannot create GLX context.  Aborting.");
    if (this->HasObserver(vtkCommand::ExitEvent))
    {
      this->InvokeEvent(vtkCommand::ExitEvent, NULL);
      return;
    }
    else
    {
      abort();
    }
  }

  // DO NOT MAP THE WINDOW (do not call XMapWindow()!)

  // free the visual info
  if (v)
  {
    XFree(v);
  }
  this->Mapped = 1;
  this->Size[0] = width;
  this->Size[1] = height;
}
