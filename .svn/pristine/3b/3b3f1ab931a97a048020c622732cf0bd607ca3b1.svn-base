/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _iqt_application_h_
#define _iqt_application_h_

#include <QApplication>
#include <QMutex>
#include <QWaitCondition>

#define ise_app ((Iqt_application *) qApp)

class Cbuf;
class Fluoro_source;
class Iqt_main_window;

class Iqt_application : public QApplication {
    Q_OBJECT
    ;
public:
    Iqt_application (int& argc, char* argv[]);
    ~Iqt_application ();

public:
    void set_synthetic_source (
        Iqt_main_window *mw,
        int rowset, int colset, double ampset, int markset, int noiset);
    void stop ();
    QWaitCondition frameLoaded;

public:
    int num_panels;
    QMutex mutex;
    Cbuf **cbuf;
    Fluoro_source *fluoro_source;
};
#endif
