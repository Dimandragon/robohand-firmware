#include "MUX74HC4067.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "hal/gpio_types.h"


/*
constexpr bool HIGH = true;
constexpr bool LOW = false;
constexpr bool OUTPUT = true;
constexpr bool INPUT = false;

esp_err_t pinMode(uint8_t gpio, bool direction){
  if (direction) {
    return gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
  }
  else{
    return gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_INPUT);
  }
}
esp_err_t digitalWrite(uint8_t gpio, bool level){
  return gpio_set_level((gpio_num_t)gpio, level);
}*/


MUX74HC4067::MUX74HC4067(uint8_t en, int8_t s0, int8_t s1, int8_t s2, int8_t s3)
    : enable_pin_(en),
      enable_status_(DISABLED),
      signal_pin_status_(-1),
      signal_pin_(-1),
      signal_mode_(0),
      current_channel_(0),
      num_of_control_pins_(1) {
  pinMode(en, OUTPUT);
  digitalWrite(en, HIGH);  // Initially disables the connection of the SIG pin
                           // to the channels

  control_pin_[0] = s0;
  num_of_control_pins_ += ((control_pin_[1] = s1) != -1) ? 1 : 0;
  num_of_control_pins_ += ((control_pin_[2] = s2) != -1) ? 1 : 0;
  num_of_control_pins_ += ((control_pin_[3] = s3) != -1) ? 1 : 0;
  for (uint8_t i = 0; i < num_of_control_pins_; ++i) {
    pinMode(control_pin_[i], OUTPUT);
  }
}

void MUX74HC4067::setChannel(int8_t pin, uint8_t set) {
  digitalWrite(enable_pin_, HIGH);
  current_channel_ = pin;
  for (uint8_t i = 0; i < num_of_control_pins_; ++i) {
    digitalWrite(control_pin_[i], pin & 0x01);
    pin >>= 1;
  }
  enable_status_ = ENABLED;
  if (set == ENABLED) digitalWrite(enable_pin_, LOW);
}

void MUX74HC4067::enable() {
  enable_status_ = ENABLED;
  digitalWrite(enable_pin_, LOW);
}

void MUX74HC4067::disable() {
  enable_status_ = DISABLED;
  digitalWrite(enable_pin_, HIGH);
}

void MUX74HC4067::signalPin(uint8_t sig, uint8_t mode, uint8_t type) {
  signal_pin_ = sig; //signal pin hear

  if (mode == INPUT) {
    signal_mode_ = INPUT;
    digitalWrite(sig, LOW);  // Disables pullup
    pinMode(sig, INPUT);
  } else if (mode == OUTPUT) {
    signal_mode_ = OUTPUT;
    pinMode(sig, OUTPUT);
  } else if (mode == INPUT_PULLUP) {
    signal_mode_ = INPUT_PULLUP;
    pinMode(sig, INPUT_PULLUP);
  }

  if (type == ANALOG) {
    signal_pin_status_ = 0;
  } else if (type == DIGITAL) {
    signal_pin_status_ = 1;
  }
}

int16_t MUX74HC4067::read(int8_t chan_pin) {
  uint8_t last_channel;
  uint8_t last_en_status = enable_status_;
  if (chan_pin != -1) {
    last_channel = current_channel_;
    setChannel(chan_pin);
  }

  int16_t data;
  if (signal_pin_status_ == 0) {
    data = analogRead(signal_pin_);
  } else if (signal_pin_status_ == 1) {
    data = digitalRead(signal_pin_);
  } else {
    data = -1;
  }

  if (chan_pin != -1) {
    setChannel(last_channel, last_en_status);
  }
  return data;
}

uint8_t MUX74HC4067::write(int8_t chan_pin, uint8_t data, int8_t type) {
  if (signal_mode_ == INPUT || signal_mode_ == INPUT_PULLUP) return -1;

  disable();

  if (type == ANALOG) {
    analogWrite(signal_pin_, data);
  } else if (type == DIGITAL) {
    digitalWrite(signal_pin_, data);
  } else {
    if (signal_pin_status_ == 0) {
      analogWrite(signal_pin_, data);
    } else if (signal_pin_status_ == 1) {
      digitalWrite(signal_pin_, data);
    }
  }

  setChannel(chan_pin);

  return 0;
}