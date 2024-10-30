#include <Arduino.h>
#include <U8g2lib.h>
#include <Bounce2.h>

#define forward_button 19
#define back_button 12

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Bounce2::Button forward = Bounce2::Button();
Bounce2::Button back = Bounce2::Button();

const char* version = "v1.1";
const char* menuItems[] = {
  "Home Automation",
  "IR Send",
  "OLED Animation",
};
const int totalMenu = sizeof(menuItems) / sizeof(menuItems[0]);

int selectItem = 0;

void setup(){
  Serial.begin(115200);
  u8g2.begin();

  forward.attach(forward_button, INPUT_PULLUP);
  forward.interval(5);
  forward.setPressedState(LOW);

  back.attach(back_button, INPUT_PULLUP);
  back.interval(5);
  back.setPressedState(LOW);
}

void loop() {
  forward.update();
  back.update();
  drawMenu();
}

void drawMenuList() {
  for (int i= 0; i < totalMenu; i++) {
    int yPos = (i * 12) + 26;
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    u8g2.drawStr(1, yPos, menuItems[i]);
  }
}

void highlightSelectedItem() {
  const int yPos[3] = {15, 27, 39};

  u8g2.setDrawColor(2);
  u8g2.drawBox(0, yPos[selectItem], 128, 12);

  if (forward.pressed()) {
    selectItem = (selectItem + 1) % totalMenu;
  }

  if (back.pressed()) {
    selectItem = (selectItem - 1 + totalMenu) % totalMenu;
  }
}

void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  // Header
  u8g2.setFont(u8g2_font_spleen8x16_mr);
  u8g2.drawStr(28, 10, "MAIN MENU");
  u8g2.drawHLine(0, 12, 128);

  drawMenuList();
  highlightSelectedItem();

  // Footer
  u8g2.drawHLine(0, 53, 128);
  u8g2.setFont(u8g2_font_minuteconsole_mr );
  u8g2.drawStr(108, 63, version);
  
  u8g2.sendBuffer();
}