#include "qyklabel.h"
#include <QPainter>
#include "YK16GrayImage.h"

using namespace std;

qyklabel::qyklabel(QWidget *parent)
	: QLabel(parent)
{
	m_pYK16Image = NULL;
	//this->width();
	//m_Rt = this->rect();

	//m_Rt.setRect()
	m_bDrawPoints = true;

	m_iMouseWheelDelta = 0;

	m_bFocusIn = false;

}

qyklabel::~qyklabel()
{
}

void qyklabel::mouseMoveEvent( QMouseEvent *ev )
{
	if (m_pYK16Image == NULL)
		return;
	
	this->x	= ev->x();
	this->y = ev->y();
	emit Mouse_Move();
}

void qyklabel::mousePressEvent( QMouseEvent *ev )
{
	if (m_pYK16Image == NULL)
		return;
	this->x	= ev->x();
	this->y = ev->y();

	if (ev->button() == Qt::LeftButton)
	{	 
		emit Mouse_Pressed_Left();
	}
	if (ev->button() == Qt::RightButton)
	{	  
		emit Mouse_Pressed_Right();
	}
}


void qyklabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (m_pYK16Image == NULL)
        return;

    if (ev->button() == Qt::LeftButton)
    {
        emit Mouse_Left_DoubleClick();
    }
    if (ev->button() == Qt::RightButton)
    {
        emit Mouse_Right_DoubleClick();
    }
}


void qyklabel::mouseReleaseEvent( QMouseEvent *ev )
{
  if (m_pYK16Image == NULL)
	return;

  //cout << "Mouse Released from qlabel" << endl;//it worked

  if (ev->button() == Qt::LeftButton)
  {	
	emit Mouse_Released_Left();
  }
  if (ev->button() == Qt::RightButton)
  {
	emit Mouse_Released_Right();  
  }
}


void qyklabel::wheelEvent( QWheelEvent * event )
{
  if (m_pYK16Image == NULL)
	return;

  m_iMouseWheelDelta = (int)(event->delta()/100.0);
  emit Mouse_Wheel();
}

void qyklabel::enterEvent (QEvent *)
{
  if (m_pYK16Image == NULL)
	return;

  m_bFocusIn = true;

  emit FocusIn();
}

void qyklabel::leaveEvent( QEvent * )
{  
	if (m_pYK16Image == NULL)
		return;	


	m_bFocusIn = false;

	emit FocusOut();
}

//void focusInEvent ( QFocusEvent * ev );
//void focusOutEvent ( QFocusEvent * ev );
//
//void qyklabel::focusInEvent( QFocusEvent * )
//{
//  cout <<"focus in" << endl;
//  emit FocusIn();  
//}
//
//void qyklabel::focusOutEvent( QFocusEvent * )
//{
//  cout <<"focus out" << endl;
//  emit FocusOut();  
//}

void qyklabel::paintEvent( QPaintEvent * )
{
	QPainter painter(this);

	painter.setPen( QPen(Qt::black, 2));
	QRect TargetRt = this->rect();	
	painter.drawRect(TargetRt);

	if (m_pYK16Image == NULL)
		return;

	if (m_pYK16Image->m_iWidth < 1 || m_pYK16Image->m_iHeight<1)
		return;

        double VH_ratio = 0.0; //if horizontal is longer than vertical        

        bool bHorLonger = false;

        double physHor = 0.0;
        double physVer = 0.0;

        int labelNewFixedWidth = 0;
        int labelNewFixedHeight = 0;
        

        if (m_pYK16Image->m_fSpacingX*m_pYK16Image->m_fSpacingY == 0)
        {         
            physHor = (double)m_pYK16Image->m_iWidth;
            physVer = (double)m_pYK16Image->m_iHeight;
        }
        else
        {
            physHor = m_pYK16Image->m_iWidth * m_pYK16Image->m_fSpacingX;
            physVer = m_pYK16Image->m_iHeight* m_pYK16Image->m_fSpacingY;
        }

        VH_ratio = physVer / physHor;

        if (physHor > physVer)            
        {
            bHorLonger = true;           
            int newFixedHeight = qRound(this->width() * VH_ratio);
            this->setFixedHeight(newFixedHeight);
        }
        else
        {
            bHorLonger = false;           
            int newFixedWidth = qRound(this->height() / VH_ratio);
            this->setFixedWidth(newFixedWidth);
        }            


	//Calculate ver / hor ratio.
	/*VH_ratio = m_pYK16Image->m_iHeight / (double) m_pYK16Image->m_iWidth;
	int newFixedHeight = qRound(this->width() * VH_ratio);
	this->setFixedHeight(newFixedHeight);*/

	if (m_pYK16Image != NULL)
	{
		//QRect imgSrcRect;
		//imgSrcRect.setRect(0,0,m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);
		//painter.drawImage(rect(), m_pYK16Image->m_QImage,imgSrcRect,);

		//int width =  m_pYK16Image->m_QImage.width();
		//int height =  m_pYK16Image->m_QImage.height();

		//m_pYK16Image->m_QImage.save("C:\\111.png");

		//QImage tmpQImage = QImage("C:\\FromFillPixmap.png");

		//QImage tmpQImage = m_pYK16Image->m_QImage;
	
		//painter.drawImage(TargetRt, m_pYK16Image->m_QImage, imgSrcRect, QT::RGB888);
		//painter.drawImage(TargetRt, m_pYK16Image->m_QImage, imgSrcRect);
		//painter.drawImage(TargetRt, m_pYK16Image->m_QImage); //it Works!YKTEMP
                painter.drawImage(TargetRt, m_pYK16Image->m_QImage); //it Works!YKTEMP
	}
	
	if (m_bDrawPoints)
	{
		painter.setPen( QPen(Qt::red, 2));
		vector<QPoint>::iterator it;
		for (it = m_vPt.begin() ; it != m_vPt.end() ; it++)
		{
			painter.drawPoint((*it).x(),(*it).y());
		}
	}	

	if (m_pYK16Image->m_bDrawROI)
	{
		painter.setPen( QPen(Qt::red, 2));
		QRect rtDraw; 
		rtDraw.setTopLeft(Data2View(QPoint(m_pYK16Image->m_rtROI.left(), m_pYK16Image->m_rtROI.top()), this->width(), this->height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight));
		rtDraw.setBottomRight(Data2View(QPoint(m_pYK16Image->m_rtROI.right(), m_pYK16Image->m_rtROI.bottom()), this->width(), this->height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight));

		painter.drawRect(rtDraw);
	}
	//if x, y >0 
	if (m_pYK16Image->m_bDrawProfileX)
	{
		QPoint crntViewPt = Data2View(m_pYK16Image->m_ptProfileProbe, this->width(), this->height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);
		painter.setPen( QPen(Qt::red, 1, Qt::DotLine));		
		painter.drawLine(0, crntViewPt.y(), this->width()-1, crntViewPt.y());
		
	}
	if (m_pYK16Image->m_bDrawProfileY)
	{
		QPoint crntViewPt = Data2View(m_pYK16Image->m_ptProfileProbe, this->width(), this->height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);
		painter.setPen( QPen(Qt::red, 1, Qt::DotLine));
		painter.drawLine(crntViewPt.x(), 0 ,crntViewPt.x(), this->height()-1);
	}

	if (m_pYK16Image->m_bDrawFOVCircle)
	{
		painter.setPen( QPen(Qt::yellow, 1, Qt::SolidLine));
		QPoint crntCenterPt = Data2View(m_pYK16Image->m_ptFOVCenter, this->width(), this->height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);		
		int crntRadius = (int)(m_pYK16Image->m_iFOVRadius / (double)m_pYK16Image->m_iWidth * this->width());
		painter.drawEllipse(crntCenterPt,crntRadius, crntRadius);
	}

	if (m_pYK16Image->m_bDrawTableLine)
	{
		painter.setPen( QPen(Qt::yellow, 1, Qt::SolidLine));
		int crntTablePosY = (int)(m_pYK16Image->m_iTableTopPos / (double)m_pYK16Image->m_iHeight * this->height());
		//int crntRadius = (int)(m_pYK16Image->m_iFOVRadius / (double)m_pYK16Image->m_iWidth * this->width());
		painter.drawLine(0, crntTablePosY ,this->width()-1, crntTablePosY);
		//painter.drawEllipse(crntCenterPt,crntRadius, crntRadius);
	}

	if (m_pYK16Image->m_bDrawCrosshair) //objects should be addressed one by one
	{
	  painter.setPen( QPen(Qt::yellow, 1, Qt::SolidLine));
	  //QPoint crosshair;

	  int dispCrossX = (int)(m_pYK16Image->m_ptCrosshair.x() / (double)m_pYK16Image->m_iWidth * this->width());
	  int dispCrossY = (int)(m_pYK16Image->m_ptCrosshair.y() / (double)m_pYK16Image->m_iHeight * this->height());

	  QPoint ptDispCrosshair = GetViewPtFromDataPt(m_pYK16Image->m_ptCrosshair.x(), m_pYK16Image->m_ptCrosshair.y());
		//crosshair.setX(crossX);
		//crosshair.setY(crossY);

		//int crntRadius = (int)(m_pYK16Image->m_iFOVRadius / (double)m_pYK16Image->m_iWidth * this->width());
	  painter.drawLine(0, ptDispCrosshair.y() ,this->width()-1, ptDispCrosshair.y());
	  painter.drawLine(ptDispCrosshair.x(), 0, ptDispCrosshair.x(), this->height()-1);
	}

        if (m_pYK16Image->m_bDrawOverlayText && m_pYK16Image->m_strOverlayText.length() > 1) //objects should be addressed one by one
        {
            /*this->setText(m_pYK16Image->m_strOverlayText);
            this->setFont(QFont("Helvetica", 12, 1, false));
            this->setAlignment(Qt::AlignBottom);*/
            int rtWidth = this->width();
            int rtHeight = this->height() / 7.0;
            int leftPos = 10;
            int topPos = qRound(this->height() - rtHeight);

            QRect rt = QRect(leftPos, topPos, rtWidth, rtHeight);
            painter.setPen(QPen(Qt::white, 3, Qt::SolidLine));
            painter.drawText(rt, m_pYK16Image->m_strOverlayText);
        }
}

void qyklabel::SetBaseImage( YK16GrayImage* pYKImage )
{
	if (pYKImage->m_pData != NULL && !pYKImage->m_QImage.isNull()) //YKTEMP
		m_pYK16Image = pYKImage;
}

void qyklabel::ConvertAndCopyPoints(vector<QPoint>& vSrcPoint, int iDataWidth, int iDataHeight)
{
	m_vPt.clear();	

	int dspWidth = this->width();
	int dspHeight = this->height();


	vector<QPoint>::iterator it;

	for (it = vSrcPoint.begin() ; it != vSrcPoint.end() ; it++)
	{
		QPoint tmpDspPt = Data2View((*it),dspWidth, dspHeight, iDataWidth, iDataHeight);
		m_vPt.push_back(tmpDspPt);
	}
}


QPoint qyklabel::View2Data(QPoint viewPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight)
{
	double fZoomX = viewWidth / (double)dataWidth;
	double fZoomY = viewHeight / (double)dataHeight;

	QPoint dataPt;
	dataPt.setX(qRound(viewPt.x() / fZoomX));
	dataPt.setY(qRound(viewPt.y() / fZoomY));

	return dataPt;
}

QPoint qyklabel::Data2View(QPoint dataPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight)
{
	double fZoomX = viewWidth / (double)dataWidth;
	double fZoomY = viewHeight / (double)dataHeight;

	QPoint viewPt;
	viewPt.setX(qRound(dataPt.x() * fZoomX));
	viewPt.setY(qRound(dataPt.y() * fZoomY));

	return viewPt;
}

void qyklabel::SetDrawPointToggle(bool bToggle )
{
	if (bToggle)
		m_bDrawPoints = true;
	else
		m_bDrawPoints = false;


	update();
}

QPoint qyklabel::GetDataPtFromMousePos()
{
  if (m_pYK16Image == NULL)
	return QPoint(0,0);

  
  if (m_pYK16Image->m_fZoom == 1.0 && m_pYK16Image->m_iOffsetX == 0 && m_pYK16Image->m_iOffsetY == 0)
	return View2Data(QPoint(this->x, this->y), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);  
  else
	return View2DataExt(QPoint(this->x, this->y), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight, QPoint(m_pYK16Image->m_iOffsetX,m_pYK16Image->m_iOffsetY),m_pYK16Image->m_fZoom );

}

QPoint qyklabel::GetDataPtFromViewPt(int viewPtX, int viewPtY)
{
  if (m_pYK16Image == NULL)
	return QPoint(0,0);  

  if (m_pYK16Image->m_fZoom == 1.0 && m_pYK16Image->m_iOffsetX == 0 && m_pYK16Image->m_iOffsetY == 0)
	return View2Data(QPoint(viewPtX, viewPtY), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);  
  else
	return View2DataExt(QPoint(viewPtX, viewPtY), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight, QPoint(m_pYK16Image->m_iOffsetX,m_pYK16Image->m_iOffsetY),m_pYK16Image->m_fZoom ); 
}

QPoint qyklabel::GetViewPtFromDataPt(int dataPtX, int dataPtY)
{
  if (m_pYK16Image == NULL)
	return QPoint(0,0);  

  if (m_pYK16Image->m_fZoom == 1.0 && m_pYK16Image->m_iOffsetX == 0 && m_pYK16Image->m_iOffsetY == 0)
	return Data2View(QPoint(dataPtX, dataPtY), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight);
  else
	return Data2ViewExt(QPoint(dataPtX, dataPtY), width(), height(), m_pYK16Image->m_iWidth, m_pYK16Image->m_iHeight, QPoint(m_pYK16Image->m_iOffsetX,m_pYK16Image->m_iOffsetY),m_pYK16Image->m_fZoom ); 
}



QPoint qyklabel::View2DataExt(QPoint viewPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight, QPoint ptDataOffset, double fUserZoom)
{
  // double fZoomX = viewWidth / (double)dataWidth * fUserZoom; // < 1 
  //double fZoomY = viewHeight / (double)dataHeight * fUserZoom;  

  
  //  dataPt.setX(qRound(viewPt.x() * fZoomX  + ptDataOffset.x()));
  //  dataPt.setY(qRound(viewPt.y() * fZoomY + ptDataOffset.y()));
  int newWidth = qRound(dataWidth/fUserZoom);
  int newHeight = qRound(dataHeight/fUserZoom);

  int dataCenterX = ptDataOffset.x() + qRound(dataWidth/2.0);
  int dataCenterY = ptDataOffset.y() + qRound(dataHeight/2.0);

  int dataLeftTopX = dataCenterX - qRound(newWidth/2.0);//data position
  int dataLeftTopY = dataCenterY - qRound(newHeight/2.0);	//data position


  QPoint dataPt;
  dataPt.setX(qRound(viewPt.x()*newWidth/(double)viewWidth + dataLeftTopX));
  dataPt.setY(qRound(viewPt.y()*newHeight/(double)viewHeight + dataLeftTopY)); 

  return dataPt;
}

QPoint qyklabel::Data2ViewExt( QPoint dataPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight, QPoint ptDataOffset, double fUserZoom )
{
  int newWidth = qRound(dataWidth/fUserZoom);
  int newHeight = qRound(dataHeight/fUserZoom);

  int dataCenterX = ptDataOffset.x() + qRound(dataWidth/2.0);
  int dataCenterY = ptDataOffset.y() + qRound(dataHeight/2.0);

  int dataLeftTopX = dataCenterX - qRound(newWidth/2.0);//data position
  int dataLeftTopY = dataCenterY - qRound(newHeight/2.0);	//data position


  QPoint viewPt;
  viewPt.setX(qRound((dataPt.x() - dataLeftTopX) * viewWidth/(double)newWidth));
  viewPt.setY(qRound((dataPt.y() - dataLeftTopY) * viewHeight/(double)newHeight));  

  return viewPt;
}

//void qyklabel::keyPressEvent( QKeyEvent* ev )
//{  
//  if (!isFocusIn())
//	return;
//
//  int enMovingKey = -1;
//
//  if (ev->key() == Qt::Key_Left)
//	enMovingKey = 0;
//  else if (ev->key() == Qt::Key_Right)
//	enMovingKey = 1;
//  else if (ev->key() == Qt::Key_Up)
//	enMovingKey = 2;
//  else if (ev->key() == Qt::Key_Down)
//	enMovingKey = 3;
//  else if (ev->key() == Qt::Key_PageUp)
//	enMovingKey = 4;
//  else if (ev->key() == Qt::Key_PageUp)
//	enMovingKey = 5;
//
//  cout << enMovingKey << endl;
//
//  emit ArrowPressed(enMovingKey);
//}