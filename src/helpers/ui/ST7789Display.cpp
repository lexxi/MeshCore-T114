#ifdef ST7789

#include "ST7789Display.h"

#ifndef X_OFFSET
#define X_OFFSET 0  // No offset needed for landscape
#endif

#ifndef Y_OFFSET
#define Y_OFFSET 1  // Vertical offset to prevent top row cutoff
#endif

// T114 display path: MeshCore strings are UTF-8, while the bundled ArialMT
// bitmap fonts are indexed by single-byte Latin-1 code points. Convert once
// here and then use an identity lookup in OLEDDisplay so bytes >= 0x80 are not
// interpreted as UTF-8 a second time.
static char latin1FontLookup(const uint8_t ch) {
  return (char)ch;
}

static String utf8ToLatin1(const char *src) {
  String out;
  if (!src) return out;

  const uint8_t *p = (const uint8_t *)src;
  while (*p) {
    uint8_t ch = *p++;

    if (ch < 0x80) {
      out += (char)ch;
      continue;
    }

    if (ch == 0xC2 && ((*p & 0xC0) == 0x80)) {
      out += (char)(*p++);
      continue;
    }

    if (ch == 0xC3 && ((*p & 0xC0) == 0x80)) {
      out += (char)((uint8_t)(*p++) + 0x40);
      continue;
    }

    // Unsupported Unicode code point: consume continuation bytes and render '?'.
    if ((ch & 0xE0) == 0xC0) {
      if ((*p & 0xC0) == 0x80) p++;
    } else if ((ch & 0xF0) == 0xE0) {
      for (int i = 0; i < 2 && (*p & 0xC0) == 0x80; ++i) p++;
    } else if ((ch & 0xF8) == 0xF0) {
      for (int i = 0; i < 3 && (*p & 0xC0) == 0x80; ++i) p++;
    }
    out += '?';
  }

  return out;
}

#ifdef HELTEC_VISION_MASTER_T190
  #define SCALE_X  2.5f        // 320 / 128
  #define SCALE_Y  2.65625f    // 170 / 64
#else
  #define SCALE_X  1.875f      // 240 / 128
  #define SCALE_Y  2.109375f   // 135 / 64
#endif

bool ST7789Display::begin() {
  if(!_isOn) {
    pinMode(PIN_TFT_VDD_CTL, OUTPUT);
    pinMode(PIN_TFT_LEDA_CTL, OUTPUT);
    digitalWrite(PIN_TFT_VDD_CTL, LOW);
  #ifdef PIN_TFT_LEDA_CTL_ACTIVE
    digitalWrite(PIN_TFT_LEDA_CTL, PIN_TFT_LEDA_CTL_ACTIVE);
  #else
    digitalWrite(PIN_TFT_LEDA_CTL, LOW);
  #endif
    digitalWrite(PIN_TFT_RST, HIGH);

    display.init();
    display.setFontTableLookupFunction(latin1FontLookup);
    display.landscapeScreen();
    display.displayOn();
    setCursor(0,0);

    _isOn = true;
  }
  return true;
}

void ST7789Display::turnOn() {
  if (!_isOn) {
    // Restore power to the display but keep backlight off
    digitalWrite(PIN_TFT_VDD_CTL, LOW);
    digitalWrite(PIN_TFT_RST, HIGH);
    
    // Re-initialize the display
    display.init();
    display.setFontTableLookupFunction(latin1FontLookup);
    display.displayOn();
    delay(20);

    // Now turn on the backlight
  #ifdef PIN_TFT_LEDA_CTL_ACTIVE
    digitalWrite(PIN_TFT_LEDA_CTL, PIN_TFT_LEDA_CTL_ACTIVE);
  #else
    digitalWrite(PIN_TFT_LEDA_CTL, LOW);
  #endif    
    _isOn = true;
  }
}

void ST7789Display::turnOff() {
  digitalWrite(PIN_TFT_VDD_CTL, HIGH);
#ifdef PIN_TFT_LEDA_CTL_ACTIVE
  digitalWrite(PIN_TFT_LEDA_CTL, !PIN_TFT_LEDA_CTL_ACTIVE);
#else
  digitalWrite(PIN_TFT_LEDA_CTL, HIGH);
#endif
  digitalWrite(PIN_TFT_RST, LOW);
  _isOn = false;
}

void ST7789Display::clear() {
  display.clear();
}

void ST7789Display::startFrame(Color bkg) {
  display.clear();
  _color = ST77XX_WHITE;
  display.setRGB(_color);
  display.setFont(ArialMT_Plain_16);
}

void ST7789Display::setTextSize(int sz) {
  switch(sz) {
    case 1 :
      display.setFont(ArialMT_Plain_16);
      break;
    case 2 :
      display.setFont(ArialMT_Plain_24);
      break;
    default:
      display.setFont(ArialMT_Plain_16);
  }
}

void ST7789Display::setColor(Color c) {
  switch (c) {
    case DisplayDriver::DARK :
      _color = ST77XX_BLACK;
      display.setColor(OLEDDISPLAY_COLOR::BLACK);
      break;
#if 0
    case DisplayDriver::LIGHT : 
      _color = ST77XX_WHITE;
      break;
    case DisplayDriver::RED : 
      _color = ST77XX_RED;
      break;
    case DisplayDriver::GREEN : 
      _color = ST77XX_GREEN;
      break;
    case DisplayDriver::BLUE : 
      _color = ST77XX_BLUE;
      break;
    case DisplayDriver::YELLOW : 
      _color = ST77XX_YELLOW;
      break;
    case DisplayDriver::ORANGE : 
      _color = ST77XX_ORANGE;
      break;
#endif
    default:
      _color = ST77XX_WHITE;
      display.setColor(OLEDDISPLAY_COLOR::WHITE);
      break;
  }
  display.setRGB(_color);
}

void ST7789Display::setCursor(int x, int y) {
  _x = x*SCALE_X + X_OFFSET;
  _y = y*SCALE_Y + Y_OFFSET;
}

void ST7789Display::print(const char* str) {
  String text = utf8ToLatin1(str);
  display.drawString(_x, _y, text);
}

void ST7789Display::printWordWrap(const char* str, int max_width) {
  String text = utf8ToLatin1(str);
  display.drawStringMaxWidth(_x, _y, max_width*SCALE_X, text);
}

void ST7789Display::fillRect(int x, int y, int w, int h) {
  display.fillRect(x*SCALE_X + X_OFFSET, y*SCALE_Y + Y_OFFSET, w*SCALE_X, h*SCALE_Y);
}

void ST7789Display::drawRect(int x, int y, int w, int h) {
  display.drawRect(x*SCALE_X + X_OFFSET, y*SCALE_Y + Y_OFFSET, w*SCALE_X, h*SCALE_Y);
}

void ST7789Display::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  // Calculate the base position in display coordinates
  uint16_t startX = x * SCALE_X + X_OFFSET;
  uint16_t startY = y * SCALE_Y + Y_OFFSET;
  
  // Width in bytes for bitmap processing
  uint16_t widthInBytes = (w + 7) / 8;
  
  // Process the bitmap row by row
  for (uint16_t by = 0; by < h; by++) {
    // Calculate the target y-coordinates for this logical row
    int y1 = startY + (int)(by * SCALE_Y);
    int y2 = startY + (int)((by + 1) * SCALE_Y);
    int block_h = y2 - y1;
    
    // Scan across the row bit by bit
    for (uint16_t bx = 0; bx < w; bx++) {
      // Calculate the target x-coordinates for this logical column
      int x1 = startX + (int)(bx * SCALE_X);
      int x2 = startX + (int)((bx + 1) * SCALE_X);
      int block_w = x2 - x1;
      
      // Get the current bit
      uint16_t byteOffset = (by * widthInBytes) + (bx / 8);
      uint8_t bitMask = 0x80 >> (bx & 7);
      bool bitSet = pgm_read_byte(bits + byteOffset) & bitMask;
      
      // If the bit is set, draw a block of pixels
      if (bitSet) {
        // Draw the block as a filled rectangle
        display.fillRect(x1, y1, block_w, block_h);
      }
    }
  }
}

uint16_t ST7789Display::getTextWidth(const char* str) {
  String text = utf8ToLatin1(str);
  return display.getStringWidth(text) / SCALE_X;
}

void ST7789Display::endFrame() {
  display.display();
}

#endif