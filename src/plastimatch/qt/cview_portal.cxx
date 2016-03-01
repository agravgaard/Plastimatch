/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <iostream>
#include <float.h>
#include <math.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QPainter>
#include <QWheelEvent>
#include <QGraphicsPixmapItem>

#include "cview_portal.h"
#include "plm_math.h"
#include "volume.h"

// TODO: * Clean up this file!
//       * Add: Multi-Volume / Layered Rendering System

/* This just determines the amount of black space
 * a portal has within it */
#define FIELD_RES 4096

/////////////////////////////////////////////////////////
// PortalWidget: public
//

PortalWidget::PortalWidget (QWidget *parent)
    : QGraphicsView (parent)
{
    scene = NULL;
    fb    = NULL;
    slice = NULL;
    pmi   = NULL;
    vol   = NULL;
    view  = Axial;
    min_intensity = FLT_MAX;
    max_intensity = FLT_MIN;
    current_slice = -1;
    pan_mode      = false;
    view_center[0] = FIELD_RES/2;
    view_center[1] = FIELD_RES/2;
    hud_mode = false;
    memset (hud_xyz, 0, 3*sizeof (float));
    memset (hud_ijk, 0, 3*sizeof (int));

    setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    scene = new QGraphicsScene (this);
    scene->setItemIndexMethod (QGraphicsScene::NoIndex);
    scene->setSceneRect (0, 0, FIELD_RES, FIELD_RES);
    scene->setBackgroundBrush (QBrush (Qt::black, Qt::SolidPattern));
    setScene (scene);

    textPhy = new QGraphicsTextItem;
    textVox = new QGraphicsTextItem;
    textPhy->setZValue (99);
    textVox->setZValue (99);
    textPhy->setDefaultTextColor (Qt::red);
    textVox->setDefaultTextColor (Qt::green);

    scene->addItem (textPhy);
    scene->addItem (textVox);
}


/////////////////////////////////////////////////////////
// PortalWidget: private
//

int
PortalWidget::getPixelValue (float* ij)
{
    int ij_f[2];
    int ij_r[2];
    float li_1[2], li_2[2];  /* linear interpolation fractions */
    float hfu;
    int idx;
    float* img = (float*) vol->img;

    li_clamp_2d (ij_f, ij_r, li_1, li_2, ij);

    idx = stride[2]*current_slice
        + stride[1]*(ij_f[1]+0)
        + stride[0]*(ij_f[0]+0);
    hfu = li_1[0] * li_1[1] * img[idx];

    idx = stride[2]*current_slice
        + stride[1]*(ij_f[1]+0)
        + stride[0]*(ij_f[0]+1);
    hfu += li_2[0] * li_1[1] * img[idx];

    idx = stride[2]*current_slice
        + stride[1]*(ij_f[1]+1)
        + stride[0]*(ij_f[0]+0);
    hfu += li_1[0] * li_2[1] * img[idx];

    idx = stride[2]*current_slice
        + stride[1]*(ij_f[1]+1)
        + stride[0]*(ij_f[0]+1);
    hfu += li_2[0] * li_2[1] * img[idx];


    /* remap to 8-bit grayscale */
    int scaled = floor ((hfu - min_intensity)
        / (max_intensity - min_intensity) * 255 );

    if (scaled > 255) {
        return 255;
    } else if (scaled < 0) {
        return 0;
    }

    return scaled;
}

void
PortalWidget::updateCursor (const QPoint& view_ij)
{
    QPointF scene_ij;       /* portal: scene coords */
    QPoint slice_ij;        /* slice: pixel coods */
    QPointF slice_xy;       /* slice: realspace coords */
    QPointF slice_offset;   /* scene coords of slice left edge */
    float slice_z;

    /* QGraphicsPixmapItem is derived from QGraphicsItem */
    if ((void*)pmi != (void*)itemAt (view_ij)) {
        return;
    } else if (!vol) {
        return;
    }

    scene_ij = mapToScene (view_ij);

    slice_offset.rx() = ((FIELD_RES - pmap.width())/2.0)*res[0];
    slice_offset.ry() = ((FIELD_RES - pmap.height())/2.0)*res[1];

    slice_xy.rx() = (scene_ij.x()*res[0]-slice_offset.x())/scale.factor() + origin[0];
    slice_xy.ry() = (scene_ij.y()*res[1]-slice_offset.y())/scale.factor() + origin[1];
    slice_z = (current_slice*spacing[2] + origin[2]);

    slice_ij.rx() = (slice_xy.x()-origin[0])/res[0];
    slice_ij.ry() = (slice_xy.y()-origin[1])/res[1];

    switch (view) {
    case Axial:
        hud_xyz[0] = slice_xy.x();
        hud_xyz[1] = slice_xy.y();
        hud_xyz[2] = slice_z;

        hud_ijk[0] = slice_ij.x()*res[0]/spacing[0];
        hud_ijk[1] = slice_ij.y()*res[1]/spacing[1];
        hud_ijk[2] = current_slice;
        break;
    case Coronal:
        hud_xyz[0] = slice_xy.x();
        hud_xyz[1] = slice_z;
        hud_xyz[2] = (-1.0)*slice_xy.y();

        hud_ijk[0] = slice_ij.x()*res[0]/spacing[0];
        hud_ijk[1] = current_slice;
        hud_ijk[2] = slice_ij.y()*res[1]/spacing[1];
        break;
    case Sagittal:
        hud_xyz[0] = slice_z;
        hud_xyz[1] = slice_xy.x();
        hud_xyz[2] = (-1.0)*slice_xy.y();

        hud_ijk[0] = current_slice;
        hud_ijk[1] = slice_ij.x()*res[0]/spacing[0];
        hud_ijk[2] = slice_ij.y()*res[1]/spacing[1];
        break;
    }
//    emit targetChanged (xyz[0], xyz[1], xyz[2]);

    updateHUD ();

#if 0
    std::cout << "--------------------------------------------\n"
              << "     View: " << view_ij.x() << "  "<< view_ij.y() << "\n"
              << "    Scene: " << scene_ij.x() << " " << scene_ij.y() << "\n"
              << "    Slice: " << slice_ij.x() << " " << slice_ij.y() << "\n"
              << "RealSpace: " << slice_xy.x() << " " << slice_xy.y() << "\n";
#endif
}

void
PortalWidget::updateHUD ()
{
    textPhy->setPlainText (QString ("Phy: %1 %2 %3")
                           .arg (hud_xyz[0], 0, 'f', 2)
                           .arg (hud_xyz[1], 0, 'f', 2)
                           .arg (hud_xyz[2], 0, 'f', 2));
    textVox->setPlainText (QString ("Vox: %1 %2 %3")
                           .arg (hud_ijk[0])
                           .arg (hud_ijk[1])
                           .arg (hud_ijk[2]));
    textPhy->setPos (mapToScene(0,0));
    textVox->setPos (mapToScene(0,this->height()-24));
}

void
PortalWidget::setWindowScale ()
{
    float tmp[2];

    tmp[0] = (float)size().height() / (float)dim[0];
    tmp[1] = (float)size().width() / (float)dim[1];

    if (tmp[0] < tmp[1]) {
        scale.window = tmp[0];
    } else {
        scale.window = tmp[1];
    }
}

void
PortalWidget::doZoom (int step)
{
    if ( (current_slice + step < ijk_max[2]) &&
         (current_slice + step >= 0)  ) {
        current_slice += step;
        renderSlice (current_slice);
    }
}

void
PortalWidget::doScale (float step)
{
    scale.user += step;
    renderSlice (current_slice);
}

void
PortalWidget::li_clamp_2d (
    int *ij_f,    /* Output: "floor" pixel in moving img in vox*/
    int *ij_r,    /* Ouptut: "round" pixel in moving img in vox*/
    float *li_1,  /* Output: Fraction for upper index voxel */
    float *li_2,  /* Output: Fraction for lower index voxel */
    float *ij     /* Input:  Unrounded pixel coordinates in vox */
)
{
    for (int n=0; n<2; n++) {
        float maff = floor (ij[n]);
        ij_f[n] = (int) maff;
        ij_r[n] = ROUND_INT (ij[n]);
        li_2[n] = ij[n] - maff;
        if (ij_f[n] < 0) {
            ij_f[n] = 0;
            ij_r[n] = 0;
            li_2[n] = 0.0f;
        } else if (ij_f[n] >= ijk_max[n]-1) {
            ij_f[n] = ijk_max[n] - 2;
            ij_r[n] = ijk_max[n] - 1;
            li_2[n] = 1.0f;
        }
        li_1[n] = 1.0f - li_2[n];
    }
}


/////////////////////////////////////////////////////////
// PortalWidget: protected
//

void
PortalWidget::wheelEvent (QWheelEvent *event)
{
    /* Note: delta is in "# of 8ths of a degree"
     *   Each wheel click is usually 15 degrees on most mice */
    int step = event->delta() / (8*15);


    if ( scale.wheelMode ) {
        doScale (0.1*(float)step);
    } else {
        doZoom (step);
    }
}

void
PortalWidget::keyPressEvent (QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_1:
        setView (Axial);
        break;
    case Qt::Key_2:
        setView (Coronal);
        break;
    case Qt::Key_3:
        setView (Sagittal);
        break;
    case Qt::Key_J:
    case Qt::Key_Minus:
        doZoom (-1);
        break;
    case Qt::Key_K:
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        doZoom (1);
        break;
    case Qt::Key_BracketRight:
    case Qt::Key_L:
        doScale (+0.1f);
        break;
    case Qt::Key_BracketLeft:
    case Qt::Key_H:
        doScale (-0.1f);
        break;
    case Qt::Key_R:
        resetPortal ();
        updateHUD ();
        break;
    case Qt::Key_Control:
        scale.wheelMode = true;
        break;
    default:
        /* Forward to default callback */
        QGraphicsView::keyPressEvent (event);
    }
}

void
PortalWidget::keyReleaseEvent (QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Control:
        scale.wheelMode = false;
        break;
    default:
        /* Forward to default callback */
        QGraphicsView::keyPressEvent (event);
    }
}

void
PortalWidget::mousePressEvent (QMouseEvent *event)
{
    QPoint view_ij;         /* portal: viewport coords */

    view_ij = event->pos();

    switch (event->button()) {
    case Qt::LeftButton:
        updateCursor (view_ij);
        updateHUD ();
        hud_mode = true;
        event->accept();
        break;
    case Qt::RightButton:
        scale.wheelMode = true;
        pan_mode = true;
        pan_xy[0] = view_ij.x();
        pan_xy[1] = view_ij.y();
        setCursor (Qt::ClosedHandCursor);
        event->accept();
        break;
    default:
        event->ignore();
        break;
    }
}

void
PortalWidget::mouseReleaseEvent (QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        hud_mode = false;
        event->ignore();
        break;
    case Qt::RightButton:
        scale.wheelMode = false;
        pan_mode = false;
        setCursor (Qt::ArrowCursor);
        event->accept();
        break;
    default:
        event->ignore();
        break;
    }
}

void
PortalWidget::mouseMoveEvent (QMouseEvent *event)
{
    int dx = event->pos().x() - pan_xy[0];
    int dy = event->pos().y() - pan_xy[1];

    if (pan_mode) {
        translate ( (double)dx, (double)dy);
        updateHUD ();
        view_center[0] -= dx;
        view_center[1] -= dy;
        pan_xy[0] = event->pos().x();
        pan_xy[1] = event->pos().y();
    }
    if (hud_mode) {
        updateCursor (event->pos());
        updateHUD ();
    }
    event->ignore();
}

void
PortalWidget::resizeEvent (QResizeEvent *event)
{
    centerOn (view_center[0], view_center[1]);
    setWindowScale ();
    renderSlice (current_slice);
    updateHUD ();
}


/////////////////////////////////////////////////////////
// PortalWidget: slots
//

void
PortalWidget::setVolume (Volume* vol)
{
    this->vol = vol;
    float* img = (float*) vol->img;
    
    /* Obtain value range */
    min_intensity = FLT_MAX;
    max_intensity = FLT_MIN;
    for (plm_long i=0; i<vol->npix; i++) {
        if ( img[i] < min_intensity ) {
            min_intensity = img[i];
        }
        if ( img[i] > max_intensity ) {
            max_intensity = img[i];
        }
    }

    /* Reset HUD */
    memset (hud_xyz, 0, 3*sizeof (float));
    memset (hud_ijk, 0, 3*sizeof (int));
    updateHUD ();

    /* Display the loaded volume */
    PortalWidget::setView (Axial);
}

void
PortalWidget::setView (enum PortalViewType view)
{
    /* Cannot setView with no volume attached to portal */
    if (!vol) { return; }

    /* Delete the old rendering surface */
    if (slice) {
        scene->removeItem (pmi);
        delete slice;
        free (fb);
    }

    this->view = view;

    /* Change coordinate systems */
    switch (view) {
    case Axial:
        ijk_max[0] = vol->dim[0];
        ijk_max[1] = vol->dim[1];
        ijk_max[2] = vol->dim[2];

        stride[0] = 1;
        stride[1] = vol->dim[0];
        stride[2] = vol->dim[1] * vol->dim[0];

        spacing[0] = fabs (vol->spacing[0]);
        spacing[1] = fabs (vol->spacing[1]);
        spacing[2] = fabs (vol->spacing[2]);

        origin[0] = vol->origin[0];
        origin[1] = vol->origin[1];
        origin[2] = vol->origin[2];
        break;
    case Coronal:
        ijk_max[0] = vol->dim[0];
        ijk_max[1] = vol->dim[2];
        ijk_max[2] = vol->dim[1];

        stride[0] = 1;
        stride[1] = vol->dim[1] * vol->dim[0];
        stride[2] = vol->dim[0];

        spacing[0] = fabs (vol->spacing[0]);
        spacing[1] = fabs (vol->spacing[2]);
        spacing[2] = fabs (vol->spacing[1]);

        origin[0] = vol->origin[0];
        origin[1] = vol->origin[2];
        origin[2] = vol->origin[1];
        break;
    case Sagittal:
        ijk_max[0] = vol->dim[1];
        ijk_max[1] = vol->dim[2];
        ijk_max[2] = vol->dim[0];

        stride[0] = vol->dim[0];
        stride[1] = vol->dim[1] * vol->dim[0];
        stride[2] = 1;

        spacing[0] = fabs (vol->spacing[1]);
        spacing[1] = fabs (vol->spacing[2]);
        spacing[2] = fabs (vol->spacing[0]);

        origin[0] = vol->origin[1];
        origin[1] = vol->origin[2];
        origin[2] = vol->origin[0];
        break;
    default:
        exit (-1);
        break;
    }

    /* Rendering surface dimensions (pix) */
    dim[0] = floor (ijk_max[0]*spacing[0]);
    dim[1] = floor (ijk_max[1]*spacing[1]);
    setWindowScale ();

    /* Portal resolution (mm per pix) */
    res[0] = (spacing[0] * (float)ijk_max[0]) / (float)dim[0];
    res[1] = (spacing[1] * (float)ijk_max[1]) / (float)dim[1];

    /* Make a new rendering surface */
    fb = (uchar*) malloc (4*dim[0]*dim[1]*sizeof (uchar));
    slice = new QImage (fb, dim[0], dim[1], QImage::Format_ARGB32);
    pmi = scene->addPixmap (pmap);

    /* We always set the portal to the center slice after changing the view */
    renderSlice (ijk_max[2] / 2);
}

void
PortalWidget::setTarget (float* xyz)
{
    // set realspace coords
}

void
PortalWidget::resetPortal ()
{
    scale.user = 1.0;
    scale.wheelMode = false;
    pan_mode = false;
    view_center[0] = FIELD_RES/2;
    view_center[1] = FIELD_RES/2;
    centerOn (view_center[0], view_center[1]);
    renderSlice (current_slice);
}

void
PortalWidget::renderSlice (int slice_num)
{
    int i, j;                /* slice coords         */
    int p[2];                /* frame buffer coords  */
    float xy[2];             /* real space coords    */
    float ij[2];             /* volume slice indices */
    int shade;
    uchar* pixel;

    if (!vol) {
        return;
    }
    else if ((slice_num < 0) || (slice_num >= ijk_max[2])) {
        return;
    }

    current_slice = slice_num;

    /* Set slice pixels */
    for (j=0; j<dim[1]; j++) {
        xy[1] = res[1]*(float)j;
        ij[1] = xy[1] / spacing[1];
        for (i=0; i<dim[0]; i++) {
            xy[0] = res[0]*(float)i;
            ij[0] = xy[0] / spacing[0];

            shade = getPixelValue (ij);

            /* Deal with Qt's inverted y-axis... sometimes */
            if ((view == Coronal) || (view == Sagittal)) {
                p[0] = i;  p[1] = dim[1] - j - 1;
            } else {
                p[0] = i;  p[1] = j;
            }

            pixel = &fb[4*dim[0]*p[1]+(4*p[0])];
            pixel[0] = (uchar)shade;    /* BLUE  */
            pixel[1] = (uchar)shade;    /* GREEN */
            pixel[2] = (uchar)shade;    /* RED   */
            pixel[3] = 0xFF;            /* ALPHA */
        }
    }

    /* Have Qt render the framebuffer */
    pmap = QPixmap::fromImage (*slice);
    pmap = pmap.scaled (dim[0]*scale.factor(), dim[1]*scale.factor());
    pmi->setPixmap (pmap);
    pmi->setOffset (((FIELD_RES - pmap.width())/2), ((FIELD_RES- pmap.height())/2));

    emit sliceChanged (current_slice);
}
