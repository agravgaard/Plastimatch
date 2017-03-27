/* -----------------------------------------------------------------------
See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <QApplication>
#include <QProcess>
#include <QThread>

#include "advantech.h"

#include "acquire_4030e_child.h"
#include "acquire_4030e_parent.h"
#include "kill.h"
#include <QString>

#include "YKOptionSetting.h"

class SleeperThread : public QThread {
public:
	static void msleep(unsigned long msecs) {
		QThread::msleep(msecs);
	}
};

//----------------------------------------------------------------------
//  main
//----------------------------------------------------------------------

#include <time.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <QString>

using namespace std;

int 
main (int argc, char* argv[])
{	
	Acquire_4030e_child *child[2];
	child[0] = NULL;
	child[1] = NULL;	


	/* During debugging, use hard-coded path */
	if (argc == 1)
	{ 
		/*** Parent process ***/

		/* Spawn child processes */
		Acquire_4030e_parent *parent //QCoreApplication		
			= new Acquire_4030e_parent (argc, argv);
		QString exePath = argv[0];
		if (!parent->initialize(exePath))
			return 0;

		/* Wait forever */
		printf ("Waiting forever.(parent)\n");
		parent->exec ();
		printf ("Wait complete.(parent)\n");
		//after event, before destroy parent

		if (child[0] != NULL) //Here, child is NULL!!
		{
			child[0]->quit();
			delete child[0];				
			child[0] = NULL;


		}
		if (child[1] != NULL)
		{
			child[1]->quit();
			delete child[1];				
			child[1] = NULL;
		}
	}

	if (argc == 3) //1:exe path, 2: child flag, 3:procIdx
	{
		if (!strcmp (argv[1], "--child"))
		{
			/*** Child process ***/
			printf ("A child is born.\n");		

			int panel_ldx = atoi(argv[2]);			

			if (panel_ldx < 2)
			{				
				child[panel_ldx]= new Acquire_4030e_child (argc, argv);

				if (!child[panel_ldx]->init(panel_ldx)) //1) DP creation. if fail --> death of program //2) open receptor
				{									
					delete child[panel_ldx];
					child[panel_ldx] = NULL;					
				}	    
				else
				{					
					child[panel_ldx]->exec(); //start event handling				
				}
			}			
		}		
	}	

#if defined (commentout)

	/* Start acquisition processes */
	QProcess process[2];
	for (int i = 0; i < 1; i++) {
		QString program = argv[0];
		QStringList arguments;
		arguments << "--child" << paths[i];
		process[i].start(program, arguments);

		connect (&process[i], SIGNAL(readyReadStandardOutput()),
			app, SLOT(log_output()));  

	}

	/* Wait (forever) for processes to complete */
	app.exec();
#endif

#if defined (commentout)
	QThread thread[2];
	Acquire_thread aq[2];
	for (int i = 0; i < 1; i++) {
		aq[i].idx = i;
		aq[i].open_receptor (paths[i]);
		aq[i].moveToThread (&thread[i]);
		thread[i].start ();
		QMetaObject::invokeMethod (&aq[i], "run", Qt::QueuedConnection);
	}

	/* Wait (forever) for threads to complete */
	app.exec();
	thread[0].wait();
	//thread[1].wait();
#endif

#if defined (commentout)
	Dips_panel dp;
	dp.open_panel (0, HIRES_IMAGE_HEIGHT, HIRES_IMAGE_WIDTH);

	/* Initialize link to panel */
	Varian_4030e vp;
	result = vp.open_receptor_link (path_1);
	result = vp.disable_missing_corrections (result);
	if (result != HCP_NO_ERR) {
		printf ("vp.open_receptor_link returns error (%d): %s\n", result, 
			Varian_4030e::error_string(result));
		return -1;
	}

	result = vp.check_link ();
	if (result != HCP_NO_ERR) {
		printf ("vp.check_link returns error %d\n", result);
		vip_close_link();
		return -1;
	}

	vp.print_sys_info ();

	result = vip_select_mode (vp.current_mode);

	if (result != HCP_NO_ERR) {
		printf ("vip_select_mode(%d) returns error %d\n", 
			vp.current_mode, result);
		vip_close_link();
		return -1;
	}

	while (true) {
#if defined (commentout)
		/* Wait for generator expose request */
		while (!advantech.ready_for_expose ()) {
			Sleep (10);
		}
#endif
		/* Get frame from panel */
		printf ("Waiting for image.\n");
		vp.rad_acquisition (&dp);
	}

	vip_close_link();
#endif

	return 0;
}
