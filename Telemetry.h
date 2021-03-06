#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <array>
#include "Config.h"
#pragma once

struct telemetry_frame {
  // Telemetry frame header is 16 bytes to reduce overhead
  uint16_t header = TMTY_HEADER;

  std::array<float, 3> gyro;
  std::array<float, 3> accel;
  std::array<float, 3> rx_scaled;

  // We will use a fixed array of 6 elements to send axis data for now
  // TODO: Add support for arbitrary number of axes and digital inputs
  std::array<uint16_t, 6> rx_raw;

  float temperature, baro_alt, radar_alt;
  float deltaT;

  float pid_p, pid_r, pid_y; // PID values
  float p_p, p_i, p_d, r_p, r_i, r_d, y_p, y_i, y_d; // PID terms
  int esc1_out, esc2_out, esc3_out, esc4_out; // ESC output values

};

#endif
