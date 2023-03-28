#ifndef CHANNELSETTINGSWIDGET_H
#define CHANNELSETTINGSWIDGET_H

#include <QWidget>
#include "rtlscleconnection.h"

//! constants for selecting the bit rate for data TX (and RX)
//! These are defined for write (with just a shift) the TX_FCTRL register
#define DWT_BR_110K		0	//!< UWB bit rate 110 kbits/s
#define DWT_BR_850K		1	//!< UWB bit rate 850 kbits/s
#define DWT_BR_6M8		2	//!< UWB bit rate 6.8 Mbits/s

//! constants for specifying the (Nominal) mean Pulse Repetition Frequency
//! These are defined for direct write (with a shift if necessary) to CHAN_CTRL and TX_FCTRL regs
#define DWT_PRF_16M		1	//!< UWB PRF 16 MHz
#define DWT_PRF_64M		2	//!< UWB PRF 64 MHz

//! constants for specifying Preamble Acquisition Chunk (PAC) Size in symbols
#define DWT_PAC8		0	//!< PAC  8 (recommended for RX of preamble length  128 and below
#define DWT_PAC16		1	//!< PAC 16 (recommended for RX of preamble length  256
#define DWT_PAC32		2	//!< PAC 32 (recommended for RX of preamble length  512
#define DWT_PAC64		3	//!< PAC 64 (recommended for RX of preamble length 1024 and up

//! constants for specifying TX Preamble length in symbols
//! These are defined to allow them be directly written into byte 2 of the TX_FCTRL register
//! (i.e. a four bit value destined for bits 20..18 but shifted left by 2 for byte alignment)
#define DWT_PLEN_4096	0x0C	//! Standard preamble length 4096 symbols
#define DWT_PLEN_2048	0x28	//! Non-standard preamble length 2048 symbols
#define DWT_PLEN_1536	0x18	//! Non-standard preamble length 1536 symbols
#define DWT_PLEN_1024	0x08	//! Standard preamble length 1024 symbols
#define DWT_PLEN_512	0x34	//! Non-standard preamble length 512 symbols
#define DWT_PLEN_256	0x24	//! Non-standard preamble length 256 symbols
#define DWT_PLEN_128	0x14	//! Non-standard preamble length 128 symbols
#define DWT_PLEN_64		0x04	//! Standard preamble length 64 symbols

typedef struct
{
    int channel ;
    int prf ;
    int datarate ;
    int preambleCode ;
    int preambleLength ;
    int pacSize ;
    int nsSFD ;
} chConfig_t ;

namespace Ui {
class ChannelSettingsWidget;
}

class QDataWidgetMapper;
class QAbstractItemModel;
class QStandardItemModel;

class ChannelSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    enum Column {
        ColumnChan = 0, ///< channel number
        ColumnPRF,      ///< prf
        ColumnDataRate, ///< data rate
        ColumnPCode,    ///< preamble code
        ColumnPLength,  ///< preamble length
        ColumnPAC,      ///< PAC
        ColumnSFD,      ///< SFD (if standard = false or non-standard = true)
        ColumnCount
    };

    enum UseCases {
        UserDefined = 0,    ///< user defined
        MaxRange,           ///< max range
        MaxDensity,         ///< max density
        Defaults,           ///< defaults
        Count
    };

    explicit ChannelSettingsWidget(QWidget *parent = nullptr);
    ~ChannelSettingsWidget();

    int getCurrentPRF(void);
    void setCurrentPRF(int prf);
    int getCurrentPreambleLength(void);
    void setCurrentPreambleLength(int plen);
    int getCurrentDataRate(void);
    void setCurrentDataRate(int datarate);
    int getCurrentPAC(void);
    void setCurrentPAC(int pac);
    int getCurrentChannel(void);
    void setCurrentChannel(int chan)  ;
    void configurePreambleCodes(void);
    int getCurrentPreambleCode(void) ;
    void setCurrentPreambleCode(int pcode);
    int getCurrentNSFD(void);
    void setCurrentNSFD(int nsfd);
    int getCurrentAntennaDly(void);

    //use these for getting the integer number to pass to CLE
    int getCurrentPRFN(void);
    int getCurrentDataRateN(void);
    int getCurrentPreambleLengthN(void);
    int getCurrentPreambleCodeN(void) ;
    int getCurrentPACN(void);

    int getCurrentBRand(void);
    int getCurrentBRate(void);
    int getCurrentSmartPow(void);
    int getCurrentSFDTO(void);
    int getCurrentPHRMode(void);
    int getCurrentIMUData(void);

    signals:
    void closeConfigWidget();

public slots:

protected slots:
    void setUseCase(int usecase);
    void resetToDefaults();

    void onReady();
    void updatePRF();
    void updatePreambleLength();
    void updateDataRate();
    void updateChannel(int);
    void updatePreambleCode(int);
    void updateNSSFD();

    void connectionStateChanged(RTLSCLEConnection::ConnectionState state);

    void rfCfgDone(void);
private:
    void setupModel();

    Ui::ChannelSettingsWidget *ui;

    int _currentprf ;
    int _currentchannel ;
    int _currentpac ;
    int _currentdatarate ;
    int _currentplen ;
    int _currentpcode ;
    int _currentnsfd ;
    int _currentantennadly ;
    int _currentsfdto ;
    int _currentsmartpow ;
    int _currentphrmode ;

    chConfig_t _chConfig ;

    chConfig_t _tmpchConfig ;

    QStandardItemModel *_model;
    QDataWidgetMapper *_mapper;

    QList<int> _preambleLengths;

    QList<QString> pcodes ;
    QList<int> pcodes_int ;

    bool _ignore;

};

#endif // CHANNELSETTINGSWIDGET_H
