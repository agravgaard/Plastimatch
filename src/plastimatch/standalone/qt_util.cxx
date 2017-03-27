#include "qt_util.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkImageDuplicator.h"
#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkImageLinearConstIteratorWithIndex.h"

#include <QtGlobal> //for qRound
#include <QString> //for qRound
#include <QtGui/QMainWindow>
#include <QStandardItemModel>
#include "gamma_gui.h"
#include <QDir>
#include <QMessageBox>
#include "itk_resample.h" //plm

bool QUTIL::QPointF_Compare(QPointF& ptData1, QPointF& ptData2)
{
    //return (ptData1.x() > ptData2.x()); //ASCENDING
    return (ptData1.x() < ptData2.x()); //DESCENDING
}


void QUTIL::Set2DTo3D(FloatImage2DType::Pointer& spSrcImg2D, UShortImageType::Pointer& spTargetImg3D, int idx, enPLANE iDirection)
{
    if (!spSrcImg2D || !spTargetImg3D) //Target image should be also ready.
        return;

    int idxHor = 0;
    int idxVer = 0;
    int idxZ = 0;

    switch (iDirection)
    {
    case PLANE_AXIAL:
        idxHor = 0;
        idxVer = 1;
        idxZ = 2;
        break;
    case PLANE_FRONTAL:
        idxHor = 0;
        idxVer = 2;
        idxZ = 1;
        break;
    case PLANE_SAGITTAL:
        idxHor = 1;
        idxVer = 2;
        idxZ = 0;
        break;
    }

    FloatImage2DType::SizeType imgDim2D = spSrcImg2D->GetBufferedRegion().GetSize();
    FloatImage2DType::SpacingType spacing2D = spSrcImg2D->GetSpacing();
    FloatImage2DType::PointType origin2D = spSrcImg2D->GetOrigin();

    UShortImageType::SizeType imgDim3D = spTargetImg3D->GetBufferedRegion().GetSize();
    UShortImageType::SpacingType spacing3D = spTargetImg3D->GetSpacing();
    UShortImageType::PointType origin3D = spTargetImg3D->GetOrigin();

    //Filtering
    if (imgDim2D[0] != imgDim3D[idxHor] ||
        imgDim2D[1] != imgDim3D[idxVer] || idx < 0 || idx >= imgDim3D[idxZ])
    {
        cout << "Error: image dimensions is not matching" << endl;
        cout << "2D= " << imgDim2D << endl;
        cout << "3D= " << imgDim3D << endl;
        return;
    }
    /*int width = imgDim[idxHor];
    int height  = imgDim[idxVer];*/



    //itk::ImageRegionConstIteratorWithIndex<FloatImage2DType> it_2D (spSrcImg2D, spSrcImg2D->GetRequestedRegion());
    itk::ImageRegionConstIterator<FloatImage2DType> it_2D(spSrcImg2D, spSrcImg2D->GetRequestedRegion());
    itk::ImageSliceIteratorWithIndex<UShortImageType> it_3D(spTargetImg3D, spTargetImg3D->GetRequestedRegion());

    it_3D.SetFirstDirection(idxHor);
    it_3D.SetSecondDirection(idxVer);
    it_3D.GoToBegin();

    int zSize = imgDim3D[idxZ];

    it_2D.GoToBegin();

    float fVal2D = 0.0;
    unsigned short outputVal = 0;

    for (int i = 0; i< zSize && !it_3D.IsAtEnd(); i++)
    {
        /*QFileInfo crntFileInfo(arrYKImage[i].m_strFilePath);
        QString crntFileName = crntFileInfo.fileName();
        QString crntPath = strSavingFolder + "/" + crntFileName;*/
        //Search matching slice using slice iterator for m_spProjCTImg  
        if (i == idx)
        {
            while (!it_3D.IsAtEndOfSlice())
            {
                while (!it_3D.IsAtEndOfLine())
                {
                    fVal2D = it_2D.Get();

                    if (fVal2D < 0.0)
                        outputVal = 0;
                    else if (fVal2D > 65535.0)
                        outputVal = 65535;
                    else
                        outputVal = (unsigned short)qRound(fVal2D);

                    it_3D.Set(outputVal);
                    //float tmpVal = (float)(it_3D.Get()); //in proj image case, this is intensity
                    //it_2D.Set(tmpVal);		  
                    ++it_2D;
                    ++it_3D;
                }//while2
                it_3D.NextLine();
            }//while1
            break;
        }
        //
        it_3D.NextSlice();
    }//end of for
}


void QUTIL::Get2DFrom3DByIndex(UShortImageType::Pointer& spSrcImg3D, UShortImage2DType::Pointer& spTargetImg2D, int idx, enPLANE iDirection)
{
    if (!spSrcImg3D)
        return;

    int idxHor = 0;
    int idxVer = 0;
    int idxZ = 0;

    switch (iDirection)
    {
    case PLANE_AXIAL:
        idxHor = 0;
        idxVer = 1;
        idxZ = 2;
        break;
    case PLANE_FRONTAL:
        idxHor = 0;
        idxVer = 2;
        idxZ = 1;
        break;
    case PLANE_SAGITTAL:
        idxHor = 1;
        idxVer = 2;
        idxZ = 0;
        break;
    }

    //Create 2D target image based on geometry of 3D
    UShortImageType::SizeType imgDim = spSrcImg3D->GetBufferedRegion().GetSize();
    UShortImageType::SpacingType spacing = spSrcImg3D->GetSpacing();
    UShortImageType::PointType origin = spSrcImg3D->GetOrigin();

    int width = imgDim[idxHor];
    int height = imgDim[idxVer];
    int zSize = imgDim[idxZ];
    //cout << "Get2DFrom3D zSize = " << zSize << endl;

    if (idx < 0 || idx >= zSize)
    {
        cout << "Error! idx is out of the range" << endl;
        return;
    }

    UShortImage2DType::IndexType idxStart;
    idxStart[0] = 0;
    idxStart[1] = 0;

    UShortImage2DType::SizeType size2D;
    size2D[0] = imgDim[idxHor];
    size2D[1] = imgDim[idxVer];

    UShortImage2DType::SpacingType spacing2D;
    spacing2D[0] = spacing[idxHor];
    spacing2D[1] = spacing[idxVer];

    UShortImage2DType::PointType origin2D;
    //  origin2D[0] = origin[idxHor];
    //  origin2D[1] = origin[idxVer];
    origin2D[0] = size2D[0] * spacing2D[0] / -2.0;
    origin2D[1] = size2D[1] * spacing2D[1] / -2.0;

    UShortImage2DType::RegionType region;
    region.SetSize(size2D);
    region.SetIndex(idxStart);

    //spTargetImg2D is supposed to be empty.
    if (spTargetImg2D)
    {
        cout << "something is here in target image. is it gonna be overwritten?" << endl;
    }

    spTargetImg2D = UShortImage2DType::New();
    spTargetImg2D->SetRegions(region);
    spTargetImg2D->SetSpacing(spacing2D);
    spTargetImg2D->SetOrigin(origin2D);

    spTargetImg2D->Allocate();
    spTargetImg2D->FillBuffer(0);

    //cout << "src size = " << spSrcImg3D->GetRequestedRegion().GetSize() << " " << endl;
    //cout << "target image size = " << spTargetImg2D->GetRequestedRegion().GetSize() << " " << endl;


    itk::ImageSliceConstIteratorWithIndex<UShortImageType> it_3D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());    
    itk::ImageRegionIterator<UShortImage2DType> it_2D(spTargetImg2D, spTargetImg2D->GetRequestedRegion());

    it_3D.SetFirstDirection(idxHor);
    it_3D.SetSecondDirection(idxVer);

    it_3D.GoToBegin();
    it_2D.GoToBegin();


    for (int i = 0; i< zSize && !it_3D.IsAtEnd(); i++)
    {
        /*QFileInfo crntFileInfo(arrYKImage[i].m_strFilePath);
        QString crntFileName = crntFileInfo.fileName();
        QString crntPath = strSavingFolder + "/" + crntFileName;*/
        //Search matching slice using slice iterator for m_spProjCTImg	
        //cout << "Get2DFrom3D: Slide= " << i  << " ";

        if (i == idx)
        {
            while (!it_3D.IsAtEndOfSlice()) //Error here why?
            {
                while (!it_3D.IsAtEndOfLine())
                {
                    float tmpVal = (float)(it_3D.Get()); //in proj image case, this is intensity
                    it_2D.Set(tmpVal);
                    ++it_2D;
                    ++it_3D;
                }//while2
                it_3D.NextLine();
            }//while1
            break;
        }	// end if 
        it_3D.NextSlice();
    }	//end of for

    //cout << "cnt = " << cnt << " TotCnt " << cntTot << endl;
    /*YK16GrayImage tmpYK;
    tmpYK.UpdateFromItkImageFloat(spTargetImg2D);
    QString str = QString("D:\\testYK\\InsideFunc_%1.raw").arg(idx);
    tmpYK.SaveDataAsRaw(str.toLocal8Bit().constData());*/
}

void QUTIL::Get2DFrom3DByIndex(FloatImageType::Pointer& spSrcImg3D, FloatImage2DType::Pointer& spTargetImg2D, int idx, enPLANE iDirection)
{
    if (!spSrcImg3D)
        return;

    int idxHor = 0;
    int idxVer = 0;
    int idxZ = 0;

    switch (iDirection)
    {
    case PLANE_AXIAL:
        idxHor = 0;
        idxVer = 1;
        idxZ = 2;
        break;
    case PLANE_FRONTAL:
        idxHor = 0;
        idxVer = 2;
        idxZ = 1;
        break;
    case PLANE_SAGITTAL:
        idxHor = 1;
        idxVer = 2;
        idxZ = 0;
        break;
    }

    //Create 2D target image based on geometry of 3D
    FloatImageType::SizeType imgDim = spSrcImg3D->GetBufferedRegion().GetSize();
    FloatImageType::SpacingType spacing = spSrcImg3D->GetSpacing();
    FloatImageType::PointType origin = spSrcImg3D->GetOrigin();

    int width = imgDim[idxHor];
    int height = imgDim[idxVer];
    int zSize = imgDim[idxZ];
    //cout << "Get2DFrom3D zSize = " << zSize << endl;

    if (idx < 0 || idx >= zSize)
    {
        cout << "Error! idx is out of the range" << endl;
        return;
    }

    FloatImage2DType::IndexType idxStart;
    idxStart[0] = 0;
    idxStart[1] = 0;

    FloatImage2DType::SizeType size2D;
    size2D[0] = imgDim[idxHor];
    size2D[1] = imgDim[idxVer];

    FloatImage2DType::SpacingType spacing2D;
    spacing2D[0] = spacing[idxHor];
    spacing2D[1] = spacing[idxVer];

    FloatImage2DType::PointType origin2D;
    //  origin2D[0] = origin[idxHor];
    //  origin2D[1] = origin[idxVer];
    origin2D[0] = size2D[0] * spacing2D[0] / -2.0;
    origin2D[1] = size2D[1] * spacing2D[1] / -2.0;

    FloatImage2DType::RegionType region;
    region.SetSize(size2D);
    region.SetIndex(idxStart);

    //spTargetImg2D is supposed to be empty.
    if (spTargetImg2D)
    {
        cout << "something is here in target image. is it gonna be overwritten?" << endl;
    }

    spTargetImg2D = FloatImage2DType::New();
    spTargetImg2D->SetRegions(region);
    spTargetImg2D->SetSpacing(spacing2D);
    spTargetImg2D->SetOrigin(origin2D);

    spTargetImg2D->Allocate();
    spTargetImg2D->FillBuffer(0);

    //cout << "src size = " << spSrcImg3D->GetRequestedRegion().GetSize() << " " << endl;
    //cout << "target image size = " << spTargetImg2D->GetRequestedRegion().GetSize() << " " << endl;


    itk::ImageSliceConstIteratorWithIndex<FloatImageType> it_3D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());
    itk::ImageRegionIterator<FloatImage2DType> it_2D(spTargetImg2D, spTargetImg2D->GetRequestedRegion());

    it_3D.SetFirstDirection(idxHor);
    it_3D.SetSecondDirection(idxVer);

    it_3D.GoToBegin();
    it_2D.GoToBegin();


    for (int i = 0; i< zSize && !it_3D.IsAtEnd(); i++)
    {
        /*QFileInfo crntFileInfo(arrYKImage[i].m_strFilePath);
        QString crntFileName = crntFileInfo.fileName();
        QString crntPath = strSavingFolder + "/" + crntFileName;*/
        //Search matching slice using slice iterator for m_spProjCTImg	
        //cout << "Get2DFrom3D: Slide= " << i  << " ";

        if (i == idx)
        {
            while (!it_3D.IsAtEndOfSlice()) //Error here why?
            {
                while (!it_3D.IsAtEndOfLine())
                {
                    float tmpVal = (float)(it_3D.Get()); //in proj image case, this is intensity
                    it_2D.Set(tmpVal);
                    ++it_2D;
                    ++it_3D;
                }//while2
                it_3D.NextLine();
            }//while1
            break;
        }	// end if 
        it_3D.NextSlice();
    }	//end of for
}

void QUTIL::Get2DFrom3DByPosition(UShortImageType::Pointer& spSrcImg3D, UShortImage2DType::Pointer& spTargImg2D, enPLANE iDirection, double pos, double& finalPos)
{
    if (!spSrcImg3D)
        return;

    int idxHor = 0;
    int idxVer = 0;
    int idxZ = 0;

    switch (iDirection)
    {
    case PLANE_AXIAL:
        idxHor = 0;
        idxVer = 1;
        idxZ = 2;
        break;
    case PLANE_SAGITTAL:
        idxHor = 1;
        idxVer = 2;
        idxZ = 0;
        break;
    case PLANE_FRONTAL:
        idxHor = 0;
        idxVer = 2;
        idxZ = 1;
        break;

    }

    //Create 2D target image based on geometry of 3D
    UShortImageType::SizeType imgDim = spSrcImg3D->GetBufferedRegion().GetSize();
    UShortImageType::SpacingType spacing = spSrcImg3D->GetSpacing();
    UShortImageType::PointType origin = spSrcImg3D->GetOrigin();

    int width = imgDim[idxHor];
    int height = imgDim[idxVer];
    int zSize = imgDim[idxZ];
    //cout << "Get2DFrom3D zSize = " << zSize << endl

    int iCntSlice = imgDim[idxZ];
    int iReqSlice = qRound((pos - origin[idxZ]) / spacing[idxZ]);

    finalPos = iReqSlice* spacing[idxZ] + origin[idxZ];

    


    if (iReqSlice < 0 || iReqSlice >= iCntSlice)
    {
        //cout << "Error! idx is out of the range" << endl;

        cout << "Error! idx is out of the range" << "iReqSlice= " << iReqSlice <<
            " iCntSlice= " << iCntSlice << endl;
        cout << " iDirection = " << iDirection << endl;
        cout << " pos = " << pos << endl;
        cout << " origin[idxZ] = " << origin[idxZ] << endl;
        cout << " spacing[idxZ] = " << spacing[idxZ] << endl;

        return;
    }

    UShortImage2DType::IndexType idxStart;
    idxStart[0] = 0;
    idxStart[1] = 0;

    UShortImage2DType::SizeType size2D;
    size2D[0] = imgDim[idxHor];
    size2D[1] = imgDim[idxVer];

    UShortImage2DType::SpacingType spacing2D;
    spacing2D[0] = spacing[idxHor];
    spacing2D[1] = spacing[idxVer];

    UShortImage2DType::PointType origin2D;
    origin2D[0] = origin[idxHor];
    origin2D[1] = origin[idxVer];
    //origin2D[0] = size2D[0] * spacing2D[0] / -2.0;
    //origin2D[1] = size2D[1] * spacing2D[1] / -2.0;

    UShortImage2DType::RegionType region;
    region.SetSize(size2D);
    region.SetIndex(idxStart);

    //spTargetImg2D is supposed to be empty.
    /* if (spTargImg2D)
     {
     cout << "something is here in target image. is it gonna be overwritten?" << endl;
     }*/

    spTargImg2D = UShortImage2DType::New();
    spTargImg2D->SetRegions(region);
    spTargImg2D->SetSpacing(spacing2D);
    spTargImg2D->SetOrigin(origin2D);

    spTargImg2D->Allocate();
    spTargImg2D->FillBuffer(0);

    itk::ImageSliceConstIteratorWithIndex<UShortImageType> it_3D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());    
    itk::ImageRegionIterator<UShortImage2DType> it_2D(spTargImg2D, spTargImg2D->GetRequestedRegion());

    it_3D.SetFirstDirection(idxHor);
    it_3D.SetSecondDirection(idxVer);

    it_3D.GoToBegin();
    it_2D.GoToBegin();
    
    for (int i = 0; i< iCntSlice && !it_3D.IsAtEnd(); i++)
    {
        if (i == iReqSlice)
        {
            while (!it_3D.IsAtEndOfSlice()) //Error here why?
            {
                while (!it_3D.IsAtEndOfLine())
                {
                    float tmpVal = (float)(it_3D.Get()); //in proj image case, this is intensity
                    it_2D.Set(tmpVal);
                    ++it_2D;
                    ++it_3D;
                }//while2
                it_3D.NextLine();
            }//while1
            break;
        }	// end if 
        it_3D.NextSlice();
    }	//end of for  
    
}

void QUTIL::Get2DFrom3DByPosition(FloatImageType::Pointer& spSrcImg3D, FloatImage2DType::Pointer& spTargImg2D, enPLANE iDirection, double pos, double& finalPos)
{
    if (!spSrcImg3D)
        return;

    int idxHor = 0;
    int idxVer = 0;
    int idxZ = 0;
    //bool bUpDownFlip = false;

    switch (iDirection)
    {
    case PLANE_AXIAL:
        idxHor = 0;
        idxVer = 1;
        idxZ = 2;
        break;
    case PLANE_SAGITTAL:
        idxHor = 1;
        idxVer = 2;
        idxZ = 0;
        //bUpDownFlip = true;
        break;
    case PLANE_FRONTAL:
        idxHor = 0;
        idxVer = 2;
        idxZ = 1;
        //bUpDownFlip = true;
        break;  
    }

    //Create 2D target image based on geometry of 3D
    FloatImageType::SizeType imgDim = spSrcImg3D->GetBufferedRegion().GetSize();
    FloatImageType::SpacingType spacing = spSrcImg3D->GetSpacing();
    FloatImageType::PointType origin = spSrcImg3D->GetOrigin();

    int width = imgDim[idxHor];
    int height = imgDim[idxVer];
    int zSize = imgDim[idxZ];
    //cout << "Get2DFrom3D zSize = " << zSize << endl

    int iCntSlice = imgDim[idxZ];
    int iReqSlice = qRound((pos - origin[idxZ]) / spacing[idxZ]);


    finalPos = iReqSlice* spacing[idxZ] + origin[idxZ];


    if (iReqSlice < 0 || iReqSlice >= iCntSlice)
    {
        cout << "Error! idx is out of the range" << "iReqSlice= " << iReqSlice <<
            " iCntSlice= " << iCntSlice << endl;
        cout << " iDirection = " << iDirection << endl;
        cout << " pos = " << pos << endl;
        cout << " origin[idxZ] = " << origin[idxZ] << endl;
        cout << " spacing[idxZ] = " << spacing[idxZ] << endl;

        return;
    }

    FloatImage2DType::IndexType idxStart2D;
    idxStart2D[0] = 0;
    idxStart2D[1] = 0;

    FloatImage2DType::SizeType size2D;
    size2D[0] = imgDim[idxHor];
    size2D[1] = imgDim[idxVer];

    FloatImage2DType::SpacingType spacing2D;
    spacing2D[0] = spacing[idxHor];
    spacing2D[1] = spacing[idxVer];

    FloatImage2DType::PointType origin2D;
    origin2D[0] = origin[idxHor];
    origin2D[1] = origin[idxVer];
    //origin2D[0] = size2D[0] * spacing2D[0] / -2.0;
    //origin2D[1] = size2D[1] * spacing2D[1] / -2.0;

    FloatImage2DType::RegionType region;
    region.SetSize(size2D);
    region.SetIndex(idxStart2D);

    //spTargetImg2D is supposed to be empty.
    /* if (spTargImg2D)
    {
    cout << "something is here in target image. is it gonna be overwritten?" << endl;
    }*/

    spTargImg2D = FloatImage2DType::New();
    spTargImg2D->SetRegions(region);
    spTargImg2D->SetSpacing(spacing2D);
    spTargImg2D->SetOrigin(origin2D);

    spTargImg2D->Allocate();
    spTargImg2D->FillBuffer(0);

    itk::ImageSliceConstIteratorWithIndex<FloatImageType> it_3D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());
    itk::ImageRegionIterator<FloatImage2DType> it_2D(spTargImg2D, spTargImg2D->GetRequestedRegion());

    it_3D.SetFirstDirection(idxHor);
    it_3D.SetSecondDirection(idxVer);

    it_3D.GoToBegin();
    it_2D.GoToBegin();    

    for (int i = 0; i< iCntSlice && !it_3D.IsAtEnd(); i++)
    {
        if (i == iReqSlice)
        {
            while (!it_3D.IsAtEndOfSlice()) //Error here why?
            {
                while (!it_3D.IsAtEndOfLine())
                {
                    float tmpVal = (float)(it_3D.Get()); //in proj image case, this is intensity
                    it_2D.Set(tmpVal);
                    ++it_2D;
                    ++it_3D;
                }//while2
                it_3D.NextLine();
            }//while1
            break;
        }	// end if 
        it_3D.NextSlice();
    }	//end of for
}

bool QUTIL::GetProfile1DByPosition(UShortImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, float fixedPos, enPROFILE_DIRECTON enDirection)
{
    if (!spSrcImg2D)
        return false;

    UShortImage2DType::SizeType imgDim = spSrcImg2D->GetBufferedRegion().GetSize();
    UShortImage2DType::SpacingType spacing = spSrcImg2D->GetSpacing();
    UShortImage2DType::PointType origin = spSrcImg2D->GetOrigin();

    int width = imgDim[0];
    int height = imgDim[1];

    //itk::ImageSliceConstIteratorWithIndex<FloatImage2DType> it_2D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());

    itk::ImageLinearConstIteratorWithIndex<UShortImage2DType> it_2D(spSrcImg2D, spSrcImg2D->GetRequestedRegion());
        
     //::SetDirection(unsigned int 	direction)

    if (!vProfile.empty())
    {
        vProfile.clear();
    }

    QPointF curPt;
    
    /*int fixedIdx = 0;
    int movingIdx = 0;*/  


    float fValX = 0.0;
    float fValY = 0.0;

    it_2D.GoToBegin();

    if (enDirection == PRIFLE_HOR)
    {
        int fixedY = qRound(fixedPos - origin[1]) / spacing[1];
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (i == fixedY)
                {
                    fValX = (double)(j*spacing[0]) + origin[0];
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }        
    else if (enDirection == PRIFLE_VER)
    {
        int fixedX = qRound(fixedPos - origin[0]) / spacing[0];                
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (j == fixedX)
                {
                    fValX = (double)(i*spacing[1]) + origin[1]; // ((i - origin[1])*spacing[1]);
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }

    if (vProfile.empty())
        return false;
    

    return true;       
}

bool QUTIL::GetProfile1DByPosition(FloatImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, float fixedPos, enPROFILE_DIRECTON enDirection)
{
    if (!spSrcImg2D)
        return false;

    FloatImage2DType::SizeType imgDim = spSrcImg2D->GetBufferedRegion().GetSize();
    FloatImage2DType::SpacingType spacing = spSrcImg2D->GetSpacing();
    FloatImage2DType::PointType origin = spSrcImg2D->GetOrigin();

    int width = imgDim[0];
    int height = imgDim[1];

    //itk::ImageSliceConstIteratorWithIndex<FloatImage2DType> it_2D(spSrcImg3D, spSrcImg3D->GetRequestedRegion());

    itk::ImageLinearConstIteratorWithIndex<FloatImage2DType> it_2D(spSrcImg2D, spSrcImg2D->GetRequestedRegion());

    //::SetDirection(unsigned int 	direction)

    if (!vProfile.empty())
    {
        vProfile.clear();
    }

    QPointF curPt;

    /*int fixedIdx = 0;
    int movingIdx = 0;*/


    float fValX = 0.0;
    float fValY = 0.0;

    it_2D.GoToBegin();

    if (enDirection == PRIFLE_HOR)
    {
        int fixedY = qRound((fixedPos - origin[1]) / spacing[1]);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (i == fixedY)
                {
                    fValX = (double)(j*spacing[0]) + origin[0];
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }
    else if (enDirection == PRIFLE_VER)
    {
        //cout << "PRIFLE_VER" << endl;
        int fixedX = qRound((fixedPos - origin[0]) / spacing[0]);

        //cout << "fixedX= " << fixedX << endl;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (j == fixedX)
                {
                    fValX = (double)(i*spacing[1]) + origin[1];
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }

    if (vProfile.empty())
        return false;


    return true;
}

bool QUTIL::GetProfile1DByIndex(UShortImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, int fixedIndex, enPROFILE_DIRECTON enDirection)
{
    if (!spSrcImg2D)
        return false;

    UShortImage2DType::SizeType imgDim = spSrcImg2D->GetBufferedRegion().GetSize();
    UShortImage2DType::SpacingType spacing = spSrcImg2D->GetSpacing();
    UShortImage2DType::PointType origin = spSrcImg2D->GetOrigin();

    int width = imgDim[0];
    int height = imgDim[1];

    itk::ImageLinearConstIteratorWithIndex<UShortImage2DType> it_2D(spSrcImg2D, spSrcImg2D->GetRequestedRegion());

    //::SetDirection(unsigned int 	direction)

    if (!vProfile.empty())
    {
        vProfile.clear();
    }

    QPointF curPt;

    /*int fixedIdx = 0;
    int movingIdx = 0;*/


    float fValX = 0.0;
    float fValY = 0.0;

    it_2D.GoToBegin();

    if (enDirection == PRIFLE_HOR)
    {
        int fixedY = fixedIndex;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (i == fixedY)
                {
                    fValX = (double)((j - origin[0])*spacing[0]);
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }
    else if (enDirection == PRIFLE_VER)
    {
        int fixedX = fixedIndex;

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (j == fixedX)
                {
                    fValX = (double)((i - origin[1])*spacing[1]);
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }

    if (vProfile.empty())
        return false;


    return true;

}

bool QUTIL::GetProfile1DByIndex(FloatImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, int fixedIndex, enPROFILE_DIRECTON enDirection)
{
    if (!spSrcImg2D)
        return false;

    FloatImage2DType::SizeType imgDim = spSrcImg2D->GetBufferedRegion().GetSize();
    FloatImage2DType::SpacingType spacing = spSrcImg2D->GetSpacing();
    FloatImage2DType::PointType origin = spSrcImg2D->GetOrigin();

    int width = imgDim[0];
    int height = imgDim[1];    

    itk::ImageLinearConstIteratorWithIndex<FloatImage2DType> it_2D(spSrcImg2D, spSrcImg2D->GetRequestedRegion());

    //::SetDirection(unsigned int 	direction)

    if (!vProfile.empty())
    {
        vProfile.clear();
    }

    QPointF curPt;

    /*int fixedIdx = 0;
    int movingIdx = 0;*/


    float fValX = 0.0;
    float fValY = 0.0;

    it_2D.GoToBegin();

    if (enDirection == PRIFLE_HOR)
    {
        int fixedY = fixedIndex;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (i == fixedY)
                {
                    fValX = (double)((j - origin[0])*spacing[0]);
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }
    else if (enDirection == PRIFLE_VER)
    {
        int fixedX = fixedIndex;

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (j == fixedX)
                {
                    fValX = (double)((i - origin[1])*spacing[1]);
                    fValY = (double)(it_2D.Get());
                    curPt.setX(fValX);
                    curPt.setY(fValY);

                    vProfile.push_back(curPt);
                }
                ++it_2D;
            }
            if (it_2D.IsAtEnd())
                break;
        }
    }

    if (vProfile.empty())
        return false;


    return true;

}

void QUTIL::LoadFloatImage2D(const char* filePath, FloatImage2DType::Pointer& spTargImg2D)
{
    typedef itk::ImageFileReader<FloatImage2DType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();

    QString strPath = filePath;

    if (strPath.length() < 1)
        return;

    reader->SetFileName(strPath.toLocal8Bit().constData());
    reader->Update();

    spTargImg2D = reader->GetOutput();
}

void QUTIL::LoadFloatImage3D(const char* filePath, FloatImageType::Pointer& spTargImg3D)
{
    typedef itk::ImageFileReader<FloatImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();//error!

    QString strPath = filePath;


    //strPath = "D:\\RD_Beam2\\RD_Beam1_comp.mha";
    if (strPath.length() < 1)
        return;

    reader->SetFileName(strPath.toLocal8Bit().constData());
    reader->Update();

    spTargImg3D = reader->GetOutput();
}

void QUTIL::SaveFloatImage2D(const char* filePath, FloatImage2DType::Pointer& spSrcImg2D)
{
    if (!spSrcImg2D)
        return;

    QString strPath = filePath;

    if (strPath.length() < 1)
        return;

    typedef itk::ImageFileWriter<FloatImage2DType> WriterType;
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(strPath.toLocal8Bit().constData());
    writer->SetUseCompression(true); //not exist in original code (rtkfdk)	
    writer->SetInput(spSrcImg2D);
    writer->Update();

    cout << "Writing image file was succeeded: " << strPath.toLocal8Bit().constData() << endl;
}

void QUTIL::SaveFloatImage3D(const char* filePath, FloatImageType::Pointer& spSrcImg3D)
{
    if (!spSrcImg3D)
        return;

    QString strPath = filePath;

    if (strPath.length() < 1)
        return;

    typedef itk::ImageFileWriter<FloatImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(strPath.toLocal8Bit().constData());
    writer->SetUseCompression(true); //not exist in original code (rtkfdk)	
    writer->SetInput(spSrcImg3D);
    writer->Update();

    cout << "Writing image file was succeeded: " << strPath.toLocal8Bit().constData() << endl;   

}

QStringList QUTIL::LoadTextFile(const char* txtFilePath)
{
    QStringList resultStrList;

    ifstream fin;
    fin.open(txtFilePath);

    if (fin.fail())
        return resultStrList;

    char str[MAX_LINE_LENGTH];    

    while (!fin.eof())
    {
        memset(str, 0, MAX_LINE_LENGTH);
        fin.getline(str, MAX_LINE_LENGTH);
        QString tmpStr = QString(str);

        resultStrList.append(tmpStr);
        //resultStrList.append("\n");
    }
   
    fin.close();

    return resultStrList;
}

void QUTIL::LoadColorTableFromFile(const char* filePath, vector<VEC3D>& vRGBTable)
{
    vRGBTable.clear();

    QStringList resultStrList;

    ifstream fin;
    fin.open(filePath);

    if (fin.fail())
    {
        cout << "No such file found: " << filePath << endl;
        return;
    }     

    char str[MAX_LINE_LENGTH];
    VEC3D curRGB;
    while (!fin.eof())
    {
        memset(str, 0, MAX_LINE_LENGTH);
        fin.getline(str, MAX_LINE_LENGTH);
        QString tmpStr = QString(str);
        resultStrList = tmpStr.split("\t");

        if (resultStrList.count() == 3)
        {
            curRGB.x = resultStrList.at(0).toFloat();
            curRGB.y = resultStrList.at(1).toFloat();
            curRGB.z = resultStrList.at(2).toFloat();
        }
        vRGBTable.push_back(curRGB);
    }
    fin.close();
    return;
}

void QUTIL::LoadColorTableInternal(vector<VEC3D>& vRGBTable, enCOLOR_TABLE col_table)
{
    vRGBTable.clear();   

    int i = 0;

    int iCntItem = 0;
    VEC3D curRGB;

    if (col_table == COL_TABLE_JET)
    {
        iCntItem = NUM_OF_TBL_ITEM_JET;

        for (int i = 0; i < iCntItem; i++)
        {
            curRGB.x = (float)colormap_jet[i][0];
            curRGB.y = (float)colormap_jet[i][1];
            curRGB.z = (float)colormap_jet[i][2];
        }       
        vRGBTable.push_back(curRGB);
    }
    else if (col_table == COL_TABLE_GAMMA)
    {
        iCntItem = NUM_OF_TBL_ITEM_GAMMA;

        for (int i = 0; i < iCntItem; i++)
        {
            curRGB.x = (float)colormap_customgamma[i][0];
            curRGB.y = (float)colormap_customgamma[i][1];
            curRGB.z = (float)colormap_customgamma[i][2];
        }
        vRGBTable.push_back(curRGB);
    }    

    return;
}


VEC3D QUTIL::GetRGBValueFromTable(vector<VEC3D>& vRGBTable, float fMinGray, float fMaxGray, float fLookupGray)
{
    VEC3D resultRGB = { 0.0, 0.0, 0.0 };

    float width = fMaxGray - fMinGray;

    if (width <= 0)
        return resultRGB;

    float fractionGray = (fLookupGray - fMinGray) / width;

    int numDiscret = vRGBTable.size();

    if (numDiscret < 1)
        return resultRGB;

    int lookupIdx = qRound(fractionGray*numDiscret);

    if (lookupIdx < numDiscret)
    {
        resultRGB = vRGBTable.at(lookupIdx);
    }
    else
    {
        resultRGB = vRGBTable.at(numDiscret - 1);
    }
    return resultRGB;
}

QString QUTIL::GetTimeStampDirPath(const QString& curDirPath, const QString& preFix, const QString& endFix)
{
    QDate curDate = QDate::currentDate();
    QString strDateStamp = curDate.toString("YYMMDD");
    QTime curTime = QTime::currentTime();
    QString strTimeStamp = curTime.toString("hhmmss");
    //QDir tmpPlmDir = QDir(curDirPath);

  /*  if (!tmpPlmDir.exists())
    {
        cout << "Error! No curDirPath is available." << tmpPlmDir.absolutePath().toLocal8Bit().constData() << endl;
        return;
    }*/

    QString strOutput = curDirPath + "/" + preFix + strDateStamp+"_" +strTimeStamp + endFix;

    return strOutput;
}


QString QUTIL::GetTimeStampDirName(const QString& preFix, const QString& endFix)
{
    QDate curDate = QDate::currentDate();
    QString strDateStamp = curDate.toString("yyMMdd");
    QTime curTime = QTime::currentTime();
    QString strTimeStamp = curTime.toString("hhmmss");   

    QString strOutput = preFix + strDateStamp + "_" + strTimeStamp + endFix;
    return strOutput;
}

void QUTIL::ShowErrorMessage(QString str)
{
    cout << str.toLocal8Bit().constData() << endl;

    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.exec();
    
}

void QUTIL::CreateItkDummyImg(FloatImageType::Pointer& spTarget, int sizeX, int sizeY, int sizeZ, float fillVal)
{
    FloatImageType::IndexType idxStart;
    idxStart[0] = 0;
    idxStart[1] = 0;
    idxStart[2] = 0;

    FloatImageType::SizeType size3D;
    size3D[0] = sizeX;
    size3D[1] = sizeY;
    size3D[2] = sizeZ;

    FloatImageType::SpacingType spacing3D;
    spacing3D[0] = 1;
    spacing3D[1] = 1;
    spacing3D[2] = 1;

    FloatImageType::PointType origin3D;
    
    origin3D[0] = size3D[0] * spacing3D[0] / -2.0;
    origin3D[1] = size3D[1] * spacing3D[1] / -2.0;

    FloatImageType::RegionType region;
    region.SetSize(size3D);
    region.SetIndex(idxStart);

    //spTargetImg2D is supposed to be empty.
    if (spTarget)
    {
        cout << "something is here in target image. is it gonna be overwritten?" << endl;
    }

    spTarget = FloatImageType::New();
    spTarget->SetRegions(region);
    spTarget->SetSpacing(spacing3D);
    spTarget->SetOrigin(origin3D);

    spTarget->Allocate();
    spTarget->FillBuffer(fillVal);
}

void QUTIL::PrintStrList(QStringList& strList)
{
    int size = strList.count();

    for (int i = 0; i < size; i++)
    {
        cout << strList.at(i).toLocal8Bit().constData() << endl;
    }
}

QString QUTIL::GetPathWithEndFix(const QString& curFilePath, const QString& strEndFix)
{
    QFileInfo fInfo(curFilePath);
    /*QString strDirPath = fInfo.absolutePath();
    QString strBaseName = fInfo.completeBaseName();
    QString strSuffixName = fInfo.completeSuffix();*/

    QString strResult = fInfo.absolutePath() + "/" + fInfo.completeBaseName() + strEndFix + "." + fInfo.completeSuffix();
    return strResult;
}

//
//void QUTIL::UpdateFloatTable3(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3,
//    QStandardItemModel* pTableModel, gamma_gui* pParent)
//{ 
//    int numOfData = 3;
//
//    if (pTableModel != NULL)
//    {
//        delete pTableModel;
//        pTableModel = NULL;
//    }
//
//    int columnSize = 1;
//    int rowSize1, rowSize2, rowSize3 = 0;
//
//    columnSize = numOfData * 2;
//
//    rowSize1 = vData1.size();
//    rowSize2 = vData2.size();
//    rowSize3 = vData3.size();
//
//    int maxRowSize = 0;
//    if (rowSize1 > rowSize2)
//    {
//        if (rowSize1 < rowSize3)
//            maxRowSize = rowSize3;
//        else
//            maxRowSize = rowSize1;
//
//    }
//    else
//    {
//        if (rowSize2 < rowSize3)
//            maxRowSize = rowSize3;
//        else
//            maxRowSize = rowSize2;
//    }
//
//    if (maxRowSize == 0)
//    {
//        cout << "MaxRowSize is 0" << endl;
//        return;
//    }
//     
//
//    pTableModel = new QStandardItemModel(maxRowSize, columnSize, pParent); //2 Rows and 3 Columns
//    pTableModel->setHorizontalHeaderItem(0, new QStandardItem(QString("x1")));
//    pTableModel->setHorizontalHeaderItem(1, new QStandardItem(QString("y1")));
//    pTableModel->setHorizontalHeaderItem(2, new QStandardItem(QString("x2")));
//    pTableModel->setHorizontalHeaderItem(3, new QStandardItem(QString("y2")));
//    pTableModel->setHorizontalHeaderItem(4, new QStandardItem(QString("x3")));
//    pTableModel->setHorizontalHeaderItem(5, new QStandardItem(QString("y3")));    
//    
//
//    for (int i = 0; i < maxRowSize; i++)
//    {
//        qreal tmpVal1 = vData1.at(i).x();
//        qreal tmpVal2 = vData1.at(i).y();
//
//        pTableModel->setItem(i, 0, new QStandardItem(QString("%1").arg(tmpVal1)));        
//        pTableModel->setItem(i, 1, new QStandardItem(QString("%1").arg(tmpVal2)));
//
//        if (i < rowSize2)
//        {
//
//            tmpVal1 = vData2.at(i).x();
//            tmpVal2 = vData2.at(i).y();
//            pTableModel->setItem(i, 2, new QStandardItem(QString("%1").arg(tmpVal1)));
//            pTableModel->setItem(i, 3, new QStandardItem(QString("%1").arg(tmpVal2)));
//        }
//
//        if (i < rowSize3)
//        {
//            tmpVal1 = vData3.at(i).x();
//            tmpVal2 = vData3.at(i).y();
//            pTableModel->setItem(i, 4, new QStandardItem(QString("%1").arg(tmpVal1)));
//            pTableModel->setItem(i, 5, new QStandardItem(QString("%1").arg(tmpVal2)));
//        }
//    }
//
//    if (pTableModel == NULL)
//    {
//        cout << "weird!" << endl;
//    }
//}



void QUTIL::GenDefaultCommandFile(QString strPathCommandFile, enRegisterOption regiOption)
{
    ofstream fout;
    fout.open(strPathCommandFile.toLocal8Bit().constData());

    if (fout.fail())
    {
        cout << "File writing error! " << endl;
        return;
    }

    fout << "#Plastimatch command file for registration.txt" << endl;
    fout << "[GLOBAL]" << endl;
    fout << "fixed=" << "TBD" << endl;
    fout << "moving=" << "TBD" << endl;

   /* if (strPathFixedMask.length() > 1)
    {
        fout << "fixed_roi=" << "TBD" << endl;
    }*/
    fout << "img_out=" << "TBD" << endl;
    fout << "xform_out=" << "TBD" << endl;
    fout << endl;
    
    //QString strOptim = "mse";    
    QString optionStr;
    QStringList optionList;

    switch (regiOption)
    {
    case PLAST_RIGID:
        fout << "[STAGE]" << endl;
        fout << "xform=" << "rigid" << endl;        
        fout << "optim=" << "versor" << endl;
        fout << "impl=" << "itk" << endl;
        fout << "threading=" << "openmp" << endl;
        fout << "background_val=" << "-1024" << endl;
        //fout << "background_val=" << "0" << endl; //-600 in HU //added
        fout << "max_its=" << "50" << endl;
        break;

    case PLAST_AFFINE:
        fout << "[STAGE]" << endl;
        fout << "xform=" << "rigid" << endl;
        fout << "optim=" << "versor" << endl;
        fout << "impl=" << "itk" << endl;
        fout << "threading=" << "openmp" << endl;
        fout << "background_val=" << "-1024" << endl;
        //fout << "background_val=" << "0" << endl; //-600 in HU //added
        fout << "max_its=" << "50" << endl;
        fout << endl;
        break;

    case PLAST_GRADIENT:
        fout << "#For gradient-based searching, moving image should be smaller than fixed image. So, CBCT image might move rather than CT" << endl;

        optionStr = "0.7, 0.7, 0.7";
        optionList = optionStr.split(",");

        fout << "[PROCESS]" << endl;
        fout << "action=adjust" << endl;
        fout << "# only consider within this  intensity values" << endl;
        fout << "parms=-inf,0,-1000,-1000,4000,4000,inf,0" << endl;
        fout << "images=fixed,moving" << endl;
        fout << endl;
        fout << "[STAGE]" << endl;
        fout << "metric=gm" << endl;
        fout << "xform=translation" << endl;
        fout << "optim=grid_search" << endl;
        fout << "gridsearch_min_overlap=" << optionList.at(0).toDouble() << " "
            << optionList.at(1).toDouble() << " "
            << optionList.at(2).toDouble() << endl;

        fout << "num_substages=5" << endl;
        //fout << "debug_dir=" << m_strPathPlastimatch.toLocal8Bit().constData() << endl;
        break;

    case PLAST_BSPLINE:        
            fout << "[STAGE]" << endl;
            fout << "xform=" << "bspline" << endl;
            fout << "impl=" << "plastimatch" << endl;            
            fout << "threading=" << "openmp" << endl;
            fout << "regularization_lambda=" << 0.005 << endl;            
            fout << "metric=" << "mse" << endl;
            fout << "max_its=" << 30 << endl;
            fout << "grid_spac=" << "30" << " " << "30" << " " << "30" << endl;//20 20 20 --> minimum
            fout << "res=" << "2" << " " << "2" << " " << "1" << endl;
            fout << "background_val=" << "-1024" << endl; //-600 in HU //added
           // fout << "img_out=" << "TBD" << endl;
            fout << endl;
        break; 
    }
    fout.close();    
}


void QUTIL::GetGeometricLimitFloatImg(FloatImageType::Pointer& spFloatImg, VEC3D& limitStart, VEC3D& limitEnd)
{
    if (!spFloatImg)
    {
        limitStart.x = 0.0;
        limitStart.y = 0.0;
        limitStart.z = 0.0;
        limitEnd.x = 0.0;
        limitEnd.y = 0.0;
        limitEnd.z = 0.0;
        return;
    }
     
    FloatImageType::SizeType imgSize = spFloatImg->GetLargestPossibleRegion().GetSize();
    FloatImageType::PointType origin = spFloatImg->GetOrigin();
    FloatImageType::SpacingType spacing = spFloatImg->GetSpacing();

    limitStart.x = origin[0];
    limitStart.y = origin[1];
    limitStart.z = origin[2];
    limitEnd.x = limitStart.x + (imgSize[0] - 1)*spacing[0];
    limitEnd.y = limitStart.y + (imgSize[1] - 1)*spacing[1];
    limitEnd.z = limitStart.z + (imgSize[2] - 1)*spacing[2];
}

void QUTIL::Get1DProfileFromTable(QStandardItemModel* pTable, int iCol_X, int iCol_Y, vector<QPointF>& vOutDoseProfile)
{
    vOutDoseProfile.clear();

    QStringList list;

    int rowCnt = pTable->rowCount();
    int columnCnt = pTable->columnCount();

    if (iCol_X >= columnCnt && iCol_Y >= columnCnt)
    {
        cout << "Error! Wrong column number." << endl;
        return;
    }
    if (rowCnt < 1)
    {
        cout << "Error! This table is empty" << endl;
        return;
    }

    for (int i = 0; i < rowCnt; i++)
    {
        QPointF dataPt;

        QStandardItem* itemX = pTable->item(i, iCol_X);
        QStandardItem* itemY = pTable->item(i, iCol_Y);

        dataPt.setX(itemX->text().toDouble());
        dataPt.setY(itemY->text().toDouble());

        vOutDoseProfile.push_back(dataPt);
    }
}

void QUTIL::ResampleFloatImg(FloatImageType::Pointer& spFloatInput, FloatImageType::Pointer& spFloatOutput, VEC3D& newSpacing)
{
    float old_spacing[3];

    old_spacing[0] = spFloatInput->GetSpacing()[0];
    old_spacing[1] = spFloatInput->GetSpacing()[1];
    old_spacing[2] = spFloatInput->GetSpacing()[2];

    float NewSpacing[3];
    NewSpacing[0] = newSpacing.x;
    NewSpacing[1] = newSpacing.y;
    NewSpacing[2] = newSpacing.z;
    
    
    spFloatOutput = resample_image(spFloatInput, NewSpacing);

    if (spFloatOutput)
    {
        //cout << "Resampling is done" << "From " << old_spacing << " To " << NewSpacing << endl;
    }  
}

const char *
QUTIL::c_str (const QString& s)
{
    return s.toUtf8().constData();
}
