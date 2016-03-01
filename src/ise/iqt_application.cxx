/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <QtGui>

#include "cbuf.h"
#include "fluoro_source.h"
#include "synthetic_source.h"
#include "iqt_application.h"

Iqt_application::Iqt_application (int& argc, char* argv[])
    : QApplication (argc, argv)
{
    this->num_panels = 1;
    this->cbuf = new Cbuf*[this->num_panels];
    
    for (int i = 0; i < this->num_panels; i++) {
        this->cbuf[i] = new Cbuf();
    }

    this->fluoro_source = 0;
}

Iqt_application::~Iqt_application ()
{
    for (int i = 0; i < this->num_panels; i++) {
        delete this->cbuf[i];
    }
    delete[] this->cbuf;

    if (fluoro_source) {
        delete fluoro_source;
    }
}

void
Iqt_application::set_synthetic_source (
    Iqt_main_window *mw,
    int rowset, int colset, double ampset, int markset, int noiset)
{
    qDebug("Set_synth Called");
    if (this->fluoro_source) {
        if (this->fluoro_source->get_type() == "Synthetic") {
            /* Already synthetic */

            qDebug("Restarting synthetic display..."); 

            qDebug("Restarting synthetic display...");
            /*unsigned long j = 0;
            for (j = 0; j < this->num_panels; j++)
            {
                this->cbuf[j]->clear ();
            }*/

	    /*perhaps find a way to call cbuf->cbuf_shutdown_queue()*/
	    //return;

        } else {
            /* Something else, so delete */
            delete this->fluoro_source;
        }
    }
    this->fluoro_source = new Synthetic_source (mw, colset, rowset, ampset, noiset);

    /* GCS FIX: For now, just a single cbuf */
    this->cbuf[0]->clear ();
    this->cbuf[0]->init (0, 10, colset, rowset);
   // this->cbuf[0]->init (0, 10, this->fluoro_source->get_size_x(cols), 
     //   this->fluoro_source->get_size_y(rows));
    this->fluoro_source->set_cbuf (this->cbuf[0]);
    this->fluoro_source->start ();
}

void
Iqt_application::stop ()
{
    this->fluoro_source->stop ();
}
