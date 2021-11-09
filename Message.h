#ifndef MESSAGE_H
#define MESSAGE_H

#include <Definitions.h>
#include <stdint.h>
#include <qbytearray.h>
#include <QIODevice>
#pragma once

struct msg_begin{
  uint8_t  start = MSG_START;               //Message Start Flag
  uint32_t length = 0;                      //Message Length
  uint8_t  cmd = 0;                         //Command
  uint16_t opt = 0;                         //Options
  uint8_t  data_start = DATA_START;         //Data Start Flag

  QByteArray serialize(){
      QByteArray byteArray;
      QDataStream stream(&byteArray, QIODevice::WriteOnly);
      stream.setVersion(QDataStream::Qt_6_2);
      stream << start
             << length
             << cmd
             << opt
             << data_start;
      return byteArray;
  }

  QDataStream* deserialize(QDataStream* dstream){
      *dstream >> start
              >> length
              >> cmd
              >> opt
              >> data_start;
      return dstream;
  }
};

struct msg_end{
  uint8_t  data_end = DATA_END;         //Data End Flag
  uint8_t  end = MSG_END;               //Message End Flag

  QByteArray serialize(){
      QByteArray byteArray;
      QDataStream stream(&byteArray, QIODevice::WriteOnly);
      stream.setVersion(QDataStream::Qt_6_2);
      stream << data_end
             << end;

      return byteArray;
  }

  msg_end* deserialize(QByteArray byteArray){
      QDataStream stream(&byteArray, QIODevice::ReadOnly);
      stream.setVersion(QDataStream::Qt_6_2);
      stream >> data_end
             >> end;
      return this;
  }
};

#endif // MESSAGE_H
