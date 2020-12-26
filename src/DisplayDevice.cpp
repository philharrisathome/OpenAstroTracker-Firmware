#include "Utility.hpp"
#include "DisplayDevice.hpp"

/// The right arrow bitmap
byte HD44780DisplayDevice::_rightArrowBitmap[8] = {
    B00000,
    B01000,
    B01100,
    B01110,
    B01100,
    B01000,
    B00000,
    B00000};

/// The left arrow bitmap
byte HD44780DisplayDevice::_leftArrowBitmap[8] = {
    B00000,
    B00010,
    B00110,
    B01110,
    B00110,
    B00010,
    B00000,
    B00000};

/// The up arrow bitmap
byte HD44780DisplayDevice::_upArrowBitmap[8] = {
    B00100,
    B01110,
    B11111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100};

/// The down arrow bitmap
byte HD44780DisplayDevice::_downArrowBitmap[8] = {
    B000100,
    B000100,
    B000100,
    B000100,
    B000100,
    B011111,
    B001110,
    B000100};

/// The degrees symbol bitmap
byte HD44780DisplayDevice::_degreesBitmap[8] = {
    B01100,
    B10010,
    B10010,
    B01100,
    B00000,
    B00000,
    B00000,
    B00000};

/// The minutes symbol bitmap
byte HD44780DisplayDevice::_minutesBitmap[8] = {
    B01000,
    B01000,
    B01000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

/// The tracking enabled bitmap
byte HD44780DisplayDevice::_trackingBitmap[8] = {
    B10111,
    B00010,
    B10010,
    B00010,
    B10111,
    B00101,
    B10110,
    B00101};

/// The tracking disabled bitmap
byte HD44780DisplayDevice::_noTrackingBitmap[8] = {
    B10000,
    B00000,
    B10000,
    B00010,
    B10000,
    B00000,
    B10000,
    B00000};

/**
 * @brief Print the given character to the LCD, converting some special ones to custom bitmaps.
 * 
 * @param ch The character to translate.
 * @return byte The CGRAM/CGROM index for the translated ch in HD44780 controller memory.
 */
byte HD44780DisplayDevice::translateChar(char ch) const
{
  if (ch == '@')  return SYMBOL_DEGREES;
  if (ch == '\'') return SYMBOL_MINUTES;
  if (ch == '<')  return SYMBOL_LEFT_ARROW;
  if (ch == '>')  return SYMBOL_RIGHT_ARROW;
  if (ch == '^')  return SYMBOL_UP_ARROW;
  if (ch == '~')  return SYMBOL_DOWN_ARROW;
  if (ch == '`')  return SYMBOL_NO_TRACKING;
  if (ch == '&')  return SYMBOL_TRACKING;
  return ch;
}

// These pins allocations may not be available in all hardware configurations, so set them to invalid but safe values to allow compilation
#if !defined(LCD_PIN8) || !defined(LCD_PIN9) || !defined(LCD_PIN4) || !defined(LCD_PIN5) || !defined(LCD_PIN6) || !defined(LCD_PIN7) 
  #define LCD_PIN8 (-1) 
  #define LCD_PIN9 (-1) 
  #define LCD_PIN4 (-1) 
  #define LCD_PIN5 (-1) 
  #define LCD_PIN6 (-1) 
  #define LCD_PIN7 (-1)
#endif

/**
 * @brief Construct the LcdKeypadShieldDisplayDevice.
 * Initialise the underlying HD44780 controller with the custom character bitmaps.
 */
LcdKeypadShieldDisplayDevice::LcdKeypadShieldDisplayDevice()
: _lcd(LCD_PIN8, LCD_PIN9, LCD_PIN4, LCD_PIN5, LCD_PIN6, LCD_PIN7)
{
  _lcd.begin(NUM_COLUMNS, NUM_ROWS);

  // Create special characters for degrees and arrows
  _lcd.createChar(SYMBOL_DEGREES, _degreesBitmap);
  _lcd.createChar(SYMBOL_MINUTES, _minutesBitmap);
  _lcd.createChar(SYMBOL_LEFT_ARROW, _leftArrowBitmap);
  _lcd.createChar(SYMBOL_RIGHT_ARROW, _rightArrowBitmap);
  _lcd.createChar(SYMBOL_UP_ARROW, _upArrowBitmap);
  _lcd.createChar(SYMBOL_DOWN_ARROW, _downArrowBitmap);
  _lcd.createChar(SYMBOL_NO_TRACKING, _noTrackingBitmap);
  _lcd.createChar(SYMBOL_TRACKING, _trackingBitmap);
}

/**
 * @brief Set the display contrast.
 * If the optional LCD_BRIGHTNESS_PIN is not #defined in Configuration_local_*.hpp then this function does nothing.
 * 
 * @param contrast The new contrast value. 0 is darkest, 255 is brightest. This is likely to be non-linear.
 */
void LcdKeypadShieldDisplayDevice::setContrast(byte contrast)
{
  // If optional LCD_BRIGHTNESS_PIN has not been defined then do nothing
  #if defined(LCD_BRIGHTNESS_PIN)
    // Not supported on ESP32 due to lack of built-in analogWrite()
    // TODO: Verify that this works correctly on ATmega (reports of crashes)
    // analogWrite(LCD_BRIGHTNESS_PIN, contrast);
  #endif
}

/**
 * @brief Construct the MCP23008_MCP23017DisplayDevice.
 * Initialise the underlying HD44780 controller with the custom character bitmaps.
 * 
 * @param mcp Either LTI_TYPE_MCP23008 or LTI_TYPE_MCP23017 from the LiquidTWI2 library
 */
MCP23008_MCP23017DisplayDevice::MCP23008_MCP23017DisplayDevice(int mcp)
: _lcd(DISPLAY_I2C_ADDR)
{
  _lcd.begin(NUM_COLUMNS, NUM_ROWS);
  _lcd.setBacklight(RED);
  _lcd.setMCPType(mcp);

  // Create special characters for degrees and arrows
  _lcd.createChar(SYMBOL_DEGREES, _degreesBitmap);
  _lcd.createChar(SYMBOL_MINUTES, _minutesBitmap);
  _lcd.createChar(SYMBOL_LEFT_ARROW, _leftArrowBitmap);
  _lcd.createChar(SYMBOL_RIGHT_ARROW, _rightArrowBitmap);
  _lcd.createChar(SYMBOL_UP_ARROW, _upArrowBitmap);
  _lcd.createChar(SYMBOL_DOWN_ARROW, _downArrowBitmap);
  _lcd.createChar(SYMBOL_NO_TRACKING, _noTrackingBitmap);
  _lcd.createChar(SYMBOL_TRACKING, _trackingBitmap);
}

void MCP23008_MCP23017DisplayDevice::setContrast(byte /* unused */)
{
  // Nothing to do - not available on this hardware
}

SSD1306DisplayDevice::SSD1306DisplayDevice()
{
  _lcd.begin();
  _lcd.setPowerSave(0);
  _lcd.clear();
  _lcd.setFont(u8x8_font_7x14_1x2_f);   // Each 7x14 character takes up 2 8-pixel rows
}

/**
 * @brief Print a single character at the current cursor location and advance cursor by one. Substitutes special chars.
 * Three font tables are used. The relevant character codes were identified through the links below:
 * https://github.com/olikraus/u8g2/wiki/fntgrpx11#7x14 for "normal" characters
 * https://github.com/olikraus/u8g2/wiki/fntgrpiconic#open_iconic_arrow_1x1 for arrow symbols
 * https://github.com/olikraus/u8g2/wiki/fntgrpiconic#open_iconic_thing_1x1 for some special symbols
 * 
 * @param ch The character to translate and display.
 */
void SSD1306DisplayDevice::printChar(char ch)
{
  if (ch == '>')
  {
    _lcd.setFont(u8x8_font_open_iconic_arrow_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+14);  // Right arrow
  }
  else if (ch == '<')
  {
    _lcd.setFont(u8x8_font_open_iconic_arrow_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+13);  // Left arrow
  }
  else if (ch == '^')
  {
    _lcd.setFont(u8x8_font_open_iconic_arrow_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+15);  // Up arrow  
  }
  else if (ch == '~')
  {
    _lcd.setFont(u8x8_font_open_iconic_arrow_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+12);  // Down arrow
  }
  else if (ch == '@')
  {
    _lcd.setFont(u8x8_font_7x14_1x2_f); 
    _lcd.drawGlyph(_lcd.tx++,_lcd.ty,176);    // Degrees
  }
  else if (ch == '&')
  {
    _lcd.setFont(u8x8_font_open_iconic_thing_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+15);  // Tracking
  }
  else if (ch == '`')
  {
    _lcd.setFont(u8x8_font_open_iconic_thing_1x1);
    _lcd.draw1x2Glyph(_lcd.tx++,_lcd.ty,64+4);  // Not tracking
  }
  else
  {
    _lcd.setFont(u8x8_font_7x14_1x2_f);  
    _lcd.drawGlyph(_lcd.tx++,_lcd.ty,ch);   // No translation
  }
}
