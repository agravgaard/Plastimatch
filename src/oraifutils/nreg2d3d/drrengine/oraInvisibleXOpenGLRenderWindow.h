//
#ifndef ORAINVISIBLEXOPENGLRENDERWINDOW_H_
#define ORAINVISIBLEXOPENGLRENDERWINDOW_H_

#include <vtkXOpenGLRenderWindow.h>

/** \class InvisibleXOpenGLRenderWindow
 * \brief Realizes a really invisible X window for off-screen rendering in X.
 *
 * This helper class inherits from vtkXOpenGLRenderWindow and realizes a really
 * invisible (unmapped) X window suitable for off-screen rendering in X window
 * environments. In contrast to the superclass implementation, this class does
 * not call XMapWindow() during hardware off-screen window creation. This is
 * important if we do not want to "see" any window.
 *
 * \warning This class includes the re-definition of
 * vtkXOpenGLRenderWindowInternal in its implementation. This may cause
 * conflicts in future VTK releases - as soon as this class interface changes.
 *
 * @see vtkXOpenGLRenderWindow
 * 
 * @author phil 
 * @version 1.0
 */
class InvisibleXOpenGLRenderWindow:
    public vtkXOpenGLRenderWindow
{
public:
  /** VTK standard instantiation **/
  static InvisibleXOpenGLRenderWindow *New();
  /** type information **/
  vtkTypeRevisionMacro(InvisibleXOpenGLRenderWindow,vtkXOpenGLRenderWindow);
  /** object information **/
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  /** Default constructor. **/
  InvisibleXOpenGLRenderWindow();
  /** Destructor. **/
  ~InvisibleXOpenGLRenderWindow();

  /**
   * Create an X window. This window won't be mapped, and will therefore be
   * really invisible.
   **/
  virtual void CreateAWindow();

private:
  /** Purposely not implemented. **/
  InvisibleXOpenGLRenderWindow(const InvisibleXOpenGLRenderWindow &);
  /** Purposely not implemented. **/
  void operator=(const InvisibleXOpenGLRenderWindow &);

};

#endif /* ORAINVISIBLEXOPENGLRENDERWINDOW_H_ */
