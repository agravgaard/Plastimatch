/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/*Qt Libraries*/
#include <QtGui>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QPen>
#include <QPixmap>
#include <QRect>
#include <QRubberBand>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QWheelEvent>
/*Other Libraries*/
#include "ise_config.h"
#include "iqt_video_widget.h"
#include "sleeper.h"
#include <stdio.h>

Iqt_video_widget::Iqt_video_widget (QWidget *parent)
    : QGraphicsView (parent)
{
    scene = new QGraphicsScene;
    this->setScene (scene);
    pmi = new QGraphicsPixmapItem (QPixmap());
    scene->addItem(pmi);
    ping_pong = 0;
    qp1 = qp2 = 0;
    drawing = false;
    qp1 = new QPixmap;
    rubberband = 0;

    tracker = new QGraphicsRectItem;
    trackPoint = new QGraphicsTextItem;

    ping_check = new QTimer (this);
    // connect (ping_check, SIGNAL(timeout()), this, SLOT(flick()));
    // ping_check->start(500);
    this->hasRect = false;
    SetCenter(QPointF(500.0, 500.0));
    //QGraphicsRectItem *rect_item 
    //  = scene->addRect (QRectF (20, 20, 10, 10));
    show();
}

void Iqt_video_widget::SetCenter (const QPointF& centerPoint) {
    QRectF visibleArea = mapToScene(rect()).boundingRect();
    QRectF sceneBounds = sceneRect();
    double boundX = visibleArea.width() / 2.0;
    double boundY = visibleArea.height() / 2.0;
    double boundwidth = sceneBounds.width() - 2.0 * boundX;
    double boundheight = sceneBounds.height() - 2.0 * boundY;
    QRectF bounds(boundX, boundY, boundwidth, boundheight);

    if(bounds.contains(centerPoint)) {
        this->currentCenter = centerPoint;
    } else {
        if(visibleArea.contains(sceneBounds)) {
	    this->currentCenter = sceneBounds.center();
	} else {
	    currentCenter = centerPoint;
	    if (centerPoint.x() > bounds.x() + bounds.width()) {
	      currentCenter.setX(bounds.x() + bounds.width());
	    } else if (centerPoint.x() < bounds.x()) {
	      currentCenter.setX(bounds.x());
	    }
	    if (centerPoint.y() > bounds.y() + bounds.height()) {
	      currentCenter.setY(bounds.y() + bounds.height());
	    } else if (centerPoint.y() < bounds.y()) {
	      currentCenter.setY(bounds.y());
	    }
	}
	centerOn(currentCenter);
    }
}

void Iqt_video_widget::mousePressEvent(QMouseEvent* event)
{
    this->origin = event->pos();
    if (event->button() == 1) {
        if (!rubberband)	       
        {
            rubberband = new QRubberBand(QRubberBand::Rectangle, this);
  	    this->drawing = true;
        }
        rubberband->setGeometry(QRect(origin, QSize()));
        rubberband->show();
    }
}

void Iqt_video_widget::mouseMoveEvent(QMouseEvent* event)
{
    if (drawing)
    {
        rubberband->setGeometry(QRect(origin, event->pos()).normalized());
    }
}

void Iqt_video_widget::mouseReleaseEvent(QMouseEvent* event)
{
    QPointF originf = mapToScene(origin);
    if (event->button()==1) {
	qDebug("Top: %d\nRight: %d\nWidth: %d\nHeight: %d", origin.x(), origin.y(), rubberband->width(), rubberband->height()); 
	QPoint dest = origin + QPoint (rubberband->width(), rubberband->height());
	QPointF destf = mapToScene(dest);
	this->fitInView(originf.x(), originf.y(), destf.x() - originf.x(),
			destf.y() - originf.y(), Qt::KeepAspectRatio);
	rubberband->hide();
    }
    if (event->button()==2) {

        qDebug() << "originf = " << originf;
        qDebug() << "boundingRect = " << pmi->boundingRect();
        QRectF br = pmi->boundingRect();

        this->pix = QPointF(
		     (1 - (br.width() - originf.x()) / br.width()) * 512 + 0.5,
		     (1 - (br.height() - originf.y()) / br.height()) * 512 + 0.5);

        qDebug() << "pix = " << pix;
	
        this->trace.setRect((originf.x()-5), (originf.y()-5), 10, 10);
	this->manual = true;
	updateTracking();
    }
    if (event->button()==4) {
	this->rescale();
    }
}

void Iqt_video_widget::rescale()
{
    QRectF va = mapToScene(rect()).boundingRect();
    QRectF br = pmi->boundingRect();
    double sf = (va.height())/(br.height());
    fitInView(va, Qt::KeepAspectRatio);
    scale(sf, sf);
}

void Iqt_video_widget::load(const QString& filename) {
    
    qp1 = new QPixmap (filename);
    QString filename_2 = QFileInfo (filename).path() + "/test2.png";
    qp2 = new QPixmap (filename_2);

    this->filename = filename;
}

void Iqt_video_widget::wheelEvent(QWheelEvent* event)
{
    QRectF visibleArea = mapToScene(rect()).boundingRect();
    QPointF pointBeforeScale(mapToScene(event->pos()));
    QPointF screenCenter = GetCenter();
    double scaleFactor = 1.15;
    if (event->delta() > 0)
	{
	    if (visibleArea.height() < 25) {
		return;
	    } else {
		scale(scaleFactor, scaleFactor);
	    }
	} else {
	if (visibleArea.height() > pmi->boundingRect().height()) {
	    return;
	} else {
	    scale(1.0/scaleFactor, 1.0/scaleFactor);
	}
    }
    QPointF pointAfterScale(mapToScene(event->pos()));
    QPointF offset = pointBeforeScale - pointAfterScale;
    QPointF newCenter = screenCenter + offset;
    SetCenter (newCenter);
}

void Iqt_video_widget::flick(void)
{
    if (!filename.isNull()) {
        delete pmi;                       //remove old pmi (IS necessary)
        if (ping_pong == 0) {
            pmi = new QGraphicsPixmapItem(*qp1);//set loaded image as new pmi
            ping_pong = 1;
        } else {
            pmi = new QGraphicsPixmapItem(*qp2);//set loaded image as new pmi
            ping_pong = 0;
        }
        scene->addItem(pmi);                //add new image to scene

    } else {
        return;
    }
}

void Iqt_video_widget::stop ()
{
    ping_check->stop();
}

void Iqt_video_widget::play (bool playing)
{   
    if (playing) {
        ping_check->start(500);
    } else {
        ping_check->stop();
    }
}

void
Iqt_video_widget::updateTracking()
{
    delete trackPoint;
    
    if (!manual) {
	//QPointF originf = mapToScene (origin);
	QRectF br = this->pmi->boundingRect();

	//this->pix = QPointF(
	//	(1 - (br.width() - originf.x()) / br.width()) * 512 + 0.5,
	//	(1 - (br.height() - originf.y()) / br.height()) * 512 + 0.5);

	QPointF originf;
	originf = QPointF (
		    ((pix.x()+0.5)/512 - 1) * br.width() + br.width(),
	            ((pix.y()+0.5)/512 - 1) * br.height() + br.height());
	this->trace.setRect ((originf.x()-5), (originf.y()-5), 10, 10);
    }
    
    if (!hasRect) {
	this->hasRect = true;
    } else {
	delete tracker;
	tracker = new QGraphicsRectItem;
    }
    this->tracker
	= scene->addRect (trace);
    tracker->setPen(QColor(255,0,0));
    tracker->setZValue (1);

    trackPoint = new QGraphicsTextItem;
    trackPoint->setDefaultTextColor (Qt::red);
    scene->addItem (trackPoint);
    

    trackPoint->setPlainText (QString("Tracking Point:  %1 ,  %2")
			      .arg((int)pix.x())
			      .arg((int)pix.y()));
    trackPoint->setPos (0,0); //looks like an owl
    trackPoint->setZValue (1);

}

Iqt_video_widget::~Iqt_video_widget ()
{
    delete qp1;
    delete qp2;
    delete pmi;
    delete ping_check;
    delete trackPoint;
    delete tracker;
}

void 
Iqt_video_widget::set_qimage (const QImage& qimage)
{
    if(qimage.isNull()) return;

    delete pmi;
		
    pmi = new QGraphicsPixmapItem(QPixmap::fromImage
				  (qimage.scaled(this->size(),
						 Qt::KeepAspectRatio)));
    scene->addItem(pmi);
    if (pix.isNull()) return;
    updateTracking();
}
