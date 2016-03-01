/* GCS - Adapted from exchkeys.h
    http://itc.wustl.edu/Exchcode.html
    http://itc.wustl.edu/exchange_files/tapeexch400.htm
*/
/***********************************************************************
 * $Id: exchkeys.h,v 1.2 2000/01/18 20:07:09 jwm Exp $
 * Description: include file for RTOG submission
 *
 ***********************************************************************
 * $Log: exchkeys.h,v $
 * Revision 1.2  2000/01/18  20:07:09  jwm
 * Correct comment
 *
 * Revision 1.1  2000/01/14  22:49:16  jwm
 * Remove comments originating in other development trees
 *
 * Revision 1.0  2000/01/14  22:42:38  jwm
 * Initial revision
 **********************************************************************/
/***********************************************************************
  Description: 

  This file contains the definitions of the keyword and keyvalues
  required to support patient data exchange as specified in:

  		Specifications for Tape Format for Exchange
		    of Treatment Planning Information
		       based on AAPM Report #10
                       as used and modified by

	       The NCI Particle Intercomparison Contract

	       The NCI High Energy Photon External Beam
       	               Treatment Planning Contract

		     The NCI Electron External Beam
		      Treatment Planning Contract

				 and

                        The RTOG 3D QA Center

 		             Version 4.00
			     

  Wm. B. Harms, Sr.
  RTOG 3D QA Center
  Radiation Oncology Center
  Mallinckrodt Institute of Radiology
  Washington University School of Medicine
  510 South Kingshighway Blvd.
  St. Louis, MO 63110

  *********************************************************************/


/* NUMBER OF CURRENTLY SUPPORTED (AND DEFINED) KEYWORDS */
#define RTOG_NUM_KEYS 119

/* NUMBER OF CURRENTLY SUPPORTED (AND DEFINED) KEY VALUES */
#define RTOG_NUM_KEY_VALS 70

/* GCS: Deleted the rather useless "key_list" and "key_value" lists */
/***********************************************************************
  
   key_list, key_list_words AND key_type MUST MAINTAIN A 1-1 CORRESPONDENCE.
   WHEN YOU ADD SOMETHING TO key_list, YOU MUST ADD A CORRESPONDING ENUM
   VALUE IN key_type AT THE SAME LOCATION IN THE LIST

   SIMILARLY, key_value, key_value_words AND key_value_type MUST MAINTAIN
   A 1-1 CORRESPONDENCE.

   WHILE IT IS NOT GOOD PROGRAMMING STYLE TO ALLOCATE MEMORY IN HEADER FILES,
   IT WAS THE SIMPLEST SOLUTION TO THE LARGE AMOUNT OF DEFINITIONS REQUIRED
   FOR THE TAPE EXCHANGE (KEYWORDS AND VALUES).  THEREFORE, A VALUE IS DEFINED
   CALLED "NOT_MAIN" WHICH MUST BE DEFINED IN ALL SOURCE CODE FILES WHICH
   DO NOT CONTAIN THE main() ROUTINE.  THIS SHOULD NOT BE DEFINED FOR THE
   main() MODULE(S)

***********************************************************************/
/* GCS Simplify the above, so that you don't define anything unless you want 
   to define the structures */ 
#if defined (DEFINE_RTOG_STRINGS)
char *key_list_words[RTOG_NUM_KEYS] =
  {
  
  /* THIS ARRAY OF STRINGS MUST MAINTAIN A 1-1 CORRESPONDENCE WITH THE key_type
     ENUM AS THEY ARE REFERENCED USING THIS ENUM.  IT CONTAINS THE EXACT
     STRING (ENGLISH) REPRESENTATIONS USED FOR OUTPUT OF THE DIRECTORY FILE. */

  /********* 0 through 9 ***********************/
  "TAPE STANDARD #",
  "INTERCOMPARISON STANDARD #",
  "INSTITUTION",
  "DATE CREATED",
  "WRITER",
  "IMAGE #",
  "IMAGE TYPE",
  "CASE #",
  "PATIENT NAME",
  "DATE WRITTEN",

  /********* 10 through 19 ***********************/

  "UNIT #",
  "FILE OF ORIGIN",
  "COMMENT DESCRIPTION",
  "SCAN TYPE",
  "CT OFFSET",
  "GRID 1 UNITS",
  "GRID 2 UNITS",
  "NUMBER REPRESENTATION",
  "BYTES PER PIXEL",
  "NUMBER OF DIMENSIONS",

  /********* 20 through 29 ***********************/

  "SIZE OF DIMENSION 1",
  "SIZE OF DIMENSION 2",
  "Z VALUE",
  "X OFFSET",
  "Y OFFSET",
  "CT-AIR",
  "CT-WATER",
  "SITE OF INTEREST",
  "SCAN DESCRIPTION",
  "SCANNER TYPE",

  /********* 30 through 39 ***********************/

  "HEAD IN/OUT",
  "POSITION IN SCAN",
  "PATIENT ATTITUDE",
  "TAPE OF ORIGIN",
  "SCAN ID",
  "SCAN#",
  "SCAN DATE",
  "SCAN FILE NAME",
  "SLICE THICKNESS",
  "CT SCALE",

  /********* 40 through 49 ***********************/

  "DISTRUST ABOVE",
  "STRUCTURE NAME",
  "STRUCTURE FORMAT",
  "NUMBER OF SCANS",
  "MAXIMUM # SCANS",
  "MAXIMUM POINTS PER SEGMENT",
  "MAXIMUM SEGMENTS PER SCAN",
  "STRUCTURE EDITION",
  "STRUCTURE COLOR",
  "STRUCTURE DESCRIPTION",

  /********* 50 through 59 ***********************/

  "STUDY NUMBER OF ORIGIN",     /* Fixed, GCS */
  "ORIENTATION OF STRUCTURE",
  "DOSE #",
  "DOSE TYPE",
  "DOSE UNITS",
  "ORIENTATION OF DOSE",
  "SIZE OF DIMENSION 3",
  "COORD 1 OF FIRST POINT",
  "COORD 2 OF FIRST POINT",
  "HORIZONTAL GRID INTERVAL",

  /********* 60 through 69 ***********************/

  "VERTICAL GRID INTERVAL",
  "DOSE DESCRIPTION",
  "DOSE EDITION",
  "PLAN # OF ORIGIN",
  "PLAN EDITION OF ORIGIN",
  "VERSION # OF PROGRAM",
  "XCOORD OF NORMALIZN POINT",
  "YCOORD OF NORMALIZN POINT",
  "ZCOORD OF NORMALIZN POINT",
  "DOSE AT NORMALIZN POINT",

  /********* 70 through 79 ***********************/

  "DOSE ERROR",
  "BEAM #",
  "BEAM MODALITY",
  "BEAM ENERGY(MEV)",
  "BEAM DESCRIPTION",
  "RX DOSE PER TX (GY)",
  "NUMBER OF TX",
  "FRACTION GROUP ID",
  "BEAM TYPE",
  "PLAN ID OF ORIGIN",

  /********* 80 through 89 ***********************/

  "COLLIMATOR TYPE",
  "APERTURE TYPE",
  "APERTURE DESCRIPTION",
  "COLLIMATOR ANGLE",
  "GANTRY ANGLE",
  "COUCH ANGLE",
  "NOMINAL ISOCENTER DIST",
  "APERTURE ID",
  "WEDGE ANGLE",
  "WEDGE ROTATION ANGLE",

  /********* 90 through 99 ***********************/

  "ARC ANGLE",
  "COMPENSATOR",
  "VOLUME TYPE",
  "NUMBER OF PAIRS",
  "MAXIMUM # PAIRS",
  "DATE OF DVH",
  "DOSE SCALE",
  "VOLUME SCALE",
  "NUMBER OF FRACTIONS",
  "FILM NUMBER",

  /********* 100 through 109 ***********************/

  "FILM DATE",
  "FILM TYPE",
  "SOURCE IMAGE DISTANCE",
  "FILM DESCRIPTION",
  "FILM SOURCE",
  "OD SCALE",
  "BITS PER PIXEL",
  "DOSE VOLUME HISTOGRAM",
  "SEED MODEL",
  "ISOTOPE",

  /********* 110 through 116 ***********************/

  "SEED STRENGTH",
  "STRENGTH UNITS",
  "DATE OF IMPLANT",
  "NUMBER OF SEEDS",
  "IMAGE SOURCE",
  "PIXEL OFFSET",
  "UNDEFINEDKEY",
  "COORD 3 OF FIRST POINT",	/* GCS */
  "DEPTH GRID INTERVAL"		/* GCS */

  };

char *key_value_words[RTOG_NUM_KEY_VALS] =
  {

  /* THIS ARRAY OF STRINGS MUST MAINTAIN A 1-1 CORRESPONDENCE WITH THE
     key_value_type ENUM AS THEY ARE REFERENCED USING THIS ENUM.  IT CONTAINS
     THE EXACT STRING REPRESENTATIONS USED FOR OUTPUT OF THE DIRECTORY FILE. */

  /***********  0 through 9 *************************/
  "COMMENT",
  "CT SCAN",
  "MRI",
  "ULTRASOUND",
  "STRUCTURE",
  "BEAM GEOMETRY",
  "DIGITAL FILM",
  "DOSE",
  "DOSE VOLUME HISTOGRAM",
  "SEED GEOMETRY",

  /***********  10 through 19 *************************/
  
  "TRANSVERSE",
  "SAGITTAL",
  "CORONAL",
  "TWO'S COMPLEMENT INTEGER",
  "CHARACTER",
  "IN",
  "OUT",
  "NOSE UP",
  "NOSE DOWN",
  "LEFT SIDE DOWN",

  /***********  20 through 39 *************************/
  
  "RIGHT SIDE DOWN",
  "SCAN-BASED",
  "NOMINAL",
  "MINIMUM",
  "MAXIMUM",
  "PHYSICAL",
  "EFFECTIVE",
  "LET",
  "OER",
  "ERROR",

  /***********  30 through 39 *************************/
  
  "GRAYS",
  "RADS",
  "CGYS",
  "PERCENT",
  "RELATIVE",
  "ABSOLUTE",
  "CGE",
  "SIMULATOR",
  "DRR",
  "PORT",

  /***********  40 through 49 *************************/
  
  "UNSIGNED BYTE",
  "X-RAY",
  "ELECTRON",
  "NEUTRON",
  "PROTON",
  "OTHER",
  "COLLIMATOR",
  "BLOCK",
  "MLC_X",
  "MLC_Y",

  /***********  50 through 59 *************************/
  
  "MLC_XY",
  "STATIC",
  "ARC",
  "SYMMETRIC",
  "ASYMMETRIC",
  "ASYMMETRIC_X",
  "ASYMMETRIC_Y",
  "NONE",
  "1D-X",
  "1D-Y",

  /***********  60 through 69 *************************/
  
  "2D",
  "3D",
  "FILM",
  "ONLINE",
  "COMPUTED",
  "I125",
  "PD103",
  "MCI",
  "CGYCM2PERHR",
  "SECONDARY CAPTURE"
  };
#endif

/* SINCE THIS IS NOT BEING INCLUDED IN main(), DEFINE THE EXTERNAL
   REFERENCE. */

extern char *key_value[];
extern char *key_list[];
extern char *key_value_words[];
extern char *key_list_words[];

/* THE ENUM'S ARE NEEDED EVERYWHERE */

typedef enum
  {
  
  /* THIS ENUM MUST MAINTAIN A 1-1 CORESPONDENCE WITH THE key_value AND
     key_list STRING ARRAYS AS THE STRINGS ARE GENERALLY REFERENCED VIA THE
     VALUE OF THE ENUM */
  
  /********* 0 through 9 ***********************/
  ekTAPESTANDARDNUMBER,
  ekINTERCOMPARISONSTANDARDNUMBER,
  ekINSTITUTION,
  ekDATECREATED,
  ekWRITER,
  ekIMAGENUMBER,
  ekIMAGETYPE,
  ekCASENUMBER,
  ekPATIENTNAME,
  ekDATEWRITTEN,

  /********* 10 through 19 ***********************/

  ekUNITNUMBER,
  ekFILEOFORIGIN,
  ekCOMMENTDESCRIPTION,
  ekSCANTYPE,
  ekCTOFFSET,
  ekGRID1UNITS,
  ekGRID2UNITS,
  ekNUMBERREPRESENTATION,
  ekBYTESPERPIXEL,
  ekNUMBEROFDIMENSIONS,

  /********* 20 through 29 ***********************/

  ekSIZEOFDIMENSION1,
  ekSIZEOFDIMENSION2,
  ekZVALUE,
  ekXOFFSET,
  ekYOFFSET,
  ekCTAIR,
  ekCTWATER,
  ekSITEOFINTEREST,
  ekSCANDESCRIPTION,
  ekSCANNERTYPE,

  /********* 30 through 39 ***********************/

  ekHEADIN,
  ekPOSITIONINSCAN,
  ekPATIENTATTITUDE,
  ekTAPEOFORIGIN,
  ekSCANID,
  ekSCANNUMBER,
  ekSCANDATE,
  ekSCANFILENAME,
  ekSLICETHICKNESS,
  ekCTSCALE,

  /********* 40 through 49 ***********************/

  ekDISTRUSTABOVE,
  ekSTRUCTURENAME,
  ekSTRUCTUREFORMAT,
  ekNUMBEROFSCANS,
  ekMAXIMUMNUMBERSCANS,
  ekMAXIMUMPOINTSPERSEGMENT,
  ekMAXIMUMSEGMENTSPERSCAN,
  ekSTRUCTUREEDITION,
  ekSTRUCTURECOLOR,
  ekSTRUCTUREDESCRIPTION,

  /********* 50 through 59 ***********************/

  ekSTUDYNUMBEROFORIGIN,
  ekORIENTATIONOFSTRUCTURE,
  ekDOSENUMBER,
  ekDOSETYPE,
  ekDOSEUNITS,
  ekORIENTATIONOFDOSE,
  ekSIZEOFDIMENSION3,
  ekCOORD1OFFIRSTPOINT,
  ekCOORD2OFFIRSTPOINT,
  ekHORIZONTALGRIDINTERVAL,

  /********* 60 through 69 ***********************/

  ekVERTICALGRIDINTERVAL,
  ekDOSEDESCRIPTION,
  ekDOSEEDITION,
  ekPLANNUMBEROFORIGIN,
  ekPLANEDITIONOFORIGIN,
  ekVERSIONNUMBEROFPROGRAM,
  ekXCOORDOFNORMALIZNPOINT,
  ekYCOORDOFNORMALIZNPOINT,
  ekZCOORDOFNORMALIZNPOINT,
  ekDOSEATNORMALIZNPOINT,

  /********* 70 through 79 ***********************/

  ekDOSEERROR,
  ekBEAMNUMBER,
  ekBEAMMODALITY,
  ekBEAMENERGYMEV,
  ekBEAMDESCRIPTION,
  ekRXDOSEPERTXGY,
  ekNUMBEROFTX,
  ekFRACTIONGROUPID,
  ekBEAMTYPE,
  ekPLANIDOFORIGIN,

  /********* 80 through 89 ***********************/

  ekCOLLIMATORTYPE,
  ekAPERTURETYPE,
  ekAPERTUREDESCRIPTION,
  ekCOLLIMATORANGLE,
  ekGANTRYANGLE,
  ekCOUCHANGLE,
  ekNOMINALISOCENTERDIST,
  ekAPERTUREID,
  ekWEDGEANGLE,
  ekWEDGEROTATIONANGLE,

  /********* 90 through 99 ***********************/

  ekARCANGLE,
  ekCOMPENSATOR,
  ekVOLUMETYPE,
  ekNUMBEROFPAIRS,
  ekMAXIMUMNUMBERPAIRS,
  ekDATEOFDVH,
  ekDOSESCALE,
  ekVOLUMESCALE,
  ekNUMBEROFFRACTIONS,
  ekFILMNUMBER,

  /********* 100 through 109 ***********************/

  ekFILMDATE,
  ekFILMTYPE,
  ekSOURCEIMAGEDISTANCE,
  ekFILMDESCRIPTION,
  ekFILMSOURCE,
  ekODSCALE,
  ekBITSPERPIXEL,
  ekDOSEVOLUMEHISTOGRAM,
  ekSEEDMODEL,
  ekSEEDISOTOPE,

  /********* 110 through 116 ***********************/

  ekSEEDSTRENGTH,
  ekSEEDSTRENGTHUNITS,
  ekDATEOFIMPLANT,
  ekNUMBEROFSEEDS,
  ekIMAGESOURCE,
  ekPIXELOFFSET,
  ekUNDEFINEDKEY,
  ekCOORD3OFFIRSTPOINT,
  ekDEPTHGRIDINTERVAL

  } key_type;

typedef enum
  {

  /* THIS ENUM MUST MAINTAIN A 1-1 CORRESPONDENCE WITH THE key_value AND
     key_value_words STRING ARRAYS AS THE STRINGS ARE GENERALLY REFERENCED
     VIA THE VALUE OF THE ENUM */

  /***********  0 through 9 *************************/
  evCOMMENT,
  evCTSCAN,
  evMRSCAN,
  evULTRASOUND,
  evSTRUCTURE,
  evBEAMGEOMETRY,
  evDIGITALFILM,
  evDOSE,
  evDOSEVOLUMEHISTOGRAM,
  evSEEDGEOMETRY,

  /***********  10 through 19 *************************/
  
  evTRANSVERSE,
  evSAGITTAL,
  evCORONAL,
  evTWOSCOMP,
  evCHARACTER,
  evIN,
  evOUT,
  evNOSEUP,
  evNOSEDOWN,
  evLEFTSIDEDOWN,

  /***********  20 through 39 *************************/
  
  evRIGHTSIDEDOWN,
  evSCANBASED,
  evNOMINAL,
  evMINIMUM,
  evMAXIMUM,
  evPHYSICAL,
  evEFFECTIVE,
  evLET,
  evOER,
  evERROR,

  /***********  30 through 39 *************************/
  
  evGRAYS,
  evRADS,
  evCGYS,
  evPERCENT,
  evRELATIVE,
  evABSOLUTE,
  evCGE,
  evSIMULATOR,
  evDRR,
  evPORT,

  /***********  40 through 49 *************************/
  
  evUNSIGNEDBYTE,
  evXRAY,
  evELECTRON,
  evNEUTRON,
  evPROTON,
  evOTHER,
  evCOLLIMATOR,
  evBLOCK,
  evMLC_X,
  evMLC_Y,

  /***********  50 through 59 *************************/
  
  evMLC_XY,
  evSTATIC,
  evARC,
  evSYMMETRIC,
  evASYMMETRIC,
  evASYMMETRIC_X,
  evASYMMETRIC_Y,
  evNONE,
  ev1D_X,
  ev1D_Y,

  /***********  60 through 69 *************************/
  
  ev2D,
  ev3D,
  evFILM,
  evONLINE,
  evCOMPUTED,
  evI125,
  evPD103,
  evMCI,
  evCGYCM2PERHR,
  evSECONDARYCAPTURE
  } key_value_type;

typedef enum
  {

  /* THESE ARE THE DATA TYPES SUPPORTED BY THE aapm_entry ROUTINE FOR
     FORMATTING AND RECORDING A KEYWORD/KEYVALUE COMBINATION IN A BUFFERED
     AAPM FILE DATA BLOCK */
  
  aSTRING,
  aINT,
  aFLOAT
  } key_data_type;


/******************************** End of exchkeys.h *************************/
