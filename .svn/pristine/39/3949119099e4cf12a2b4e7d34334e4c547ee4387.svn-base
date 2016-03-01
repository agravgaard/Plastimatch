/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "fileload.h"

int compare(const void *a, const void *b)
{
    const char ** str1 = (const char **) a;
    const char ** str2 = (const char **) b;

    return(strcmp(*str1, *str2));
}

Ise_Error fileload_init(FileloadInfo *fl, unsigned int mode)
{
    //set it to null for clean start
    memset(fl, 0, sizeof(FileloadInfo));

    //in matrox_source, if the mode is fluoro, then it is
    //necessary to allocate resource to the application
    //identifiers. However, in bitflow, this is not nessary.
    //so we return ISE_SUCCESS irregardless of the mode
    return ISE_SUCCESS;
}

Ise_Error fileload_open(FileloadInfo *fl)
{
    // common dialog box structure
    OPENFILENAME ofn;
    // buffer for file name
    char szFile[MAX_PATH];
    // buffer for file name
    HWND hwnd = NULL;              

    char load_dir[MAX_PATH];
    char load_pat[MAX_PATH];
    unsigned int i, slash_i;
    struct _finddata_t fileinfo;
    long hf;
    int id_base = 0;
    double timestamp_base = 0;
    int prev_dark = 0;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;

    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Raw (*.raw)\0*.RAW\0All (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn)==TRUE) 
    {
	//fileload_load_frames (ofn.lpstrFile);
        // Get the directory 
        slash_i = 0;
        
        for (i=0; ofn.lpstrFile[i]; i++) 
	    if (ofn.lpstrFile[i] == '\\') 
	        slash_i = i;
        
        if (!slash_i)
	    return ISE_FILE_OPEN_FAILED;
        
        strncpy (load_dir, ofn.lpstrFile, slash_i);
        load_dir[slash_i] = '\0';

        // Enumerate the directory, looking for raw files
        sprintf (load_pat, "%s\\*.raw", load_dir);
        
        hf = _findfirst (load_pat, &fileinfo);

        if (hf == -1L) 
	    return ISE_FILE_FIND_FAILED;

        fl->nImages[0] = 0;
        fl->nImages[1] = 0;
        fl->curIdx[0] = 0;
        fl->curIdx[1] = 0;
        
        // Obtain image size information
        if (fileinfo.size == sizeof(unsigned short)*HIRES_IMAGE_WIDTH*HIRES_IMAGE_HEIGHT)
        {
            fl->sizeX = HIRES_IMAGE_WIDTH;
            fl->sizeY = HIRES_IMAGE_HEIGHT;
        }
        else if (fileinfo.size == sizeof(unsigned short)*LORES_IMAGE_WIDTH*LORES_IMAGE_HEIGHT)
        {
            fl->sizeX = LORES_IMAGE_WIDTH;
            fl->sizeY = LORES_IMAGE_HEIGHT;
        }
        else 
            return ISE_FILE_BAD_IMAGE_SIZE;

        // create the initial array to store file names
        fl->imFileList[0] = (char **) malloc(MAX_PATH * sizeof(char *));
        fl->imFileList[1] = (char **) malloc(MAX_PATH * sizeof(char *));
        
        if (fl->imFileList[0] == NULL || fl->imFileList[1] == NULL)
            return ISE_FILE_LIST_INIT_FAILED;

        for (i=0; i < MAX_PATH; i ++) 
        {
            fl->imFileList[0][i] = (char *) malloc(MAX_PATH * sizeof(char));
            fl->imFileList[1][i] = (char *) malloc(MAX_PATH * sizeof(char));

            if (fl->imFileList[0][i] == NULL || fl->imFileList[1][i] == NULL)
                return ISE_FILE_LIST_INIT_FAILED;
        }
        fl->maxNImages = MAX_PATH;

        // store all the filenames in the array
        do 
        {
	    int idx, rc;
	    unsigned long id;
	    double timestamp;

            // Parse filename.  Filename should be: panel_seq_timestamp.raw 
	    rc = sscanf (fileinfo.name, "%d_%d_%lg.raw", &idx, &id, &timestamp);
	    if (rc != 3) 
            {
	        // If not, fill in bogus values 
	        idx = 0;
	        id = id_base ++;
	        timestamp = timestamp_base += 1.0 / 7.5;
	    }
            
            if (fl->nImages[idx] > fl->maxNImages) 
            { 
                //reallocate memory
                fl->imFileList[idx] = (char **) realloc(fl->imFileList[idx], sizeof(char*) * 2 * fl->maxNImages);
                if (fl->imFileList[idx] == NULL)
                    return ISE_FILE_LIST_GROW_FAILED;
                
                for (i=fl->maxNImages; i < 2 * fl->maxNImages; i ++)
                {
                    fl->imFileList[0][i] = (char *) malloc(sizeof(char) * MAX_PATH);
                    fl->imFileList[1][i] = (char *) malloc(sizeof(char) * MAX_PATH);

                    if (fl->imFileList[0][i] == NULL || fl->imFileList[1][i] == NULL)
                        return ISE_FILE_LIST_GROW_FAILED;
                }
                //update maxNImages
                fl->maxNImages *= 2;
            }
            // store file names
            strncpy(fl->imFileList[idx][fl->nImages[idx]++], fileinfo.name, MAX_PATH);
        }
        while (_findnext (hf, &fileinfo) == 0);

        _findclose(hf);

        if (fl->nImages[0] > 0 && fl->nImages[1] > 0)
            fl->nPanels = 2;

        // sort the filename list -- _filefirst and _filenext not working properly
        // for files stored on the network drive
        if (fl->nImages[0] > 0) 
            qsort(fl->imFileList[0], fl->nImages[0], sizeof(char*), compare);

        if (fl->nImages[0] > 0) 
            qsort(fl->imFileList[1], fl->nImages[1], sizeof(char*), compare);

    } // end if

    return ISE_SUCCESS;
}

Ise_Error fileload_load_image (unsigned short *img, FileloadInfo *fl, int idx)
{
    int rc;
    FILE* fp;

    // open the current raw image file
    debug_printf("open %s \n", fl->imFileList[idx][fl->curIdx[idx]]);
    if (!(fp = fopen(fl->imFileList[idx][fl->curIdx[idx]], "rb"))) {
	return ISE_FILE_OPEN_FAILED;
    }

    rc = fread (img, sizeof(unsigned short), fl->sizeX*fl->sizeY, fp);
    fclose (fp);
	fp = fopen("test.raw","wb");
	if (!fp) {
	    fprintf (stdout, "Couldn't open tmp1.raw\n");
	}
	fwrite (img, 2, fl->sizeX * fl->sizeY, fp);
	fclose (fp);


    if (rc == fl->sizeX * fl->sizeY) 
	return ISE_SUCCESS;
    else 
	return ISE_FILE_READ_ERROR;
    
}

void fileload_shutdown(FileloadInfo *fl)
{
    unsigned long i;

    // free list of file names
    for (i = 0; i< fl->maxNImages; i ++){
        if (! fl->imFileList[0][i]) free(fl->imFileList[0][i]);
        if (! fl->imFileList[1][i]) free(fl->imFileList[1][i]);
    }

    if (!fl->imFileList[0]) free(fl->imFileList[0]);
    if (!fl->imFileList[1]) free(fl->imFileList[1]);
}
