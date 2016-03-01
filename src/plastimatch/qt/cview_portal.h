/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _cview_portal_h_
#define _cview_portal_h_

#include <QtGui/QGraphicsView>

class Volume;

class PortalWidget : public QGraphicsView
{
    private:
    class ScaleHandler
    {
        public:
            bool wheelMode;     /* mouse wheel scaling toggle */
            float window;       /* determined by portal size  */
            float user;         /* determined by user         */
            float factor () { return window*user; }
            ScaleHandler ()
            {
                wheelMode = false;
                window    = 1.0;
                user      = 1.0;
            };
    };

    /****************************************************************/

    Q_OBJECT  /* Needed for QT signals/slots */

    public:
        enum PortalViewType {
            Axial,
            Coronal,
            Sagittal
        };

    private:
        /* Qt4 rendering */
        QGraphicsScene* scene;      /* Portal's QGraphicsScene    */
        uchar* fb;                  /* Framebuffer for drawing    */
        QImage* slice;              /* QImage tied to Framebuffer */
        QPixmap pmap;               /* QPixmap for rendering      */
        QGraphicsPixmapItem* pmi;   /* Needed to update QPixmap   */

        /* Image Attributes */
        Volume* vol;            /* Plastimatch volume                    */
        float min_intensity;    /* Min floating point value from volume  */
        float max_intensity;    /* Max floating point value from volume  */
        int stride[3];          /* volume strides for current view type  */

        /* Portal Geometry */
        int ijk_max[3];         /* index limits for current view type    */
        int current_slice;      /* Current slice index within volume     */
        int dim[2];             /* rendering surface dims (in pixels)    */
        float res[2];           /* portal resolution (in mm per pixel)   */
        float spacing[3];       /* voxel spacing in slice (in mm)        */
        float origin[3];        /* volume slice origin (in mm)           */
        PortalViewType view;    /* Axial or Coronal or Sagittal          */
        ScaleHandler scale;

        /* Scroll/Panning */
        bool pan_mode;          /* is panning mode enabled?              */
        int pan_xy[2];          /* tracking variables for panning        */
        int view_center[2];     /* point in scene that is in view center */

        /* HUD */
        bool hud_mode;
        float hud_xyz[3];
        int hud_ijk[3];
        QGraphicsTextItem* textPhy;
        QGraphicsTextItem* textVox;

    public:
        PortalWidget (QWidget *parent = 0);
        int getNumSlices () { return ijk_max[2]; }

    private:
        int getPixelValue (float* ij);
        void updateCursor (const QPoint& view_ij);
        void updateHUD ();
        void setWindowScale ();
        void doZoom (int step);
        void doScale (float step);
        void li_clamp_2d (int *ij_f, int *ij_r, float *li_1, float *li_2, float *ij);

    signals:
        void targetChanged (float* xyz);                /* on cursor change */
        void sliceChanged (int n);                      /* on slice change  */

    public slots:
        void setVolume (Volume* vol);               /* Attach volume to portal   */
        void detachVolume () { vol = NULL; }        /* Detach volume from portal */
        void setView (enum PortalViewType view);    /* Set portal view type      */
        void setTarget (float* xyz);                /* Set the cursor RS coords  */
        void resetPortal ();                        /* Removes user transforms   */
        void renderSlice (int slice_num);           /* Render slice_num          */

    protected:
        void wheelEvent (QWheelEvent *event);
        void keyPressEvent (QKeyEvent *event);
        void keyReleaseEvent (QKeyEvent *event);
        void mousePressEvent (QMouseEvent *event);
        void mouseReleaseEvent (QMouseEvent *event);
        void mouseMoveEvent (QMouseEvent *event);
        void resizeEvent (QResizeEvent *event);
};

#endif
