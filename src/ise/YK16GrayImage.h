#pragma once

class QPixmap;
class QLabel;
class QPainter;

#define DEFAULT_WINLEVEL_MID 10000
#define DEFAULT_WINLEVEL_WIDTH 20000

//typedef itk::Image<unsigned short, 2> UnsignedShortImageType;

//#include "itkImage.h"
#include <QImage>
#include <vector>
#include "acquire_4030e_define.h"

using namespace std;

class YK16GrayImage
{	
public:
	YK16GrayImage(void);
	YK16GrayImage(int width, int height);
	~YK16GrayImage(void);

	int m_iWidth;
	int m_iHeight;

	unsigned short* m_pData; // 0 - 65535

	QPixmap* m_pPixmap;
	QImage m_QImage;
	//QPainter* m_pPainter;

	bool LoadRawImage(const char *filePath, int width, int height);
	bool CopyFromBuffer(unsigned short* pImageBuf, int width, int height);

	bool CreateImage(int width, int height, unsigned short usVal);

	bool FillPixMap(int winMid, int winWidth);
	bool FillPixMapMinMax(int winMin, int winMax); //0-65535 Сп window level

	bool SaveDataAsRaw (const char *filePath);
	bool DrawToLabel(QLabel* lbDisplay);

	bool IsEmpty();
	bool ReleaseBuffer();

	bool CalcImageInfo (double& meanVal, double& STDV, double& minVal, double& maxVal);
	double CalcAveragePixelDiff(YK16GrayImage& other);

	bool DoPixelReplacement(vector<BADPIXELMAP>& vPixelMapping); //based on pixel mapping information, some bad pixels will be replaced with median pixel value near by

	//static void CopyYKImage2ItkImage(YK16GrayImage* pYKImage, UnsignedShortImageType::Pointer& spTarImage);
	//static void CopyItkImage2YKImage(UnsignedShortImageType::Pointer& spSrcImage, YK16GrayImage* pYKImage);
};
