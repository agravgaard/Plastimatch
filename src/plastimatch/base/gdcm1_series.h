/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gdcm1_series_h_
#define _gdcm1_series_h_

#include "plmbase_config.h"
#include "sys/plm_int.h"
#include <map>
#include <list>
#include <vector>
#include "metadata.h"

class Rt_study_metadata;

/* Forward declarations */
namespace gdcm {
    class File;
    typedef std::vector<File* > FileList;
    class SerieHelper2;
};

class PLMBASE_API Gdcm_series 
{
public:
    Gdcm_series ();
    ~Gdcm_series ();

    void load (const char *dicom_dir);
    void digest_files (void);
    void get_slice_info (int *slice_no, std::string *ct_slice_uid, float z);
    gdcm::File *get_ct_slice (void);
    void get_slice_uids (Rt_study_metadata *rsm);
    std::string get_patient_position ();
    const std::string& get_rtdose_filename ();
    const std::string& get_rtstruct_filename ();
    void get_metadata (const Metadata::Pointer& meta);

    gdcm::SerieHelper2 *m_gsh2;

    int m_have_ct;
    gdcm::FileList *m_ct_file_list;
    gdcm::FileList *m_rtdose_file_list;
    gdcm::FileList *m_rtstruct_file_list;
    plm_long m_dim[3];
    double m_origin[3];
    double m_spacing[3];
};
#endif
