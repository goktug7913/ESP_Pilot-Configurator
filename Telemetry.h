#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <array>
#include "Config.h"
#include <QDataStream>
#pragma once

struct telemetry_frame {
  int header = TMTY_HEADER;

  std::array<float, 3> gyro;
  std::array<float, 3> accel;
  std::array<float, 3> rx_scaled;
  std::array<uint16_t, pwm_ch_amt> rx_raw;

  float pid_p, pid_r, pid_y;
  int esc1_out, esc2_out, esc3_out, esc4_out;

};

#endif // TELEMETRY_H
