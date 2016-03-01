#ifndef QYKLABEL_H
#define QYKLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QRect>
#include <vector>
//#include <QDebug>
class YK16GrayImage;

using namespace std;
class qyklabel : public QLabel
{
	Q_OBJECT

public:
	YK16GrayImage* m_pYK16Image;
	QRect m_Rt;
	std::vector<QPoint> m_vPt;
	bool m_bDrawPoints;
	int m_iMouseWheelDelta;	

	bool m_bFocusIn;

public:
	qyklabel(QWidget *parent);
	~qyklabel();

	bool isFocusIn() {return m_bFocusIn;}

	// virtual function reimplementation	
	void mousePressEvent(QMouseEvent *ev); //ev->buttons() == Qt::LeftButton
	void mouseMoveEvent(QMouseEvent *ev); 	
	void mouseReleaseEvent(QMouseEvent *ev);
	void wheelEvent(QWheelEvent * event); //this->setText("Delta Value: "+QString::number(event->delta()));

        void mouseDoubleClickEvent(QMouseEvent *ev);

	//void keyPressEvent ( QKeyEvent *ev);
	//void focusInEvent ( QFocusEvent * ev );
	//void focusOutEvent ( QFocusEvent * ev );

	void enterEvent (QEvent *);
	void leaveEvent(QEvent *);
	int x,y;

	void SetBaseImage(YK16GrayImage* pYKImage);
	//void ConvertAndCopyPoints(vector<QPoint>& vSrcPoint);
	void ConvertAndCopyPoints(vector<QPoint>& vSrcPoint, int iDataWidth, int iDataHeight);

	QPoint View2Data(QPoint viewPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight);
	QPoint View2DataExt(QPoint viewPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight, QPoint ptDataOffset, double fUserZoom);
	QPoint Data2View(QPoint dataPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight);
	QPoint Data2ViewExt(QPoint dataPt, int viewWidth, int viewHeight, int dataWidth, int dataHeight, QPoint ptDataOffset, double fUserZoom);

	QPoint GetDataPtFromMousePos(); //Return data position of the mouse position.m_pYK16 image is mandatory
	QPoint GetDataPtFromViewPt(int viewPtX, int viewPtY);

	QPoint GetViewPtFromDataPt(int dataPtX, int dataPtY);


protected:
	void paintEvent(QPaintEvent *);

signals:	
	void Mouse_Pressed_Left();
	void Mouse_Pressed_Right();
	void Mouse_Move();	

	void Mouse_Released_Left();
	void Mouse_Released_Right();
	void Mouse_Wheel();

        void Mouse_Left_DoubleClick();
        void Mouse_Right_DoubleClick();

	void FocusIn();
	void FocusOut();

	//void ArrowPressed(int arrowDirection);// 0: Left, 1: Upward, 2: Right, 3: Downward
	//void OutFromWindow();


public slots:
	void SetDrawPointToggle(bool bToggle);

private:
	
};

#endif // QYKLABEL_H
