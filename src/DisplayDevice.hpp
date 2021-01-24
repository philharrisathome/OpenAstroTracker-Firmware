#pragma once

#include <LiquidCrystal.h>  // https://github.com/arduino-libraries/LiquidCrystal
#include <LiquidTWI2.h>     // https://github.com/lincomatic/LiquidTWI2
#include <U8x8lib.h>        // https://github.com/olikraus/u8g2

/**
 * @brief The prototypical interface to the underlying hardware of a DisplayDevice
 * 
 * A new *DisplayDevice should derive and then re-implement the functions in this interface for
 * the specific hardware available. The new *DisplayDevice should then be registered against a 
 * specific DISPLAY_TYPE in the preprocessor mapping below (search for DISPLAY_TYPE).
 */
class DisplayDeviceInterface
{
protected:
  static int const NUM_ROWS = 1;      ///< The number of character columns in the display (should consider font scaling)
  static int const NUM_COLUMNS = 1;   ///< The number of character rows in the display (should consider font scaling)

  /**
   * @brief Construct a new DisplayDevice.
   * 
   */
  DisplayDeviceInterface() {};

  /**
   * @brief Set the display contrast. This is best effort - some displays may not support software control.
   * 
   * @param contrast The new contrast value. 0 is darkest, 255 is brightest. This is likely to be non-linear.
   */
  void setContrast(byte contrast);

  /**
   * @brief Clear all content from the display.
   */
  void clear();

  /**
   * @brief Move the cursor position to {col (x), row (y)} in character coordinates (i.e. considering font scaling)
   * 
   * @param col The new column position. 0 is leftmost position, NUM_COLUMNS-1 is rightmost
   * @param row The new row position. 0 is topmost position, NUM_ROWS-1 is bottommost 
   */
  void setCursor(byte col, byte row);

  /**
   * @brief Print a character at the current character location. The cursor location is moved 1 character to the right 
   * after each call, wrapping to the next row if required.
   * Some characters are translated to special symbols as below (the exact symbols display are implementation-specific):
   * | ch | Mapped to:                    |
   * |----|-------------------------------|
   * | @  | Degrees symbol                |
   * | \  | Minutes symbol                |
   * | <  | Left arrow symbol             |
   * | >  | Left arrow symbol             |
   * | ^  | Up arrow symbol               |
   * | ~  | Down arrow symbol             |
   * | `  | Not currently tracking symbol |   
   * | &  | Currently tracking symbol     |   
   * 
   * @param ch The character to display, subject to symbol translation
   */
  void printChar(char ch);
};

/**
 * @brief A null device for headless operation.
 */
class NullDisplayDevice : public DisplayDeviceInterface
{
public:
  static int const NUM_ROWS = 2;      // Assume a 1602 display format
  static int const NUM_COLUMNS = 16;

public:
  NullDisplayDevice() {};

  void setContrast(byte contrast) {};
  void clear()  {};
  void setCursor(byte col, byte row)  {};
  void printChar(char ch) {};
};

/**
 * @brief Many displays share the HD44780 controller in 160x format. This class captures 
 * the common management of custom symbols and character to symbol translation.
 */
class HD44780DisplayDevice : public DisplayDeviceInterface
{
public:
  static int const NUM_ROWS = 2;      // Assume a 1602 display format
  static int const NUM_COLUMNS = 16;

protected:
  HD44780DisplayDevice() {};
  byte translateChar(char ch) const;

protected:

  // The CGRAM storage locations for custom symbols in the HD44780 memory.
  enum SpecialSymbols 
  { 
    SYMBOL_DEGREES = 0,
    SYMBOL_MINUTES,
    SYMBOL_LEFT_ARROW,
    SYMBOL_RIGHT_ARROW,
    SYMBOL_UP_ARROW,
    SYMBOL_DOWN_ARROW,
    SYMBOL_NO_TRACKING,
    SYMBOL_TRACKING
  };

  // The special character bitmaps
  static byte _degreesBitmap[8];
  static byte _minutesBitmap[8];
  static byte _leftArrowBitmap[8];
  static byte _rightArrowBitmap[8];
  static byte _upArrowBitmap[8];
  static byte _downArrowBitmap[8];
  static byte _noTrackingBitmap[8];
  static byte _trackingBitmap[8];
};

/**
 * @brief The combined LCD & keypad shield for the full-size Arduino form factor with parallel interface.
 * @pre Expects LCD_PIN8, LCD_PIN9, LCD_PIN4, LCD_PIN5, LCD_PIN6, LCD_PIN7 to be #defined in Configuration_local_*.hpp with the pin numbers for accessing the display.
 * @see https://electropeak.com/1602-lcd-keypad-shield-for-arduino
 */
class LcdKeypadShieldDisplayDevice : public HD44780DisplayDevice
{
public:
  LcdKeypadShieldDisplayDevice();
  void setContrast(byte contrast);
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, row); };
  void printChar(char ch) { _lcd.write(translateChar(ch)); };

protected:
  LiquidCrystal _lcd;   // The LCD screen that we'll display the menu on
};

/**
 * @brief The combined LCD & keypad shield for the full-size Arduino form factor with I2C interface
 * @see https://www.adafruit.com/product/716
 */
class MCP23008_MCP23017DisplayDevice : public HD44780DisplayDevice
{
public:
  void setContrast(byte contrast);
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, row); };
  void printChar(char ch) { _lcd.write(translateChar(ch)); };

  uint8_t readButtons() { return _lcd.readButtons(); }

protected:
  enum {
    DISPLAY_I2C_ADDR = 0x20          ///< I2C address of the display driver
  };

  MCP23008_MCP23017DisplayDevice(int mcp);

  LiquidTWI2 _lcd;   // The LCD screen that we'll display the menu on
};

/**
 * @brief Specialization of MCP23008_MCP23017DisplayDevice for MCP23008 controller
 */
class MCP23008DisplayDevice : public MCP23008_MCP23017DisplayDevice
{
public:
  MCP23008DisplayDevice() : MCP23008_MCP23017DisplayDevice(LTI_TYPE_MCP23008) {}
};

/**
 * @brief Specialization of MCP23008_MCP23017DisplayDevice for MCP23017 controller
 */
class MCP23017DisplayDevice : public MCP23008_MCP23017DisplayDevice
{
public:
  MCP23017DisplayDevice() : MCP23008_MCP23017DisplayDevice(LTI_TYPE_MCP23017) {}
};

/**
 * @brief A 128x32 OLED display based on the SD1306 driver with I2C interface.
 * @see https://uk.banggood.com/Geekcreit-0_91-Inch-128x32-IIC-I2C-Blue-OLED-LCD-Display-DIY-Module-SSD1306-Driver-IC-DC-3_3V-5V-p-1140506.html
 */
class SSD1306DisplayDevice : public DisplayDeviceInterface
{
public:
  static int const NUM_ROWS = 2;      // 1602 display format with 7x14 font (i.e. 32 / 14 = 2)
  static int const NUM_COLUMNS = 16;

public:
  SSD1306DisplayDevice();
  void setContrast(byte contrast)  { _lcd.setContrast(contrast); };
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, 2*row); };  // With a 7x14 font character height occupies two 8-pixel rows
  void printChar(char ch);

private:
  U8X8_SSD1306_128X32_UNIVISION_HW_I2C _lcd;  
};

/**
 * @brief Use DISPLAY_TYPE to map a concrete implementation of the *DisplayDevice to the public DisplayDevice type
 * 
 */
#if DISPLAY_TYPE == DISPLAY_TYPE_NONE
  typedef NullDisplayDevice DisplayDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD
  typedef LcdKeypadShieldDisplayDevice DisplayDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23008
  typedef MCP23008DisplayDevice DisplayDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23017
  typedef MCP23017DisplayDevice DisplayDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_JOY_I2C_SSD1306
  typedef SSD1306DisplayDevice DisplayDevice;
#else
  #error DISPLAY_TYPE must be defined
#endif
