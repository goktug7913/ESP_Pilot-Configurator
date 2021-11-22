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
    data_seek();
}

void sThread::onWriteCmd(uint8_t cmd){
    onWriteCmd(cmd, nullptr);
}

void sThread::onWriteCmd(uint8_t cmd, uint8_t* dataptr){
    QByteArray b_msg, b_data, b_footer;
    msg_begin header;
    msg_end footer;

    QDataStream stream(&b_msg, QIODevice::WriteOnly);
    QDataStream stream2(&b_data, QIODevice::WriteOnly);
    QDataStream stream3(&b_footer, QIODevice::WriteOnly);

    header.start = MSG_START;
    header.data_start = DATA_START;
    header.cmd = cmd;
    header.length = sizeof(header)+sizeof(footer);
    footer.data_end = DATA_END;
    footer.end = MSG_END;

    //since sizeof is compile time, we need to figure out data type at runtime
    uint16_t datasize = 0;
    if (cmd == WRITE_CFG){datasize = sizeof(FC_cfg); header.length += datasize;}

    stream.writeRawData((const char*)&header, sizeof(header));
    stream2.writeRawData((const char*)&dataptr, datasize);
    stream3.writeRawData((const char*)&footer, sizeof(footer));

    if(dataptr != nullptr){b_msg.append(b_data);} //Do not append data if there's none
    b_msg.append(b_footer);

    qDebug() << "Bytes Sent:" << activeport.write(b_msg, b_msg.length());
    qDebug() << "Byte array in hex: " << b_msg.toHex(' ');
}

void sThread::data_seek(){
    rxdata += activeport.readAll();

    //Find out the beginning and end of the message
    if (rxdata.contains(MSG_START) && rxdata.contains(MSG_END) && rxdata.contains(DATA_START) && rxdata.contains(DATA_END)){
        qDebug() << "Bytes recv:" << rxdata.size();
        qDebug() << "Rx Bytes:" << rxdata.toHex(' ');

        startindex = rxdata.indexOf(MSG_START);
        dataindex = rxdata.indexOf(DATA_START)+4; //Add 4 to skip the marker byte
        footerindex = rxdata.indexOf(DATA_END)+4; //Add 4 to skip the marker byte
        endindex = rxdata.indexOf(MSG_END);
        state = reading;
        data_read(); // a lot of nesting happens after this point, probably should fix it
    }
}

void sThread::data_read(){
    if (state != reading){return;}

    memcpy(&header, &rxdata[startindex], sizeof(header));
    memcpy(&footer, &rxdata[startindex], sizeof(footerindex));

    if(1+endindex-startindex != header.length){
        // This means data was corrupted on the way, we should drop the packet
        qDebug() << "Length in buffer:" << endindex - startindex;
        qDebug() << "Length in header:" << header.length << "\n" << "Dropping packet!";
        rxdata.clear();
        state = seek;
        return;
    }

    if (header.cmd == CFG_DATA_FLAG){memcpy(&config, &rxdata[dataindex], sizeof(FC_cfg));}
    else if (header.cmd == TMTY_DATA_FLAG){memcpy(&tmty_frame, &rxdata[dataindex], sizeof(telemetry_frame));}

    rxdata.clear();
    state = done;
    data_done();
}

void sThread::data_done(){
    if (state != done){return;}

    switch (header.cmd){
        case HANDSHAKE:
        portstate = hshake;
        emit ConnectionStatus(portstate);
        qDebug() << "Handshake reply";
        break;
        // - - - - - - - - - - - - -
        case SERIALPOLL:
        portstate = hshake;
        emit ConnectionStatus(portstate);
        qDebug() << "ESP32 polled serial, handshake ok";
        break;
        // - - - - - - - - - - - - -
        case W_EEPROM_OK:
            qDebug() << "EEPROM Flashed!";
        break;
        // - - - - - - - - - - - - -
        case W_EEPROM_ERR:
            qDebug() << "EEPROM Flash Error!";
        break;
        // - - - - - - - - - - - - -
        case CFG_DATA_FLAG:
            emit cfgDataReady(config);
        break;
        // - - - - - - - - - - - - -
        case TMTY_DATA_FLAG:
            emit tmtyDataReady(tmty_frame);
        break;
        // - - - - - - - - - - - - -
        default:
            qDebug() << "Unknown command: " << header.cmd;
        break;
    }
    state = seek;
}
