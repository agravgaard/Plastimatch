/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <QtGui>
#include <QTimer>
#if defined (commentout)
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include "vtkSmartPointer.h"
#endif

#include "iqt_main_window.h"

/* Some hints on displaying video.  It may be necessary to explore 
   multiple options.  Start with the QWidget option?

   Use a QGraphicsItem, which you put in a QGraphicsScene
   http://www.qtforum.org/article/35428/display-live-camera-video-in-a-graphics-item.html
   Use glTexture
   http://www.qtforum.org/article/20311/using-opengl-to-display-dib-bmp-format-from-video-for-windows.html
   Use a QWidget
   http://sourceforge.net/projects/qtv4lcapture/
   http://qt-apps.org/content/show.php/Qt+Opencv+webcam+viewer?content=89995
   http://doc.trolltech.com/qq/qq16-fader.html
   Use phonon
   http://doc.qt.nokia.com/latest/phonon-overview.html
*/

Iqt_main_window::Iqt_main_window ()
{
    /* Sets up the GUI */
    setupUi (this);

    /* Start the timer */
    m_qtimer = new QTimer (this);
    connect (m_qtimer, SIGNAL(timeout()), this, SLOT(slot_timer()));
    m_qtimer->start(10000);

    /* Render a sphere ?? */
    //this->render_sphere ();
}

#if defined (commentout)
void
Iqt_main_window::render_sphere ()
{
    // sphere
    vtkSmartPointer<vtkSphereSource> sphereSource = 
	vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->Update();
    vtkSmartPointer<vtkPolyDataMapper> sphereMapper =
	vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
    vtkSmartPointer<vtkActor> sphereActor = 
	vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
 
    // VTK Renderer
    vtkSmartPointer<vtkRenderer> renderer = 
	vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(sphereActor);
 
    // VTK/Qt wedded
    this->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);
};
#endif

Iqt_main_window::~Iqt_main_window ()
{
    QSettings settings;
    settings.sync ();

    delete m_qtimer;
}

void
Iqt_main_window::slot_load ()
{
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_load() was called"));
}

void
Iqt_main_window::slot_timer ()
{
    //this->video_widget;
#if defined (commentout)
    QMessageBox::information (0, QString ("Info"), 
	QString ("slot_timer() was called"));
#endif
}
