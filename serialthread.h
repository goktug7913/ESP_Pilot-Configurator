#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QtCore>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDataStream>

#include <Definitions.h>
#include <Config.h>
#include <Telemetry.h>
#include <Message.h>

enum dState {done, reading, seek};
enum cState {con, disc, hshake};

class sThread : public QThread{
    Q_OBJECT
public:
    explicit sThread(QObject *parent = 0);
    void run();

    QSerialPort activeport;
    QByteArray rxdata;
    QByteArray msgdata;

    FC_cfg config;
    telemetry_frame tmty_frame;

    dState state = seek;
    cState portstate = disc;
    msg_begin header;
    msg_end footer;

signals:
    void cfgDataReady(FC_cfg);
    void tmtyDataReady(telemetry_frame);

    void ConnectionStatus(cState);

public slots:
    void handledata();
    void onConnect(QString name, int baud);
    void onDisconnect();
    void onWriteCmd(uint8_t cmd, uint8_t *dataptr);
    void onWriteCmd(uint8_t cmd);

private:
    int i = 0;

    void data_seek();
    void data_read();
    void data_done();

    long long startindex, dataindex, footerindex, endindex;
};

#endif // SERIALTHREAD_H
