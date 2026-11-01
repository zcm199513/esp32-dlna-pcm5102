#include <WiFiManager.h>
#include <DlnaRenderer.h>
#include <AudioTools.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ========== 引脚定义 ==========
#define I2S_BCK 42
#define I2S_WS  38
#define I2S_DIN 40

#define OLED_SDA 16
#define OLED_SCL 17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
// 你的屏幕：0x78(8bit) = 0x3C(7bit)；0x7A(8bit)=0x3D
#define OLED_ADDR 0x3C

I2SStream i2s;
DlnaRenderer dlna;
TwoWire oledWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &oledWire, OLED_RESET);

bool oledOK = false;
const String devName = "ESP32-PCM5102-DLNA";

void screenPrint(String line1, String line2, String line3, String line4){
  if(!oledOK) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(line1);
  display.println(line2);
  display.println(line3);
  display.println(line4);
  display.display();
}

void setup() {
  Serial.begin(115200);

  // OLED初始化，容错：没接屏幕直接跳过，不卡死
  oledWire.begin(OLED_SDA, OLED_SCL);
  if(display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)){
    oledOK = true;
    screenPrint("Starting...", devName, "", "");
  }else{
    Serial.println("SSD1306 未检测到，跳过屏幕");
    oledOK = false;
  }

  // WiFiManager配网热点
  WiFiManager wm;
  wm.autoConnect("ESP32-DLNA");

  // I2S PCM5102A配置
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = I2S_BCK;
  cfg.pin_ws  = I2S_WS;
  cfg.pin_data_out = I2S_DIN;
  i2s.begin(cfg);

  // DLNA启动
  dlna.begin(i2s);
  dlna.setName(devName.c_str());
  dlna.setVolume(0.8f); // 默认音量 0~1.0

  if(oledOK){
    screenPrint("WiFi OK", WiFi.localIP().toString(), "Ready", "");
  }
}

void loop() {
  dlna.copy();

  // 1秒刷新一次屏幕
  static uint32_t lastT = 0;
  if(millis() - lastT > 1000 && oledOK){
    lastT = millis();
    String vol = "Vol:" + String(dlna.getVolume()*100,0) + "%";
    String title = dlna.getTitle();
    if(title.length()>20) title = title.substring(0,20);
    screenPrint(devName, WiFi.localIP().toString(), vol, title);
  }
}
