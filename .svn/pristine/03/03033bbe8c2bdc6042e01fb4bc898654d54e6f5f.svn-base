/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _iqt_video_widget_h_
#define _iqt_video_widget_h_

#include <QGraphicsView>
#include <QRubberBand>
#include <QPixmap>
#include <QtGui/QGraphicsView>
#include <QGraphicsScene>
#include <QTime>

class Iqt_video_widget : public QGraphicsView {
    Q_OBJECT

private:

    QGraphicsScene* scene;
    QPixmap pmap;
    QTime *time;
    QTimer *ping_check;
    QString filename;
    QString empty;
    QPixmap *qp1;
    QPixmap *qp2;
    int ping_pong;

public slots:
    void load(const QString& filename);
    void flick (void);
    void stop (void);
    void play (bool playing);
    void rescale (void);

public:
    Iqt_video_widget (QWidget *parent = 0);
    QGraphicsPixmapItem* pmi;
    int j;
    QGraphicsRectItem *tracker;
    QRectF trace;
    bool drawing;
    bool hasRect;
    bool manual;
    QPoint origin;
    QPointF pix;
    QGraphicsTextItem *trackPoint;
    QRubberBand* rubberband;
    ~Iqt_video_widget ();
    void set_qimage (const QImage& qimage);
    void updateTracking (void);

protected:
    //void paintEvent (QPaintEvent *event);
    QPointF currentCenter;
    QPointF GetCenter() { return currentCenter; }
    void SetCenter(const QPointF& centerPoint);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    //virtual void resizeEvent(QResizeEvent* event);

public:
    QPixmap m_pixmap;
};
#endif
