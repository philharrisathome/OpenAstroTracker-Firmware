#ifndef _LCDMENU_HPP_
#define _LCDMENU_HPP_
#include <Arduino.h>
#include "../Configuration_adv.hpp"
#include "DisplayDevice.hpp"
#include "KeypadDevice.hpp"

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


class FullLcdMenu
{
public:
  // Create a new menu, using the given number of LCD display columns and rows
  FullLcdMenu(int maxItems);

  void startup();

  KeypadDevice& getKeypad() { return _keypad; }

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

  uint8_t readButtons();

private:
  byte const _maxItems;   // The maximum number of MenuItems that will be stored in _menuItems
  MenuItem** _menuItems;  // The first menu item (array of _maxItems)

  byte _numMenuItems;
  byte _activeMenuIndex;
  byte _longestDisplay;   // The number of characters in the longest menu item
  byte _activeRow;        // The row that the LCD cursor is on
  byte _activeCol;        // The column that the LCD cursor is on
  String _lastDisplay[DisplayDevice::NUM_ROWS]; // The last string that was displayed on each row
  byte _brightness;

  DisplayDevice _display;
  KeypadDevice _keypad;
};

/**
 * @brief Specialised empty implementation for headless operation to minimise memory usage.
 * 
 */
class NullLcdMenu
{
public:
  NullLcdMenu(byte cols, byte rows, int maxItems) {};
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

/**
 * @brief Use DISPLAY_TYPE to map a concrete implementation of the LcdMenu to the public LcdMenu type
 * 
 */
#if DISPLAY_TYPE == DISPLAY_TYPE_NONE
  typedef NullLcdMenu LcdMenu;
#else
  typedef FullLcdMenu LcdMenu;
#endif

#endif

