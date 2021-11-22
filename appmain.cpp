#include "appmain.h"
#include "ui_appmain.h"

appmain::appmain(QWidget *parent) : QMainWindow(parent), ui(new Ui::appmain){

    ui->setupUi(this); // Qt Code
    serialthr = new sThread(this); // Create serial handler thread
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // GUI Signal Connections
    connect(ui->portbutton, &QPushButton::released, this, &appmain::on_port_button_clicked);
    connect(ui->tuning_readbtn, &QPushButton::released, this, &appmain::readConfig);
    connect(ui->tuning_writebtn, &QPushButton::released, this, &appmain::writeConfig);
    connect(ui->reboot_btn, &QPushButton::released, this, &appmain::on_reboot_btn_released);
    connect(ui->tmty_start, &QPushButton::released, this, &appmain::startTelemetry);
    connect(ui->tmty_stop, &QPushButton::released, this, &appmain::stopTelemetry);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Serial Port Signal Connections
    connect(&serialthr->activeport, &QSerialPort::readyRead, serialthr, &sThread::handledata);     //Connect Serial Port to Handler Thread
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Worker Thread Signal Connections
    connect(serialthr, &sThread::cfgDataReady, this, &appmain::oncfgDataReady);         //Config Struct Ready
    connect(serialthr, &sThread::tmtyDataReady, this, &appmain::ontmtyDataReady);       //Telemetry Frame Ready
    connect(serialthr, &sThread::ConnectionStatus, this, &appmain::onConnectionStatus); //Connection Status Changed

    connect(this, &appmain::Connect, serialthr, &sThread::onConnect);                   //Start Serial Comms
    connect(this, &appmain::Disconnect, serialthr, &sThread::onDisconnect);             //Stop Serial Comms

    connect(this, SIGNAL(WriteCmd(uint8_t)), serialthr, SLOT(onWriteCmd(uint8_t))); //Write Cmd /wo Data
    connect(this, SIGNAL(WriteCmd(uint8_t, uint8_t*)), serialthr, SLOT(onWriteCmd(uint8_t,uint8_t*))); //Write Cmd /w Data
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    QDataStream rxstream(rxdata);
    listports();
    statusBar()->showMessage("Ready");

    serialthr->start();
}

void appmain::ontmtyDataReady(telemetry_frame tmtyframe){
    qDebug() << "Worker thread tmty";
    tmty_frame = tmtyframe;
    updateTmty();
}

void appmain::oncfgDataReady(FC_cfg rxcfg){
    qDebug() << "Worker thread cfg";
    config = rxcfg;
    updateUi();
}

void appmain::onConnectionStatus(cState status){
    qDebug() << "Worker thread Connection status changed:" << status;
    switch (status){

    case con:
        s_connected = 1;
        statusBar()->showMessage("Connected."); // Update Status text
        ui->portbutton->setText("Disconnect");  // Set Connect button to disconnect
    break;

    case disc:
        s_connected = 0;
        ui->portbutton->setText("Connect");
        statusBar()->showMessage("Disconnected.");
    break;

    case hshake:
        statusBar()->showMessage("Handshake Complete!");
    break;
    }
}

appmain::~appmain(){
    delete ui;
}

void appmain::listports(){
    // Detects and iterates through available COM ports on the system, probes information about the port
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        ui->ports->addItem(serialPortInfo.portName());
    }
}

void appmain::on_port_button_clicked(){
    // Connect / disconnect function
    if (!s_connected){
        auto baud = ui->baudselector->currentText(); // ffs...
        emit Connect(ui->ports->currentText(), baud.toUInt());
        emit WriteCmd(CFG_MODE); // Send USB mode request
    } else {
        emit Disconnect();
    }
}

void appmain::readConfig(){
    emit WriteCmd(READ_CFG);
}

void appmain::writeConfig(){
    config.header = CFG_MAGIC;

    config.Kp_pitch = ui->pp_box->value();
    config.Ki_pitch = ui->pi_box->value();
    config.Kd_pitch = ui->pd_box->value();

    config.Kp_roll = ui->rp_box->value();
    config.Ki_roll = ui->ri_box->value();
    config.Kd_roll = ui->rd_box->value();

    config.Kp_yaw = ui->yp_box->value();
    config.Ki_yaw = ui->yi_box->value();
    config.Kd_yaw = ui->yd_box->value();

    config.max_angle = ui->maxangle_box->value();

    config.esc1_pin = ui->esc1_pin->value();
    config.esc2_pin = ui->esc2_pin->value();
    config.esc3_pin = ui->esc3_pin->value();
    config.esc4_pin = ui->esc4_pin->value();

    config.esc_pwm_hz = ui->esc_freq->value();

    config.nrf24_telemetry = ui->nrf_enable->isChecked();
    config.oled_display = ui->oled_enable->isChecked();
    config.radar_altimeter = ui->radar_enable->isChecked();
    config.compass = ui->magnet_enable->isChecked();

    emit WriteCmd(WRITE_CFG, (uint8_t*)&config);
}

void appmain::updateUi(){
    ui->pp_box->setValue(config.Kp_pitch);
    ui->pi_box->setValue(config.Ki_pitch);
    ui->pd_box->setValue(config.Kd_pitch);

    ui->rp_box->setValue(config.Kp_roll);
    ui->ri_box->setValue(config.Ki_roll);

    ui->rd_box->setValue(config.Kd_roll);

    ui->yp_box->setValue(config.Kp_yaw);
    ui->yi_box->setValue(config.Ki_yaw);
    ui->yd_box->setValue(config.Kd_yaw);

    ui->esc1_pin->setValue(config.esc1_pin);
    ui->esc2_pin->setValue(config.esc2_pin);
    ui->esc3_pin->setValue(config.esc3_pin);
    ui->esc4_pin->setValue(config.esc4_pin);
    ui->esc_freq->setValue(config.esc_pwm_hz);

    ui->maxangle_box->setValue(config.max_angle);
}

void appmain::updateTmty(){
    ui->deltat->setText(QString::number(tmty_frame.deltaT));

    ui->gyrox->setText(QString::number(tmty_frame.gyro[0]));
    ui->gyroy->setText(QString::number(tmty_frame.gyro[1]));
    ui->gyroz->setText(QString::number(tmty_frame.gyro[2]));

    ui->accx->setText(QString::number(tmty_frame.accel[0]));
    ui->accy->setText(QString::number(tmty_frame.accel[1]));
    ui->accz->setText(QString::number(tmty_frame.accel[2]));

    ui->t_esc1->setText(QString::number(tmty_frame.esc1_out,10));
    ui->t_esc2->setText(QString::number(tmty_frame.esc2_out,10));
    ui->t_esc3->setText(QString::number(tmty_frame.esc3_out,10));
    ui->t_esc4->setText(QString::number(tmty_frame.esc4_out,10));

    ui->targetx->setText(QString::number(tmty_frame.rx_scaled[0]));
    ui->targety->setText(QString::number(tmty_frame.rx_scaled[1]));
    ui->targetz->setText(QString::number(tmty_frame.rx_scaled[2]));

    ui->rx1->setText(QString::number(tmty_frame.rx_raw[0]));
    ui->rx2->setText(QString::number(tmty_frame.rx_raw[1]));
    ui->rx3->setText(QString::number(tmty_frame.rx_raw[2]));
    ui->rx4->setText(QString::number(tmty_frame.rx_raw[3]));
    ui->rx5->setText(QString::number(tmty_frame.rx_raw[4]));
    ui->rx6->setText(QString::number(tmty_frame.rx_raw[5]));

    ui->pid_p->setText(QString::number(tmty_frame.pid_p));
    ui->pid_r->setText(QString::number(tmty_frame.pid_r));
    ui->pid_y->setText(QString::number(tmty_frame.pid_y));

    ui->p_p->setText(QString::number(tmty_frame.p_p));
    ui->p_i->setText(QString::number(tmty_frame.p_i));
    ui->p_d->setText(QString::number(tmty_frame.p_d));

    ui->r_p->setText(QString::number(tmty_frame.r_p));
    ui->r_i->setText(QString::number(tmty_frame.r_i));
    ui->r_d->setText(QString::number(tmty_frame.r_d));

    ui->y_p->setText(QString::number(tmty_frame.y_p));
    ui->y_i->setText(QString::number(tmty_frame.y_i));
    ui->y_d->setText(QString::number(tmty_frame.y_d));
}

void appmain::on_reboot_btn_released(){emit WriteCmd(RESTART_FC);}

void appmain::startTelemetry(){emit WriteCmd(TELEMETRY_START); s_tmtydataflag = 1;}
void appmain::stopTelemetry(){emit WriteCmd(TELEMETRY_STOP); s_tmtydataflag = 0;}

void appmain::arm(){emit WriteCmd(ARM_TETHERED);}
void appmain::disarm(){emit WriteCmd(DISARM);}

// - - - - - - - - - - - - - - - - - - - - - - - - - -
// SLIDER BINDINGS - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - -
void appmain::on_pp_slider_sliderMoved(int position){
    ui->pp_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->rp_box->setValue((float)position/100);
    }
}

void appmain::on_pp_box_valueChanged(double arg1){
    ui->pp_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->rp_slider->setValue(arg1*100);
    }
}

void appmain::on_pi_slider_sliderMoved(int position){
    ui->pi_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->ri_box->setValue((float)position/100);
    }
}

void appmain::on_pi_box_valueChanged(double arg1){
    ui->pi_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->ri_slider->setValue(arg1*100);
    }
}

void appmain::on_pd_slider_sliderMoved(int position){
    ui->pd_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->rd_box->setValue((float)position/100);
    }
}

void appmain::on_pd_box_valueChanged(double arg1){
    ui->pd_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->rd_slider->setValue(arg1*100);
    }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - -
void appmain::on_rp_slider_sliderMoved(int position){
    ui->rp_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->pp_box->setValue((float)position/100);
    }
}

void appmain::on_rp_box_valueChanged(double arg1){
    ui->rp_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->pp_slider->setValue(arg1*100);
    }
}

void appmain::on_ri_slider_sliderMoved(int position){
    ui->ri_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->pi_box->setValue((float)position/100);
    }
}

void appmain::on_ri_box_valueChanged(double arg1){
    ui->ri_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->pi_slider->setValue(arg1*100);
    }
}

void appmain::on_rd_slider_sliderMoved(int position){
    ui->rd_box->setValue((float)position/100);
    if (ui->PR_link->isChecked()){
        ui->pd_box->setValue((float)position/100);
    }
}

void appmain::on_rd_box_valueChanged(double arg1){
    ui->rd_slider->setValue(arg1*100);
    if (ui->PR_link->isChecked()){
        ui->pd_slider->setValue(arg1*100);
    }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - -
void appmain::on_yp_slider_sliderMoved(int position){
    ui->yp_box->setValue((float)position/100);
}

void appmain::on_yp_box_valueChanged(double arg1){
    ui->yp_slider->setValue(arg1*100);
}

void appmain::on_yi_slider_sliderMoved(int position){
    ui->yi_box->setValue((float)position/100);
}

void appmain::on_yi_box_valueChanged(double arg1){
    ui->yi_slider->setValue(arg1*100);
}

void appmain::on_yd_slider_sliderMoved(int position){
    ui->yd_box->setValue((float)position/100);
}

void appmain::on_yd_box_valueChanged(double arg1){
    ui->yd_slider->setValue(arg1*100);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - -
