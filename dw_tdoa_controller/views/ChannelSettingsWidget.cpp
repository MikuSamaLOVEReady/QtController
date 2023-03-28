// -------------------------------------------------------------------------------------------------------------------
//
//  File: ChannelSettingsWidget.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "ChannelSettingsWidget.h"
#include "ui_ChannelSettingsWidget.h"

#include "RTLSControllerApplication.h"
#include "RTLSControl.h"
#include "mainwindow.h"
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDesktopWidget>

ChannelSettingsWidget::ChannelSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChannelSettingsWidget),
    //set default as invalid selection - will be correctly populated by defaults below
    _currentprf(-1),
    _currentchannel(-1),
    _currentpac(-1),
    _currentdatarate(-1),
    _currentplen(-1),
    _currentpcode(-1),
    _currentnsfd(-1), //set automatically when the rate is set
    _currentantennadly(16450), //~514ns/15.65ps/2
    _currentsfdto(4161),
    _currentsmartpow(-1), //set automatically when the rate is set
    _currentphrmode(0)
{
    ui->setupUi(this);

    _mapper = new QDataWidgetMapper(this);
    _model = new QStandardItemModel(1, ColumnCount, this);


    _preambleLengths << DWT_PLEN_64 << DWT_PLEN_128 << DWT_PLEN_256
                     << DWT_PLEN_512 << DWT_PLEN_1024 << DWT_PLEN_1536
                     << DWT_PLEN_2048 << DWT_PLEN_4096 ;


    //center on the screen.
    QDesktopWidget desk;
    QRect screenres = desk.screenGeometry(0); //obsolete
    this->setGeometry(QRect(screenres.width()/2,screenres.height()/2,screenres.width()/2,screenres.height()/2));

    _ignore = false;

    RTLSControllerApplication::connectReady(this, "onReady()");
}

ChannelSettingsWidget::~ChannelSettingsWidget()
{

    delete _model;
    delete _mapper;

    delete ui;
}


void ChannelSettingsWidget::onReady()
{
    //QObject::connect(ui->ChannelSettingsWidget, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(anchorSelected(QModelIndex,QModelIndex)));

    //set the defaults
    _chConfig.channel = 2;
    _chConfig.datarate = DWT_BR_6M8 ;
    _chConfig.nsSFD = 0;
    _chConfig.preambleLength = DWT_PLEN_128 ;
    _chConfig.preambleCode = 9 ;
    _chConfig.prf = DWT_PRF_64M ;
    _chConfig.pacSize = DWT_PAC8 ;


    _mapper->setModel(_model);

    _mapper->addMapping(ui->channel, ColumnChan, "currentIndex");
    _mapper->addMapping(ui->pCode, ColumnPCode);
    _mapper->addMapping(ui->nsSFD, ColumnSFD);

    //_mapper->addMapping(_currentprf, ColumnPRF);
    QObject::connect(ui->channel, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannel(int)));
    QObject::connect(ui->pCode, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePreambleCode(int)));
    QObject::connect(ui->nsSFD, SIGNAL(clicked()), _mapper, SLOT(submit())); // Bug with QDataWidgetMapper (QTBUG-1818)
    QObject::connect(ui->nsSFD, SIGNAL(clicked()), this, SLOT(updateNSSFD()));

    QObject::connect(ui->prf16, SIGNAL(clicked()), this, SLOT(updatePRF()));
    QObject::connect(ui->prf64, SIGNAL(clicked()), this, SLOT(updatePRF()));

    QObject::connect(ui->l64, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l128, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l256, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l512, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l1024, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l1536, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l2048, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));
    QObject::connect(ui->l4096, SIGNAL(clicked()), this, SLOT(updatePreambleLength()));

    QObject::connect(ui->dr110, SIGNAL(clicked()), this, SLOT(updateDataRate()));
    QObject::connect(ui->dr850, SIGNAL(clicked()), this, SLOT(updateDataRate()));
    QObject::connect(ui->dr6M8, SIGNAL(clicked()), this, SLOT(updateDataRate()));

    //set up defaults
    resetToDefaults();
    ui->nsSFD->setVisible(true);

    QObject::connect(ui->ok, SIGNAL(clicked()), RTLSControllerApplication::control(), SLOT(sendRFConfigurationClick()));
    QObject::connect(RTLSControllerApplication::control(), SIGNAL(rfCfgApplied()), this, SLOT(rfCfgDone()));
    QObject::connect(ui->usecase, SIGNAL(currentIndexChanged(int)), this, SLOT(setUseCase(int)));


    QObject::connect(RTLSControllerApplication::mainWindow(), SIGNAL(setTagRate(int, int, int)), this, SLOT(setTagRate(int, int, int)));
    QObject::connect(RTLSControllerApplication::mainWindow(), SIGNAL(setTagID(QString)), this, SLOT(setTagID(QString)));
    QObject::connect(RTLSControllerApplication::mainWindow(), SIGNAL(serialError(void)), this, SLOT(serialError(void)));

    QObject::connect(RTLSControllerApplication::cleConnection(), SIGNAL(connectionStateChanged(RTLSCLEConnection::ConnectionState)),
                     this, SLOT(connectionStateChanged(RTLSCLEConnection::ConnectionState)));

    ui->ok->setEnabled(false); //wait for connection before enabling configuration

}


void ChannelSettingsWidget::setUseCase(int usecase)
{

    switch(usecase)
    {

        case Defaults:
        {
            resetToDefaults();
        }
        break;

        case MaxDensity: //Ch 2, PRF 64, 6.8Mbps, PL 64, SFD-Standard8, special 64 length gearing table, packet length < 32 bytes including CRC.
        //Note that this short a preamble is not ideal for seeing highly attenuated first paths and that the Xtal offset that can be handled is limited to +/-15ppm.
        {
            //set up defaults
            setCurrentChannel(2);
            setCurrentPRF(DWT_PRF_64M);
            //PAC is set via the preamble length selection...
            setCurrentPreambleLength(DWT_PLEN_64);
            setCurrentDataRate(DWT_BR_6M8);
            setCurrentNSFD(0);
            ui->nsSFD->setChecked(0);
            setCurrentPreambleCode(9);
            configurePreambleCodes();
        }
        break;

        case MaxRange: //Ch. 2, PRF 64, 110kbps, PL 1536, SFD-NonStandard64.
        {
            //set up defaults
            setCurrentChannel(2);
            setCurrentPRF(DWT_PRF_64M);
            //PAC is set via the preamble length selection...
            setCurrentPreambleLength(DWT_PLEN_1536);
            setCurrentDataRate(DWT_BR_110K);
            setCurrentNSFD(1);
            ui->nsSFD->setChecked(1);
            setCurrentPreambleCode(9);
            configurePreambleCodes();
        }
        break;

        case UserDefined: //what ever last configuration was
        default:
        {

        }
        break;
    }
}

void ChannelSettingsWidget::resetToDefaults(void)
{
    //set up defaults
    setCurrentChannel(_chConfig.channel);
    setCurrentPRF(_chConfig.prf);
    //PAC is set via the preamble length selection...
    setCurrentPreambleLength(_chConfig.preambleLength);
    setCurrentDataRate(_chConfig.datarate);
    setCurrentNSFD(_chConfig.nsSFD);
    ui->nsSFD->setChecked(_chConfig.nsSFD);
    setCurrentPreambleCode(_chConfig.preambleCode);
    configurePreambleCodes();
}

void ChannelSettingsWidget::rfCfgDone(void)
{
    QMessageBox::StandardButton reply;

    //QMessageBox::about(this, tr("Tag Configuration"), "New configuration has been applied.");
    reply = QMessageBox::question(this, tr("Anchor Configuration"), "New configuration has been applied. Done?",
                                  QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        //close the window
        //this->hide();
        emit closeConfigWidget();
    }
}

void ChannelSettingsWidget::connectionStateChanged(RTLSCLEConnection::ConnectionState state)
{
    if(state == RTLSCLEConnection::Connected)
        ui->ok->setEnabled(true);
    else
        ui->ok->setEnabled(false);
}

int ChannelSettingsWidget::getCurrentSmartPow(void)
{
    return _currentsmartpow ;
}

int ChannelSettingsWidget::getCurrentSFDTO(void)
{
    return _currentsfdto ;
}

int ChannelSettingsWidget::getCurrentPHRMode(void)
{
    return _currentphrmode ;
}


void ChannelSettingsWidget::updatePRF()
{
    if(ui->prf16->isChecked())
        _currentprf = DWT_PRF_16M;

    if(ui->prf64->isChecked())
        _currentprf = DWT_PRF_64M;

    configurePreambleCodes();
}

int ChannelSettingsWidget::getCurrentAntennaDly(void)
{
    return _currentantennadly ;
}

int ChannelSettingsWidget::getCurrentNSFD(void)
{
    return _currentnsfd ;
}

void ChannelSettingsWidget::setCurrentNSFD(int nsfd)
{
    _currentnsfd = nsfd ;
    ui->nsSFD->setChecked((nsfd==1)?true:false);
}

int ChannelSettingsWidget::getCurrentPRF(void)
{
    return _currentprf ;
}

int ChannelSettingsWidget::getCurrentPRFN(void)
{
    return ((_currentprf == DWT_PRF_64M) ? 64 : 16);
}

void ChannelSettingsWidget::setCurrentPRF(int prf)
{
    if(prf == DWT_PRF_16M)
    {
        ui->prf16->setChecked(true);
        _currentprf = DWT_PRF_16M;
    }
    else
    {
        ui->prf64->setChecked(true);
        _currentprf = DWT_PRF_64M;
    }
}

void ChannelSettingsWidget::updateDataRate()
{
    if(ui->dr110->isChecked())
    {
        _currentdatarate = DWT_BR_110K;
        setCurrentNSFD(1);
    }
    if(ui->dr850->isChecked())
    {
        _currentdatarate = DWT_BR_850K;
        setCurrentNSFD(1);
    }
    if(ui->dr6M8->isChecked())
    {
        _currentdatarate = DWT_BR_6M8;
        setCurrentNSFD(0);
    }
}

int ChannelSettingsWidget::getCurrentDataRate(void)
{
    return _currentdatarate ;
}

int ChannelSettingsWidget::getCurrentDataRateN(void)
{
    return ((_currentdatarate == DWT_BR_110K) ? 110 : (_currentdatarate == DWT_BR_850K) ? 850 : 6810);
}

void ChannelSettingsWidget::setCurrentDataRate(int datarate)
{
    if(datarate == DWT_BR_110K)
    {
        ui->dr110->setChecked(true);
        _currentdatarate = DWT_BR_110K;
        _currentsmartpow = 0;
    }
    else if(datarate == DWT_BR_850K)
    {
        ui->dr850->setChecked(true);
        _currentdatarate = DWT_BR_850K;
        _currentsmartpow = 0;
    }
    else
    {
        ui->dr6M8->setChecked(true);
        _currentdatarate = DWT_BR_6M8;
        _currentsmartpow = 1;
    }
}

int ChannelSettingsWidget::getCurrentPreambleLengthN(void)
{
    int len = 0;

    switch(_currentplen)
    {
        case DWT_PLEN_64: len = 64; break;
        case DWT_PLEN_128: len = 128; break;
        case DWT_PLEN_256: len = 256; break;
        case DWT_PLEN_512: len = 512; break;
        case DWT_PLEN_1024: len = 1024; break;
        case DWT_PLEN_1536: len = 1536; break;
        case DWT_PLEN_2048: len = 2048; break;
        case DWT_PLEN_4096: len = 4096; break;
    }


    return len;
}

int ChannelSettingsWidget::getCurrentPACN(void)
{
    int len = 0;

    switch(_currentpac)
    {
        case DWT_PAC8: len = 8; break;
        case DWT_PAC16: len = 16; break;
        case DWT_PAC32: len = 32; break;
        case DWT_PAC64: len = 64; break;
    }


    return len;
}

void ChannelSettingsWidget::updateNSSFD()
{
    _currentnsfd = ui->nsSFD->isChecked();
}

int ChannelSettingsWidget::getCurrentPreambleCodeN(void)
{
    return _currentpcode ;
}

void ChannelSettingsWidget::updatePreambleLength()
{
    if(ui->l64->isChecked())
    {
        _currentplen = DWT_PLEN_64;
        _currentpac = DWT_PAC8;
    }
    if(ui->l128->isChecked())
    {
        _currentplen = DWT_PLEN_128;
        _currentpac = DWT_PAC8;
    }
    if(ui->l256->isChecked())
    {
        _currentplen = DWT_PLEN_256;
        _currentpac = DWT_PAC16;
    }
    if(ui->l512->isChecked())
    {
        _currentplen = DWT_PLEN_512;
        _currentpac = DWT_PAC16;
    }
    if(ui->l1024->isChecked())
    {
        _currentplen = DWT_PLEN_1024;
        _currentpac = DWT_PAC32;
    }
    if(ui->l1536->isChecked())
    {
        _currentplen = DWT_PLEN_1536;
        _currentpac = DWT_PAC64;
    }
    if(ui->l2048->isChecked())
    {
        _currentplen = DWT_PLEN_2048;
        _currentpac = DWT_PAC64;
    }
    if(ui->l4096->isChecked())
    {
        _currentplen = DWT_PLEN_4096;
        _currentpac = DWT_PAC64;
    }
}

int ChannelSettingsWidget::getCurrentPAC(void)
{
    return _currentpac ;
}

void ChannelSettingsWidget::setCurrentPAC(int pac)
{
    _currentpac = pac;
}

int ChannelSettingsWidget::getCurrentPreambleLength(void)
{
    return _currentplen ;
}

void ChannelSettingsWidget::setCurrentPreambleLength(int plen)
{
    if(plen == DWT_PLEN_64)
    {
        ui->l64->setChecked(true);
        _currentplen = DWT_PLEN_64;
        _currentpac = DWT_PAC8;
    }
    else if(plen == DWT_PLEN_128)
    {
        ui->l128->setChecked(true);
        _currentplen = DWT_PLEN_128;
        _currentpac = DWT_PAC8;
    }
    else if(plen == DWT_PLEN_256)
    {
        ui->l256->setChecked(true);
        _currentplen = DWT_PLEN_256;
        _currentpac = DWT_PAC16;
    }
    else if(plen == DWT_PLEN_512)
    {
        ui->l512->setChecked(true);
        _currentplen = DWT_PLEN_512;
        _currentpac = DWT_PAC16;
    }
    else if(plen == DWT_PLEN_1024)
    {
        ui->l1024->setChecked(true);
        _currentplen = DWT_PLEN_1024;
        _currentpac = DWT_PAC32;
    }
    else if(plen == DWT_PLEN_1536)
    {
        ui->l1536->setChecked(true);
        _currentplen = DWT_PLEN_1536;
        _currentpac = DWT_PAC64;
    }
    else if(plen == DWT_PLEN_2048)
    {
        ui->l2048->setChecked(true);
        _currentplen = DWT_PLEN_2048;
        _currentpac = DWT_PAC64;
    }
    else if(plen == DWT_PLEN_4096)
    {
        ui->l4096->setChecked(true);
        _currentplen = DWT_PLEN_4096;
        _currentpac = DWT_PAC64;
    }
    else
    {
        _currentplen = -1;
    }
}

int ChannelSettingsWidget::getCurrentChannel(void)
{
    return _currentchannel ;
}

void ChannelSettingsWidget::setCurrentChannel(int chan)
{
    if((chan != 1) && (chan != 3))
    {
        if(chan == 7)
            ui->channel->setCurrentIndex(5);
        else
            ui->channel->setCurrentIndex(chan-1);
    }
}

void ChannelSettingsWidget::updateChannel(int index)
{
    if((index != 0) && (index != 2))
    {
        if(index == 5)
            _currentchannel = 7;
        else
            _currentchannel = index + 1;

        configurePreambleCodes();

        _ignore = false;
    }
    else
    {
        if(_ignore == false)
        {
            _ignore = true;
            setCurrentChannel(_currentchannel);
        }
    }
}

void ChannelSettingsWidget::updatePreambleCode(int index)
{
     if((pcodes_int.size() > index) && (index >= 0))
         _currentpcode = pcodes_int.at(index);
}

void ChannelSettingsWidget::configurePreambleCodes(void)
{
    pcodes.clear();
    pcodes_int.clear();

    if (_currentprf == DWT_PRF_64M)           // see IEEE Std 802.15.4a-2007, Table 39e
    {
        // use 127 bit codes
        switch(_currentchannel)
        {
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 8:
        case 9:
        case 10:
            // Codes allowed are 9, 10, 11, 12
            pcodes << " 9" << "10" << "11" << "12";
            pcodes_int << 9 << 10 << 11 << 12;
            break;
        case 4:
        case 7:
        case 11:
            // codes allowed are 17, 18, 19 and 20
            pcodes << "17" << "18" << "19" << "20"
            // Also for compatabibility with narrow band implementations can select 1,2,3,4,5, and 6.
             << " 9 - for interworking" << "10 - for interworking" << "11 - for interworking" << "12 - for interworking";
            pcodes_int << 17 << 18 << 19 << 20 << 9 << 10 << 11 << 12;
            break ;
        default:
            return ;
        } // end switch
    }
    else   // see IEEE Std 802.15.4a-2007, Table 39d
    {
        // use 31 bit codes
        switch(_currentchannel)
        {
        case 1:
        case 8:
            // Codes allowed are 1,2
            pcodes << " 1 " <<  " 2 ";
            pcodes_int << 1 << 2;
            break;
        case 2:
        case 5:
        case 9:
            // codes allowed are 3,4
            pcodes << " 3 " <<  " 4 ";
            pcodes_int << 3 << 4;
            break ;
        case 3:
        case 6:
        case 10:
            // codes allowed are 5,6
            pcodes << " 5 " <<  " 6 ";
            pcodes_int << 5 << 6;
            break ;
        case 4:
        case 7:
        case 11:
            // codes allowed are 7,8
            pcodes << " 7 " <<  " 8 "
            // Also for compatabibility with narrow band implementations can select 1,2,3,4,5, and 6.
             <<  " 1 - for ch 1,8 interworking" <<  " 2 - for ch 1,8 interworking"
             <<  " 3 - for ch 2,5,9 interworking" <<  " 4 - for ch 2,5,9 interworking"
             <<  " 5 - for ch 4,7,11 interworking" <<  " 6 - for ch 4,7,11 interworking";
            pcodes_int << 7 << 8 << 1 << 2 << 3 << 4 << 5 << 6;
            break ;
        default:
            return ;
        } // end switch
    } // end else 31 bit codes

    //remove all items from the combobox
    while(ui->pCode->count())
    {
        ui->pCode->removeItem(0);
    }

    //add new items to the combobox
    ui->pCode->addItems(pcodes);

    for (int i = 0; i < pcodes_int.size(); ++i) {
        if (pcodes_int.at(i) == _currentpcode)
            ui->pCode->setCurrentIndex(i);
    }
}

int ChannelSettingsWidget::getCurrentPreambleCode(void)
{
    return _currentpcode ;
}

void ChannelSettingsWidget::setCurrentPreambleCode(int pcode)
{
    configurePreambleCodes();
    _currentpcode = pcode ;

    for (int i = 0; i < pcodes_int.size(); ++i) {
        if (pcodes_int.at(i) == _currentpcode)
            ui->pCode->setCurrentIndex(i);
    }
}
