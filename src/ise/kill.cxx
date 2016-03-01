/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
kill_process (const char* process_name)
{
    HANDLE snapshot;
    HANDLE process;
    PROCESSENTRY32 pe;
    BOOL process_found;
    BOOL rc;
    DWORD my_process_id;

    /* Make sure it isn't my
    process id! */

	/*QMessageBox msg;
	msg.setText("KillProcess");
	msg.exec();*/


    my_process_id = GetCurrentProcessId ();

    snapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
    process_found = Process32First (snapshot, &pe);
    while (process_found) {
        /* Check for exact match on process name */
	process_found = Process32Next (snapshot, &pe);
	if (_stricmp(pe.szExeFile, process_name)) {
            continue;
	}

        /* Make sure it isn't my process id! */
        if (pe.th32ProcessID == my_process_id) {
            continue;
        }

        /* Strangely, I seem to have enough priveleges to kill the 
           process, even without calling AdjustTokenPrivileges() */
        process = OpenProcess (PROCESS_ALL_ACCESS, 0, pe.th32ProcessID);
        if (!process) {
            printf ("Open failed\n");
            continue;
        }
        rc = TerminateProcess (process, 0);
        if (!rc) {
            printf ("Kill failed\n");
            continue;
        }
        printf ("Kill succeeded\n");
    }
    CloseHandle (snapshot);
}
