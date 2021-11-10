#ifndef TMTY_THREAD_H
#define TMTY_THREAD_H

#include <QtCore>

class tmty_thread : public QThread{
    Q_OBJECT
public:

    explicit tmty_thread(QObject *parent = 0);

    void run();

signals:
    void dataReady(int);

public slots:

private:
    int i = 0;
};

#endif // TMTY_THREAD_H
