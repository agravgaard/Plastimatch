#pragma once
#include <QDialog>
#include "ui_DlgGammaView.h"
class gamma_gui;

class DlgGammaView : public QDialog,
    public Ui::DlgGammaViewClass
{
    Q_OBJECT
    ;

public slots:   


public:
    DlgGammaView();    
    DlgGammaView(QWidget *parent);
    ~DlgGammaView(); 







public: 
    gamma_gui* m_pParent; //to pull 3D images       


private:
    Ui::DlgGammaViewClass ui;
	
};
