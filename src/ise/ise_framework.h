/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ise_framework_h__
#define __ise_framework_h__

int
ise_startup (unsigned long mode, 
		 int num_panels, 
		 char *client_ip_1,
		 char *server_ip_1,
		 int board_1,
		 int flip_1,
		 unsigned int num_frames_1,
		 double framerate_1,
		 char *client_ip_2,
		 char *server_ip_2,
		 int board_2,
		 int flip_2,
		 unsigned int num_frames_2,
		 double framerate_2);
void ise_shutdown (void);

int
image_source_init (IseFramework* ig, 
		int idx,
		char* ip_client,
		char* ip_server,
		unsigned int board_no,
		unsigned int rotate,
		unsigned int track,
		unsigned int num_frames,
		double framerate
		);
int ise_grab_grab_image (IseFramework* ig, int idx, unsigned char* buffer);
int ise_grab_set_igpax_fluoro (IseFramework* ig, double framerate);
int ise_grab_grab_fluoro_autosense (IseFramework* ig);
void ise_grab_close (IseFramework* ig);
int ise_grab_enable_radiographic_autosense (IseFramework* ig, int idx);
int ise_grab_warmup (IseFramework* ig, int idx);
void kill_igpax (void);
int ise_grab_start_capture_threads (IseFramework* ig, void (*notify)(int));
void ise_grab_configure_writing (IseFramework* ig, int write_flag, int write_dark);

void ise_grab_get_resolution (IseFramework* ig, int* h, int* w);

int ise_grab_lock_next_writable_frame (IseFramework* ig, int idx, Frame** framep);

int ise_grab_continuous_start (IseFramework* ig, void (*notify)(int));
int ise_grab_continuous_stop (IseFramework* ig);
void ise_grab_set_source (IseFramework* ig, unsigned long new_source);

#endif
