#pragma once

//v20130830 : his header buffer, itk compatible

class QPixmap;
class QLabel;
class QPainter;

//class QImage;

#define DEFAULT_WINLEVEL_MID 10000
#define DEFAULT_WINLEVEL_WIDTH 20000

#define DEFAULT_ELEKTA_HIS_HEADER_SIZE 100

#include "itkImage.h"
#include <QImage>
#include <vector>
#include <QVector>
#include <QRgb>
#include <QColor>

#include "yk_config.h"

struct BADPIXELMAP{
	int BadPixX;
	int BadPixY;
	int ReplPixX;
	int ReplPixY;
};

class CContourROI
{
public: 
    CContourROI(void){ ; }
    ~CContourROI(void){ _vDataPt.clear(); }

    bool _bDrawROI;
    QString _strNameROI;
    QRgb _rgb; //QRgb(255,0,0)
    int _thick;
    std::vector<QPoint> _vDataPt;
};

enum enProfileDirection{
	DIRECTION_HOR = 0,
	DIRECTION_VER,	
};

enum enSplitOption{
    PRI_LEFT_TOP = 0, //Primary Left Top
    PRI_RIGHT_TOP, //Primary Left Top
    PRI_LEFT,	
    PRI_RIGHT,	
    PRI_TOP,	
    PRI_BOTTOM,	
};


typedef itk::Image<unsigned short, 2> UnsignedShortImageType;
typedef itk::Image<float, 2> FloatImageType2D;

using namespace std;

class YK16GrayImage
{	
public:
	YK16GrayImage(void);
	YK16GrayImage(int width, int height);
	~YK16GrayImage(void);

	int m_iWidth;
	int m_iHeight;
	//added: 20140206
	double m_fSpacingX; //[mm/px]
	double m_fSpacingY;

        double m_fOriginX;
        double m_fOriginY;

        float m_fIntensityMag;//intensity magnification factor default = 1.0;
        float m_fIntensityOffset;//intensity Offset factor default = 0.0;
        float m_fNormValue;

        void SetIntensityModification(float intensityMag, float intensityOffset){ m_fIntensityMag = intensityMag; m_fIntensityOffset = intensityOffset; }
        float GetOriginalIntensityVal(unsigned short usPixVal); //regarding m_fIntensityMag and m_fIntensityOffset
        unsigned short GetWrappingIntensityVal(float fPixVal);

	unsigned short* m_pData; // 0 - 65535

	QPixmap* m_pPixmap; //Actually, no need!
	QImage m_QImage;
	//QPainter* m_pPainter;

	bool LoadRawImage(const char *filePath, int width, int height);
	bool LoadRawImage(const char *filePath, int width, int height, int headerOffset);
	bool LoadRawImage(const char *filePath, int width, int height, int headerOffset, bool bInvert);
	bool CopyFromBuffer(unsigned short* pImageBuf, int width, int height);
	bool CloneImage(YK16GrayImage& other);

	bool CreateImage(int width, int height, unsigned short usVal);

	bool FillPixMap(int winMid, int winWidth);
	bool FillPixMapMinMax(int winMin, int winMax); //0-65535 Сп window level
        
        bool FillPixMapDose(float normval);
        bool FillPixMapDose(); //m_fNormValue
        void SetNormValueOriginal(float normval);

        QColor GetColorFromDosePerc(float percVal);
        bool FillPixMapGamma();
        QColor GetColorFromGamma(float gammaVal);        

	bool FillPixMapDual(int winMid1, int winMid2,int winWidth1, int winWidth2);
	bool FillPixMapMinMaxDual(int winMin1, int winMin2, int winMax1, int winMax2); //0-65535 Сп window level

	bool SaveDataAsRaw (const char *filePath);
	//bool DrawToLabel(QLabel* lbDisplay);

	bool IsEmpty();
	bool ReleaseBuffer();

	//bool CalcImageInfo (double& meanVal, double& STDV, double& minVal, double& maxVal);
	bool CalcImageInfo ();
	double CalcAveragePixelDiff(YK16GrayImage& other);

	bool DoPixelReplacement(std::vector<BADPIXELMAP>& vPixelMapping); //based on pixel mapping information, some bad pixels will be replaced with median pixel value near by

	static void CopyYKImage2ItkImage(YK16GrayImage* pYKImage, UnsignedShortImageType::Pointer& spTarImage);
	static void CopyItkImage2YKImage(UnsignedShortImageType::Pointer& spSrcImage, YK16GrayImage* pYKImage);

	QString m_strFilePath;

	double m_fPixelMean;
	double m_fPixelSD;
	double m_fPixelMin;
	double m_fPixelMax;

	static void Swap(YK16GrayImage* pImgA, YK16GrayImage* pImgB);

	QRect m_rtROI;
	bool setROI(int left, int top, int right, int bottom); //if there is error, go to default: entire image
	bool CalcImageInfo_ROI();
	double m_fPixelMean_ROI;
	double m_fPixelSD_ROI;
	double m_fPixelMin_ROI;
	double m_fPixelMax_ROI;
	bool m_bDrawROI;

	void DrawROIOn(bool bROI_Draw); //only rectangle


	//Elekta CBCT recon
	char* m_pElektaHisHeader;
	void CopyHisHeader(const char *hisFilePath);
	//bool SaveDataAsHis (const char *filePath);
	bool SaveDataAsHis( const char *filePath, bool bInverse );
	bool m_bShowInvert;

	void MultiplyConstant(double multiplyFactor);

	void SetSpacing(double spacingX, double spacingY)
	{
		m_fSpacingX = spacingX;
		m_fSpacingY = spacingY;
	};
        void SetOrigin(double originX, double originY)
        {
            m_fOriginX = originX;
            m_fOriginY = originY;
        };

	QPoint m_ptProfileProbe; //Mouse Clicked Position --> Data
	bool m_bDrawProfileX;
	bool m_bDrawProfileY;

	QPoint m_ptFOVCenter; // data pos
	int m_iFOVRadius;//data pos (pixel)
	bool m_bDrawFOVCircle;

	int m_iTableTopPos;//data pos
	bool m_bDrawTableLine;

	QPoint m_ptCrosshair; //data position
	bool m_bDrawCrosshair;

	////ZOOM and PAN function. Using these information below, prepare the m_QImage for displaying 
	//in qlabel in FillPixMap function
	int m_iOffsetX; //for Pan function.. this is data based offset
	int m_iOffsetY;
	void SetOffset(int offsetX, int offsetY){m_iOffsetX = offsetX; m_iOffsetY = offsetY;}
	double m_fZoom;
	void SetZoom(double fZoom);
	unsigned short GetPixelData(int x, int y);
        unsigned short GetCrosshairPixelData();//Get pixel data of crosshair
        float GetCrosshairOriginalData();//Get pixel data of crosshair
        float GetCrosshairPercData();//Get pixel data of crosshair

	//SPLIT VIEW
	QPoint m_ptSplitCenter; //Fixed image with Moving image. center is based on dataPt.//Fixed Image: Left Top + Right Bottom, Moving: Right Top + Left Bottom        
	int m_enSplitOption;
	//This cetner is moved while Left Dragging //All split and crosshair are data point based!
	void SetSplitOption(enSplitOption option) {m_enSplitOption = option;}
	void SetSplitCenter(QPoint& ptSplitCenter);	//From mouse event, data point	
	//void SetSplitCenter(int centerX, int centerY) {m_ptSplitCenter.setX(centerX); m_ptSplitCenter.setY(centerY);}//From mouse event, data point
	bool ConstituteFromTwo(YK16GrayImage& YKImg1,YK16GrayImage& YKImg2); //YKImg1 and two should be in exactly same dimension and spacing
	bool isPtInFirstImage(int dataX, int dataY);

	void SetProfileProbePos(int dataX, int dataY);                
        void SetCrosshairPosPhys(float physX, float physY, enPLANE plane);

	unsigned short GetProfileProbePixelVal();	
	void GetProfileData(int dataX, int dataY, QVector<double>& vTarget, enProfileDirection direction); 
	void GetProfileData(QVector<double>& vTarget, enProfileDirection direction);
       

	void EditImage_Flip();
	void EditImage_Mirror();

	void MedianFilter(int iMedianSizeX, int iMedianSizeY);

	double m_fResampleFactor;//if it is not the 1.0, the data is already resampled.	

	UnsignedShortImageType::Pointer CloneItkImage();
	void ResampleImage(double fResampleFactor);

	void UpdateFromItkImage(UnsignedShortImageType::Pointer& spRefItkImg);
	void UpdateFromItkImageFloat(FloatImageType2D::Pointer& spRefItkImg);
        void UpdateFromItkImageFloat(FloatImageType2D::Pointer& spRefItkImg, float fIntenistyMag, float fIntensityOffset, bool bYFlip= false); 
        void UpdateToItkImageFloat(FloatImageType2D::Pointer& spRefItkImg); //to be implemented

	void InvertImage();

	QString m_strTimeStamp; //HHMMSSFFF (9digits)
	int m_iProcessingElapsed; //processing time in ms

	//will be added later
	/*void EditImage_CW90();
	void EditImage_CCW90();
	void EditImage_Rotation(double angle);*/

	std::vector<QPoint> m_vMarker;//position in data dimension (not either physical nor display)
	void AddMarkerPos(int dataX, int dataY);
	void ClearMarkerPos();
	bool m_bDrawMarkers;
	int m_iNumOfDispMarker; //for display      e.g.)  0,1,2,3,4
	int m_iNumOfTrackMarker; //for tracking   GetMeanPos  e.g.)  0,1,2 only
	QPoint GetMeanPos(); //m_iNumOfTrackMarker based

	//Reference marker group
	std::vector<QPoint> m_vMarkerRef;//position in data dimension (not either physical nor display)
	std::vector<bool> m_bvRefOutOfRange;//leave a tag when calculated 2D ref position is out of image
	void AddMarkerPosRef(int dataX, int dataY);
	void ClearMarkerPosRef();
	bool m_bDrawMarkersRef;	
	QPoint GetMeanPosRef(); //m_iNumOfDispMarker based
	bool existRefMarkers();        

	//should be implemented later
	double m_track_priorErr;
	double m_track_motionErr;
	double m_track_CC_penalty;	

	//Below are the flexmap related stuffs
	float m_fMVGantryAngle;
	float m_fPanelOffsetX; //mm
	float m_fPanelOffsetY;//mm
	bool m_bKVOn;
	bool m_bMVOn;
	int m_iXVI_ElapseMS;

	void UpdateTrackingData(YK16GrayImage* pYKProcessedImage);
	bool m_bDraw8Bit; //if some processing is done by 8bit

        bool m_bDrawOverlayText;
        QString m_strOverlayText;
	
        //2D points in data map. only outer contour points are included here after trimming out.
        //std::vector<QPoint> m_vContourROI; //later it will be array or linked list to display mutliple ROIs

        //std::vector<std::vector<QPoint>*> m_vvContourROI;
        //std::vector<bool> m_vbDrawContourROI;
        //bool m_bDrawContours;

        std::vector<CContourROI*> m_vvContourROI;
        void AddContourROI(QString& strROIName, bool bDisplay, QRgb color, int thickness, std::vector<QPoint>& vContourPts);
        void ClearContourROI();// delete all the data of contourROI
        bool SetDisplayStatus(QString& strROIName, bool bDisplay);
        //void GetColorSingleROI()


        vector<VEC3D> m_vColorTable;
        //vector<VEC3D> m_vColorTableGammaLow;
        //vector<VEC3D> m_vColorTableGammaHigh;

        void SetColorTable(vector<VEC3D>& vInputColorTable);

        
};