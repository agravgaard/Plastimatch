/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"

#include "itk_dicom_load.h"
#include "logfile.h"
#include "print_and_exit.h"

/* winbase.h defines GetCurrentTime which conflicts with gdcm function */
#if defined GetCurrentTime
# undef GetCurrentTime
#endif

#include "gdcmFile.h"
#if GDCM_MAJOR_VERSION < 2
#include "gdcmUtil.h"
#else
#include "gdcmUIDGenerator.h"
#endif

/* -----------------------------------------------------------------------
    Definitions
   ----------------------------------------------------------------------- */
typedef itk::ImageSeriesReader < CharImageType > DicomCharReaderType;
typedef itk::ImageSeriesReader < UCharImageType > DicomUCharReaderType;
typedef itk::ImageSeriesReader < ShortImageType > DicomShortReaderType;
typedef itk::ImageSeriesReader < UShortImageType > DicomUShortReaderType;
typedef itk::ImageSeriesReader < Int32ImageType > DicomInt32ReaderType;
typedef itk::ImageSeriesReader < UInt32ImageType > DicomUInt32ReaderType;
typedef itk::ImageSeriesReader < FloatImageType > DicomFloatReaderType;
typedef itk::ImageSeriesReader < DoubleImageType > DicomDoubleReaderType;


/* -----------------------------------------------------------------------
   functions
   ----------------------------------------------------------------------- */
#if GDCM_MAJOR_VERSION < 2
static bool
test_dicom_ok (const std::string& fn)
{
    gdcm::File header;
    header.SetLoadMode (0);
    header.SetFileName (fn);
    header.Load ();

    if (!header.IsReadable()) {
        return false;
    }
    std::string s;

    /* Reject GE Scouts */
    s = header.GetEntryValue (0x0018, 0x0022);
    if (s == "SCOUT MODE") {
        logfile_printf ("Rejecting GE scout\n");
        return false;
    }

    /* Reject GE Dose reports */
    s = header.GetEntryValue (0x0008, 0x103e);
    if (s == "Dose Report" || s == "Dose Report ") {
        logfile_printf ("Rejecting GE dose report\n");
        return false;
    }

    /* Reject RTDOSE, which can get interpreted as an image (and gets 
       read incorretly anyway).  Dose is read by rtds.cxx instead. */
    s = header.GetEntryValue (0x0008, 0x0060);
    if (s == "RTDOSE") {
        logfile_printf ("Rejecting RTDOSE\n");
        return false;
    }

    return true;
}
#endif

template<class T>
void
load_dicom_dir_rdr(T rdr, const char *dicom_dir)
{
    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer dicomIO = ImageIOType::New();
    rdr->SetImageIO (dicomIO);

    /* Read the filenames from the directory */
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

    nameGenerator->SetUseSeriesDetails (true);

    /* GCS 2011-09-16.  AddRestriction() causes seg fault when reading 
       DICOM files with empty fields.  Wow.  Anyway, we can't use them. 
       Possibly only with GDCM 1.X? */
#if GDCM_MAJOR_VERSION == 2
    /* Reject RTDOSE, which can get interpreted as an image (and gets 
       read incorretly anyway).  Dose is read by rtds.cxx instead. */
    gdcm::SerieHelper* gsh = nameGenerator->GetSeriesHelper ();
    gsh->AddRestriction (0x0008, 0x0060, "RTDOSE", gdcm::GDCM_DIFFERENT);
    /* Reject GE Scouts */
    gsh->AddRestriction (0x0018, 0x0022, "SCOUT MODE", gdcm::GDCM_DIFFERENT);
    /* Reject GE Dose reports */
    gsh->AddRestriction (0x0008, 0x103e, "Dose Report", gdcm::GDCM_DIFFERENT);
#endif

    nameGenerator->SetDirectory (dicom_dir);
    try {
        std::cout << std::endl << "The directory: " << std::endl;
        std::cout << std::endl << dicom_dir << std::endl << std::endl;
        std::cout << "Contains the following DICOM Series: ";
        std::cout << std::endl;

        typedef std::vector< std::string > SeriesIdContainer;
        const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
        SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
        SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
        while (seriesItr != seriesEnd) {
            std::cout << seriesItr->c_str() << std::endl;
            seriesItr++;
        }
        std::cout << std::endl;

        /* Loop through series and use first one that loads */
        seriesItr = seriesUID.begin();
        bool dicom_load_succeeded = false;
        while (!dicom_load_succeeded && seriesItr != seriesEnd) {
            std::string seriesIdentifier;
            seriesIdentifier = seriesItr->c_str();

            std::cout << "Now reading series: " << std::endl;
            std::cout << seriesIdentifier << std::endl;

            /* Read the files */
            typedef std::vector< std::string > FileNamesContainer;
            FileNamesContainer file_names;
            file_names = nameGenerator->GetFileNames (seriesIdentifier);

            /* Get the first filename */
            std::string first_fn = *(file_names.begin());

            /* Test against restrictions */
#if GDCM_MAJOR_VERSION < 2
            if (!test_dicom_ok (first_fn)) {
                seriesItr++;
                continue;
            }
#endif

#if defined (commentout)
            /* Print out the file names */
            FileNamesContainer::const_iterator fn_it = file_names.begin();
            printf ("File names are:\n");
            while (fn_it != file_names.end()) {
                printf ("  %s\n", fn_it->c_str());
                fn_it ++;
            }
#endif

            rdr->SetFileNames (file_names);
            try {
                rdr->Update();
                dicom_load_succeeded = true;
            } catch (itk::ExceptionObject &ex) {
                /* do nothing */
                logfile_printf ("Failed to load: %s\n", ex.GetDescription());
            }
            seriesItr++;
        }
        if (!dicom_load_succeeded) {
            print_and_exit ("Error, unable to load dicom series.\n");
        }
    } catch (itk::ExceptionObject &ex) {
        std::cout << ex << std::endl;
        print_and_exit ("Error loading dicom series.\n");
    }
}

CharImageType::Pointer
load_dicom_char (const char *dicom_dir)
{
    DicomCharReaderType::Pointer fixed_input_rdr
        = DicomCharReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

UCharImageType::Pointer
load_dicom_uchar (const char *dicom_dir)
{
    DicomUCharReaderType::Pointer fixed_input_rdr
        = DicomUCharReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

ShortImageType::Pointer
load_dicom_short (const char *dicom_dir)
{
    DicomShortReaderType::Pointer fixed_input_rdr
        = DicomShortReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

UShortImageType::Pointer
load_dicom_ushort (const char *dicom_dir)
{
    DicomUShortReaderType::Pointer fixed_input_rdr
        = DicomUShortReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

Int32ImageType::Pointer
load_dicom_int32 (const char *dicom_dir)
{
    DicomInt32ReaderType::Pointer fixed_input_rdr
        = DicomInt32ReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

UInt32ImageType::Pointer
load_dicom_uint32 (const char *dicom_dir)
{
    DicomUInt32ReaderType::Pointer fixed_input_rdr
        = DicomUInt32ReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

FloatImageType::Pointer
load_dicom_float (const char *dicom_dir)
{
    DicomFloatReaderType::Pointer fixed_input_rdr
        = DicomFloatReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

DoubleImageType::Pointer
load_dicom_double (const char *dicom_dir)
{
    DicomDoubleReaderType::Pointer fixed_input_rdr
        = DicomDoubleReaderType::New();
    load_dicom_dir_rdr (fixed_input_rdr, dicom_dir);
    fixed_input_rdr->Update();
    return fixed_input_rdr->GetOutput();
}

/* Explicit instantiations */
template void load_dicom_dir_rdr(DicomShortReaderType::Pointer rdr, const char *dicom_dir);
template void load_dicom_dir_rdr(DicomUShortReaderType::Pointer rdr, const char *dicom_dir);
template void load_dicom_dir_rdr(DicomFloatReaderType::Pointer rdr, const char *dicom_dir);
