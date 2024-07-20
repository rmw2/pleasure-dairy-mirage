#pragma once

#include <ETH.h>
#include <WifiUdp.h>

#include "wled.h"
#include "mpr121.h"
#include "mpr121_defs.h"


#define GPIO_RANGE_MAX 33
#define I2C_ADDRESS 0x5A
#define SCL_GPIO 22
#define SDA_GPIO 21
#define IRQ_GPIO 15

#define TOUCH_VAL 40
#define RELEASE_VAL 20
#define N_TOUCHES 12

#define UDP_PORT 42069


const char *HOST = "192.168.1.100";

class TouchToUDP : public Usermod {
  private:
    MPR121_t dev_;
    WiFiUDP udp_;

    uint16_t touch_thresh_ = 40;
    uint16_t release_thresh_ = 20;
    uint8_t buffer_[2];

    void sendOverUDP(uint8_t id, bool touch) {
      buffer_[0] = id;
      buffer_[1] = touch ? 1 : 0;
      

      udp_.beginPacket(HOST, UDP_PORT);
      udp_.write(buffer_, 2);
      udp_.endPacket();
      memset(buffer_, 0, 2);
    }

  public:
    void setup() {
      if (!MPR121_begin(&dev_, I2C_ADDRESS, TOUCH_VAL, RELEASE_VAL, IRQ_GPIO,
                        SDA_GPIO, SCL_GPIO)) {
        Serial.println("MPR121 init error: ");
        // MPR121_getError(&dev_);
      }
      
      MPR121_setTouchThresholdAll(&dev_, touch_thresh_);
      MPR121_setReleaseThresholdAll(&dev_, release_thresh_);

      MPR121_setFFI(&dev_, FFI_10);
      MPR121_setSFI(&dev_, SFI_10);
      MPR121_setGlobalCDT(&dev_, CDT_4US);
      MPR121_autoSetElectrodesDefault(&dev_, true);
    }

    void connected() {
      Serial.println("TouchMod: Connecting to upstream host");


    }

    void loop() {
      MPR121_updateAll(&dev_);

      for (uint8_t i = 0; i < N_TOUCHES; i++) {
        if (MPR121_isNewTouch(&dev_, i)) {
          sendOverUDP(i, true);
        } else if (MPR121_isNewRelease(&dev_, i)) {
          sendOverUDP(i, false);
        }
      }
    }
};