#include "serialthread.h"
#include "appmain.h"

sThread::sThread(QObject *parent) : QThread(parent){
// Constructor
}

void sThread::run(){

}

void sThread::onConnect(QString name, int baud){
    activeport.setPortName(name);
    activeport.setBaudRate(baud);

    if(activeport.open(QIODevice::ReadWrite)){
        portstate = con;
        emit ConnectionStatus(portstate); // Ok
        qDebug() << "Connected";
    } else {
        portstate = disc;
        emit ConnectionStatus(portstate); // Failed
        qDebug() << "Disconnected";
    }
}

void sThread::onDisconnect(){
    activeport.close();
    portstate = disc;
    emit ConnectionStatus(portstate);
}

void sThread::handledata(){
    rxdata += activeport.readAll();

    //ui->textBrowser->clear();

    if ((uint8_t)rxdata[rxdata.size()-1] == (uint8_t)MSG_END){
        memcpy(&header, rxdata, sizeof(header));
        memcpy(&footer, (uint8_t*)&rxdata+sizeof(rxdata)-2, sizeof(footer));
        state = done;

        qDebug() << rxdata.toHex(' ');
        //ui->textBrowser->insertPlainText(rxdata.toHex(' '));
    }

    if (state == done){
        switch (header.cmd){
            case HANDSHAKE:
            portstate = hshake;
            emit ConnectionStatus(portstate);
            qDebug() << "Handshake reply";
            break;
            // - - - - - - - - - - - - -
            case W_EEPROM_OK:
                //statusBar()->showMessage("EEPROM Flashed!");
            break;
            // - - - - - - - - - - - - -
            case W_EEPROM_ERR:
                //statusBar()->showMessage("EEPROM Flash Error!");
            break;
            // - - - - - - - - - - - - -
            case CFG_DATA_FLAG:
                memcpy(&config, (uint8_t*)&rxdata[16], sizeof(config));
                emit cfgDataReady(config);
            break;
            // - - - - - - - - - - - - -
            case TMTY_DATA_FLAG:
                memcpy(&tmty_frame, (uint8_t*)&rxdata[16], sizeof(tmty_frame));
                emit tmtyDataReady(tmty_frame);
            break;
            // - - - - - - - - - - - - -
            default:
            break;
        }
    }
    //Clear buffer
    if (state == done){
        rxdata.clear();
        state = seek;
    }
}

void sThread::onWriteCmd(uint8_t cmd){
    QByteArray byte;
    QByteArray temp;
    msg_begin header;
    msg_end footer;

    QDataStream stream(&byte, QIODevice::WriteOnly);
    QDataStream stream2(&temp, QIODevice::WriteOnly);

    header.cmd = cmd;
    header.length = sizeof(header)+sizeof(footer);

    stream.writeRawData((const char*)&header, sizeof(header));
    stream2.writeRawData((const char*)&footer, sizeof(footer));

    byte.append(temp);

    qDebug() << "Bytes Sent:" << activeport.write(byte, byte.length());
    qDebug() << "Byte array in hex: " << byte.toHex(' ');
}
