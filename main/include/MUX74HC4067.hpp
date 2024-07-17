/**
 * Name: MUX74HC4067
 * Author: Nick Lamprianidis { nlamprian@gmail.com }
 * Version: 1.0
 * Description: A library for interfacing the 74HC4067
 *              multiplexers/demultiplexers
 * Source: https://github.com/nlamprian/MUX74HC4067
 * License: Copyright (c) 2014 Nick Lamprianidis
 *          This library is licensed under the MIT license
 *          http://www.opensource.org/licenses/mit-license.php
 *
 * Filename: MUX74HC4067.h
 * File description: Definitions and methods for the MUX74HC4067 library
 */

#pragma once

#include <Arduino.h>

#define ANALOG 0
#define DIGITAL 1

#define DISABLED 0
#define ENABLED 1
#include <cstdint>

/**
 * @brief Interfaces the 74HC4067 multiplexers/demultiplexers.
 */
class MUX74HC4067 {
 public:
  /**
   * @brief Construct a new MUX74HC4067 object
   *
   * @param en arduino pin to which the EN pin connects
   * @param s0 arduino pin to which the S0 pin connects
   * @param s1 (optional) arduino pin to which the S1 pin connects
   * @param s2 (optional) arduino pin to which the S2 pin connects
   * @param s3 (optional) arduino pin to which the S3 pin connects
   */
  MUX74HC4067(uint8_t en, int8_t s0, int8_t s1 = -1, int8_t s2 = -1,
              int8_t s3 = -1);

  /**
   * @brief Selects the given channel, and enables its connection with the SIG
   * pin by default
   *
   * @param pin the channel to select
   * @param set flag (DISABLED or ENABLED) to indicate whether to leave the
   * connection of the channel to the SIG pin disabled or enabled
   */
  void setChannel(int8_t pin, uint8_t set = ENABLED);

  /**
   * @brief Enables the connection of the SIG pin to the channel that was set
   * before.
   */
  void enable();

  /**
   * @brief Disables the connection of the SIG pin from the channel that was set
   * before.
   */
  void disable();

  /**
   * @brief Configures the signal pin.
   * @details If set to input, it reads from the signal pin. If set to output,
   * it writes to the signal pin. If set to digital, it reads or writes a
   * digital value to the signal pin. If set to analog, either it reads an
   * analog value from the signal pin, or it writes a PWM output to the signal
   * pin.
   *
   * @param sig arduino pin to which the SIG pin connects
   * @param mode INPUT, OUTPUT, or INPUT_PULLUP
   * @param type DIGITAL or ANALOG
   */
  void signalPin(uint8_t sig, uint8_t mode, uint8_t type = DIGITAL);

  /**
   * @brief It reads from the pre-configured or the given channel.
   * @details If the signal pin was set to DIGITAL, it returns HIGH or LOW. If
   * the signal pin was set to ANALOG, it returns the value read by the ADC. If
   * the signal pin was not set earlier, it returns -1. There is an optional
   * argument for changing momentarily the channel from which to read. This
   * doesn't change the previous configuration, e.g. if channel 7 was selected
   * and its connection was disabled, after reading the given channel, the
   * configuration will return to channel 7 in disabled status.
   *
   * @param chan_pin (optional) read the given channel instead of the
   * pre-configured one
   * @return int16_t Input value read from the pin
   */
  int16_t read(int8_t chan_pin = -1);

  /**
   * @brief It writes to the requested channel.
   * @details If the signal pin was set to DIGITAL, it writes HIGH or LOW. If
   * the signal pin was set to ANALOG, it writes a PWM output. There is also an
   * option to override the ANALOG/DIGITAL configuration of the signal pin.
   *
   * @param chan_pin channel to which to write
   * @param data data to write to channel chan_pin. If the signal pin was set to
   * digital or DIGITAL was specified as argument, data should be HIGH or LOW.
   * If the signal pin was set to analog or ANALOG was specified as argument,
   * data should the duty cycle of the PWM output
   * @param type (optional) it's ANALOG or DIGITAL and overrides the
   * ANALOG/DIGITAL configuration of the signal pin
   * @return uint8_t -1 if the signal pin has been set as input, 0 otherwise.
   */
  uint8_t write(int8_t chan_pin, uint8_t data, int8_t type = -1);

 private:
  int8_t signal_pin_status_;  // -1 - Not Applicable
                              //  0 - Analog Mode
                              //  1 - Digital Mode
  uint8_t signal_mode_;       // INPUT, OUTPUT, INPUT_PULLUP
  uint8_t num_of_control_pins_;
  uint8_t enable_status_;  // DISABLED, ENABLED
  uint8_t enable_pin_;
  int8_t signal_pin_;
  int8_t control_pin_[4];
  uint8_t current_channel_;
};
