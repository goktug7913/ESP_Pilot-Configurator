#include "tmty_thread.h"

tmty_thread::tmty_thread(QObject *parent) : QThread(parent){

}

void tmty_thread::run(){
    for (int i = 0; ;i++){
         emit dataReady(i);
         sleep(1);
    }
}
