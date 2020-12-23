#ifndef _LCDMENU_HPP_
#define _LCDMENU_HPP_
#include <Arduino.h>
#include "../Configuration_adv.hpp"

#include <LiquidCrystal.h>
#include <LiquidTWI2.h>
#include <U8x8lib.h>        // https://github.com/olikraus/u8g2

// A single menu item (like RA, HEAT, POL, etc.)
// The ID is just a number, it has no relevance for the order of the items
// The display is what is shown on the menu.
class MenuItem {
  const char* _display;     // What to display on the screen
  byte _id;             // The ID of the menu item
public:
  MenuItem(const char* display, byte id) {
    _display = display;
    _id = id;
  }

  const char* display() {
    return _display;
  }

  byte id() {
    return _id;
  }
};




class LcdInterface
{
};

class LcdNull : public LcdInterface
{
public:
  LcdNull();

  void setContrast(byte contrast) {};
  void clear()  {};
  void setCursor(byte col, byte row)  {};
  void printChar(char ch) {};
  void print(char const * str) {};
};

class LcdHD44780 : public LcdInterface
{
protected:
  LcdHD44780() {};
  byte translateChar(char ch) const;

protected:

  byte _degrees = 1;
  byte _minutes = 2;
  byte _leftArrow = 3;
  byte _rightArrow = 4;
  byte _upArrow = 5;
  byte _downArrow = 6;
  byte _tracking = 7;
  byte _noTracking = 0;

  // The special character bitmaps
  static byte RightArrowBitmap[8];
  static byte LeftArrowBitmap[8];
  static byte UpArrowBitmap[8];
  static byte DownArrowBitmap[8];
  static byte DegreesBitmap[8];
  static byte MinutesBitmap[8];
  static byte TrackingBitmap[8];
  static byte NoTrackingBitmap[8];
};

class LcdKeypadShield : public LcdHD44780
{
public:
  LcdKeypadShield(byte cols, byte rows);
  void setContrast(byte contrast);
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, row); };
  void printChar(char ch) { _lcd.write(translateChar(ch)); };
  void print(char const * str) { _lcd.print(str); }

public:
  LiquidCrystal _lcd;   // The LCD screen that we'll display the menu on
};

class LcdMCP23008_MCP23017 : public LcdHD44780
{
protected:
  LcdMCP23008_MCP23017(byte cols, byte rows, int mcp);
public:
  void setContrast(byte contrast);
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, row); };
  void printChar(char ch) { _lcd.write(translateChar(ch)); };
  void print(char const * str) { _lcd.print(str); }

public:
  LiquidTWI2 _lcd;   // The LCD screen that we'll display the menu on
};

class LcdMCP23008 : public LcdMCP23008_MCP23017
{
public:
  LcdMCP23008(byte cols, byte rows) : LcdMCP23008_MCP23017(cols, rows, LTI_TYPE_MCP23008) {}
};

class LcdMCP23017 : public LcdMCP23008_MCP23017
{
public:
  LcdMCP23017(byte cols, byte rows) : LcdMCP23008_MCP23017(cols, rows, LTI_TYPE_MCP23017) {}
};

class LcdSSD1306 : public LcdInterface
{
public:
  LcdSSD1306(byte cols, byte rows);
  void setContrast(byte contrast);
  void clear()  { _lcd.clear(); };
  void setCursor(byte col, byte row)  { _lcd.setCursor(col, 2*row); };
  void printChar(char ch);
  void print(char const * str) { _lcd.print(str); }

private:
  U8X8_SSD1306_128X32_UNIVISION_HW_I2C _lcd;  
};





template <class T> class LcdMenuBase
{
public:
  // Create a new menu, using the given number of LCD display columns and rows
  LcdMenuBase(byte cols, byte rows, int maxItems);

  void startup();

  // Find a menu item by its ID
  MenuItem* findById(byte id);

  // Add a new menu item to the list (order matters)
  void addItem(const char* disp, byte id);

  // Get the currently active item ID
  byte getActive();

  // Set the active menu item
  void setActive(byte id);

  // Pass thru utility function
  void setCursor(byte col, byte row);

  // Set and get the brightness of the backlight
  void setBacklightBrightness(int level, bool persist = true);
  int getBacklightBrightness();

  // Pass thru utility function
  void clear();

  // Go to the next menu item from currently active one
  void setNextActive();

  // Update the display of the LCD with the current menu settings
  // This iterates over the menu items, building a menu string by concatenating their display string.
  // It also places the selector arrows around the active one.
  // It then sends the string to the LCD, keeping the selector arrows centered in the same place.
  void updateDisplay();

  // Print a string to the LCD at the current cursor position, substituting the special arrows and padding with spaces to the end
  void printMenu(String line);

  // Print a character at a specific position
  void printAt(int col, int row, char ch);

  #if DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23008 || DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23017
  uint8_t readButtons();
  #endif

private:
  byte const _columns;    // The number of columns in the LCD display
  byte const _maxItems;

  MenuItem** _menuItems;  // The first menu item (array of _maxItems)

  byte _numMenuItems;
  byte _activeMenuIndex;
  byte _longestDisplay;   // The number of characters in the longest menu item
  byte _activeRow;        // The row that the LCD cursor is on
  byte _activeCol;        // The column that the LCD cursor is on
  String _lastDisplay[2]; // The last string that was displayed on each row
  byte _brightness;

  T _lcd;
};

/**
 * @brief Specialised empty implementation for headless operation to minimise memory usage.
 * 
 */
template <> class LcdMenuBase<LcdNull>
{
public:
  LcdMenuBase(byte cols, byte rows, int maxItems) {};
  MenuItem *findById(byte id) { return 0; };
  void addItem(const char *disp, byte id) {};
  byte getActive() { return 0; };
  void setActive(byte id) {};
  void setCursor(byte col, byte row) {};
  void clear() {};
  void setNextActive() {};
  void updateDisplay() {};
  void printMenu(String line) {};
  void printChar(char ch) {};
  void printAt(int col, int row, char ch) {};
};

// Explicitly instantiate to allow definition in cpp file
template class LcdMenuBase<LcdKeypadShield>;
template class LcdMenuBase<LcdMCP23008>;
template class LcdMenuBase<LcdMCP23017>;
template class LcdMenuBase<LcdSSD1306>;

/**
 * @brief Use DISPLAY_TYPE to map a concrete implementation of the LcdMenuInterface to the public LcdMenu type
 * 
 */
#if DISPLAY_TYPE == DISPLAY_TYPE_NONE
  typedef LcdMenuBase<LcdNull> LcdMenu;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD
  typedef LcdMenuBase<LcdKeypadShield> LcdMenu;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23008
  typedef LcdMenuBase<LcdMCP23008> LcdMenu;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23017
  typedef LcdMenuBase<LcdMCP23017> LcdMenu;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_JOY_I2C_SSD1306
  typedef LcdMenuBase<LcdSSD1306> LcdMenu;
#endif

#endif
