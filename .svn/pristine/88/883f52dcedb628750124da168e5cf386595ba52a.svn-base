/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <stdio.h>
#include "ise.h"
#include "ise_config.h"

#define BUFLEN 1024

Globals globals;

void
gs_strncpy (char* dst, char* src, int n)
{
    int i = 0;
    for (i = 0; i < n; i++) {
        if (!(dst[i] = src[i])) {
            return;
        }
    }
    dst[i-1] = 0;
}

/* Return pointer to first non-whitespace character */
char*
trim_left (char* s)
{
    while (isspace(*s)) s++;
    return s;
}

/* Truncate whitespace at end of string */
void
trim_right (char* s)
{
    int j;
    for (j = strlen(s)-1; j >= 0; j--) {
	if (isspace(s[j])) {
	    s[j] = 0;
	}
    }
}

void
save_globals (void)
{
    FILE* fp;

    fp = fopen ("ise.cfg", "w");
    if (!fp) return;

    if (globals.sync_relays == SYNC_RELAY_1) {
	fprintf (fp, "sync_relays=1\n");
    } else if (globals.sync_relays == SYNC_RELAY_2) {
	fprintf (fp, "sync_relays=2\n");
    }

    if (globals.panel_select == USE_PANEL_1) {
	fprintf (fp, "panel_select=1\n");
    } else if (globals.panel_select == USE_PANEL_2) {
	fprintf (fp, "panel_select=2\n");
    } else if (globals.panel_select == USE_BOTH_PANELS) {
	fprintf (fp, "panel_select=both\n");
    }

    fprintf (fp, "buffer_num_frames=%d\n", globals.buffer_num_frames);

    fclose (fp);
}

void
interpret_line (char* key, char* val)
{
    if (!strcmp (key, "sync_relays")) {
	if (!strcmp (val, "1")) {
	    globals.sync_relays = SYNC_RELAY_1;
	}
	else if (!strcmp (val, "2")) {
	    globals.sync_relays = SYNC_RELAY_2;
	}
    }
    if (!strcmp (key, "panel_select")) {
	if (!strcmp (val, "1")) {
	    globals.panel_select = USE_PANEL_1;
	    globals.num_panels = 1;
	}
	else if (!strcmp (val, "2")) {
	    globals.panel_select = USE_PANEL_2;
	    globals.num_panels = 1;
	}
	else if (!strcmp (val, "both")) {
	    globals.panel_select = USE_BOTH_PANELS;
	    globals.num_panels = 2;
	}
	else {
	    /* Error */
	}
    }
    if (!strcmp (key, "buffer_num_frames")) {
	globals.buffer_num_frames = strtol (val, 0, 0);
	if (globals.buffer_num_frames <= 6) {
	    globals.buffer_num_frames = 6;
	}
    }
}

void
load_globals (void)
{
    FILE* fp;
    char buf[BUFLEN];
    char *b, *s;

    fp = fopen ("ise.cfg", "r");
    if (!fp) return;

    while (fgets (buf, BUFLEN, fp)) {
	b = trim_left(buf);
	if (*b == '#') continue;
	s = strstr (b, "=");
	if (!s) continue;
	*s++ = 0;
	s = trim_left (s);
	trim_right (b);
	trim_right (s);

	interpret_line (b, s);
    }

    fclose (fp);
}

void
init_globals_default (void)
{
    /* Program logic */
    globals.quit = 0;
    globals.notify[0] = 0;
    globals.notify[1] = 0;

    /* Frame grabber */
    globals.have_matrox_hardware = 0;

    /* Communication with indico process */
    memset (&globals.indico_info, 0, sizeof(Indico_Info));

    /* Image panel config */
    globals.sync_relays = SYNC_RELAY_1;
    globals.panel_select = USE_BOTH_PANELS;
    globals.num_panels = 2;

    /* Program config */
    globals.buffer_num_frames = ISE_NUM_FRAMES;

    /* Program config */
    globals.program_state = PROGRAM_STATE_STOPPED;
    globals.hold_bright_frame = 0;
    globals.auto_window_level = 1;
    globals.drop_dark_frames = 1;
    globals.is_writing = 0;
    globals.gating_flag = 0;
    globals.tracking_flag = 0;
    globals.loadfrom_file = 0;

    /* Framework vars */
    globals.ig.num_idx = 0;
    globals.ig.fw = 0;
}

void
init_globals (void)
{
    init_globals_default ();
    load_globals ();
}
