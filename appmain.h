#ifndef APPMAIN_H
#define APPMAIN_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDataStream>

#include <Definitions.h>
#include <Config.h>
#include <Telemetry.h>
#include <Message.h>
#include <tmty_thread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class appmain; }
QT_END_NAMESPACE

class appmain : public QMainWindow{
    Q_OBJECT

public:

    appmain(QWidget *parent = nullptr);
    ~appmain();

    QSerialPort activeport;
    QByteArray rxdata;
    QByteArray msgdata;
    QDataStream rxstream;

    FC_cfg config;
    telemetry_frame tmty_frame;

    enum dState {done, reading, seek};
    dState state = seek;

    msg_begin header;
    msg_end footer;

    tmty_thread *tmty_thr = nullptr;

    void listports();
    void writeCmd(uint8_t cmd);

    void readConfig();
    void writeConfig();

    void updateUi();
    void updateTmty();

    void startTelemetry();
    void stopTelemetry();

    void arm();
    void disarm();

private:

    Ui::appmain *ui;
    bool s_connected = 0;
    bool s_cfgdataflag = 0;
    bool s_tmtydataflag = 0;

private slots:

    void on_port_button_clicked();
    void handledata();
    void on_reboot_btn_released();

    void on_pp_slider_sliderMoved(int position);
    void on_pp_box_valueChanged(double arg1);
    void on_pi_slider_sliderMoved(int position);
    void on_pi_box_valueChanged(double arg1);
    void on_pd_slider_sliderMoved(int position);
    void on_pd_box_valueChanged(double arg1);

    void on_rp_slider_sliderMoved(int position);
    void on_rp_box_valueChanged(double arg1);
    void on_ri_slider_sliderMoved(int position);
    void on_ri_box_valueChanged(double arg1);
    void on_rd_slider_sliderMoved(int position);
    void on_rd_box_valueChanged(double arg1);

    void on_yp_slider_sliderMoved(int position);
    void on_yp_box_valueChanged(double arg1);
    void on_yi_slider_sliderMoved(int position);
    void on_yi_box_valueChanged(double arg1);
    void on_yd_slider_sliderMoved(int position);
    void on_yd_box_valueChanged(double arg1);

    void onTmtyDataReady(int);
};

#endif // APPMAIN_H
