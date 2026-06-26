#include "esp_camera.h"

// --- Standard ESP32-S3 Cam Pins ---
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5
#define Y2_GPIO_NUM       11
#define Y3_GPIO_NUM       9
#define Y4_GPIO_NUM       8
#define Y5_GPIO_NUM       10
#define Y6_GPIO_NUM       12
#define Y7_GPIO_NUM       18
#define Y8_GPIO_NUM       17
#define Y9_GPIO_NUM       16
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

void setup() {
  Serial.begin(921600); 
  delay(2000);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  config.xclk_freq_hz = 10000000; 
  config.frame_size = FRAMESIZE_QXGA;   // CRITICAL: Must initialize at max size for driver allocation
  config.pixel_format = PIXFORMAT_JPEG; 
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM; 
  config.jpeg_quality = 12; 
  config.fb_count = 2;      

  if (esp_camera_init(&config) != ESP_OK) {
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL && s->id.PID == OV3660_PID) {
    s->set_reg(s, 0x3824, 0x1f, 0x04); 
    s->set_reg(s, 0x460c, 0x02, 0x02); 
    s->set_aec2(s, 1);           
  }
  
  // Set initial starting preview to small resolution
  if(s != NULL) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }
}

// Helper function to flush corrupt transition frames from driver memory
void flushOldBuffers() {
  for(int i = 0; i < 2; i++) {
    camera_fb_t * fb = esp_camera_fb_get();
    if(fb) {
      esp_camera_fb_return(fb);
      delay(50);
    }
  }
}

void loop() {
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    sensor_t * s = esp_camera_sensor_get();
    
    if (s != NULL) {
      bool changed = false;
      if (incoming == '1') { s->set_framesize(s, FRAMESIZE_QVGA);  changed = true; }
      else if (incoming == '2') { s->set_framesize(s, FRAMESIZE_VGA);   changed = true; }
      else if (incoming == '3') { s->set_framesize(s, FRAMESIZE_SVGA);  changed = true; }
      else if (incoming == '4') { s->set_framesize(s, FRAMESIZE_QXGA);  changed = true; }
      
      if (changed) {
        delay(200);          // Allow sensor clock time to lock onto new parameters
        flushOldBuffers();  // Erase old resolution dimension data packets
        return;
      }
    }

    if (incoming == 'R') {
      camera_fb_t * fb = esp_camera_fb_get();
      if (fb) {
        Serial.print("START\n");
        Serial.print(String(fb->len) + "\n");
        Serial.write(fb->buf, fb->len);
        esp_camera_fb_return(fb); 
      } else {
        Serial.print("FAIL\n");
      }
    }
  }
}
