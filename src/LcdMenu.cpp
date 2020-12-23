#include "Utility.hpp"
#include "EPROMStore.hpp"
#include "LcdMenu.hpp"
#include "inc/Config.hpp"

// The right arrow bitmap
byte LcdHD44780::RightArrowBitmap[8] = {
    B00000,
    B01000,
    B01100,
    B01110,
    B01100,
    B01000,
    B00000,
    B00000};

// The left arrow bitmap
byte LcdHD44780::LeftArrowBitmap[8] = {
    B00000,
    B00010,
    B00110,
    B01110,
    B00110,
    B00010,
    B00000,
    B00000};

byte LcdHD44780::UpArrowBitmap[8] = {
    B00100,
    B01110,
    B11111,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100};

byte LcdHD44780::DownArrowBitmap[8] = {
    B000100,
    B000100,
    B000100,
    B000100,
    B000100,
    B011111,
    B001110,
    B000100};

byte LcdHD44780::DegreesBitmap[8] = {
    B01100,
    B10010,
    B10010,
    B01100,
    B00000,
    B00000,
    B00000,
    B00000};

byte LcdHD44780::MinutesBitmap[8] = {
    B01000,
    B01000,
    B01000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

byte LcdHD44780::TrackingBitmap[8] = {
    B10111,
    B00010,
    B10010,
    B00010,
    B10111,
    B00101,
    B10110,
    B00101};

byte LcdHD44780::NoTrackingBitmap[8] = {
    B10000,
    B00000,
    B10000,
    B00010,
    B10000,
    B00000,
    B10000,
    B00000};

// Print the given character to the LCD, converting some special ones to our bitmaps
byte LcdHD44780::translateChar(char ch) const
{
  if (ch == '>')
  {
    return _rightArrow;
  }
  else if (ch == '<')
  {
   return _leftArrow;
  }
  else if (ch == '^')
  {
    return _upArrow;
  }
  else if (ch == '~')
  {
    return _downArrow;
  }
  else if (ch == '@')
  {
    return _degrees;
  }
  else if (ch == '\'')
  {
   return _minutes;
  }
  else if (ch == '&')
  {
    return _tracking;
  }
  else if (ch == '`')
  {
    return _noTracking;
  }
  else
  {
    return ch;
  }
}

LcdKeypadShield::LcdKeypadShield(byte cols, byte rows)
: _lcd(LCD_PIN8, LCD_PIN9, LCD_PIN4, LCD_PIN5, LCD_PIN6, LCD_PIN7)
{
  _lcd.begin(cols, rows);

  // Create special characters for degrees and arrows
  _lcd.createChar(_degrees, DegreesBitmap);
  _lcd.createChar(_minutes, MinutesBitmap);
  _lcd.createChar(_leftArrow, LeftArrowBitmap);
  _lcd.createChar(_rightArrow, RightArrowBitmap);
  _lcd.createChar(_upArrow, UpArrowBitmap);
  _lcd.createChar(_downArrow, DownArrowBitmap);
  _lcd.createChar(_tracking, TrackingBitmap);
  _lcd.createChar(_noTracking, NoTrackingBitmap);
}

void LcdKeypadShield::setContrast(byte contrast)
{
  // If optional LCD_BRIGHTNESS_PIN has not been defined then do nothing
  #if defined(LCD_BRIGHTNESS_PIN)
    // Not supported on ESP32 due to lack of built-in analogWrite()
    // TODO: Verify that this works correctly on ATmega (reports of crashes)
    // analogWrite(LCD_BRIGHTNESS_PIN, contrast);
  #endif
}

LcdMCP23008_MCP23017::LcdMCP23008_MCP23017(byte cols, byte rows, int mcp)
: _lcd(0x20)
{
  _lcd.begin(cols, rows);
  _lcd.setBacklight(RED);
  _lcd.setMCPType(mcp);

  // Create special characters for degrees and arrows
  _lcd.createChar(_degrees, DegreesBitmap);
  _lcd.createChar(_minutes, MinutesBitmap);
  _lcd.createChar(_leftArrow, LeftArrowBitmap);
  _lcd.createChar(_rightArrow, RightArrowBitmap);
  _lcd.createChar(_upArrow, UpArrowBitmap);
  _lcd.createChar(_downArrow, DownArrowBitmap);
  _lcd.createChar(_tracking, TrackingBitmap);
  _lcd.createChar(_noTracking, NoTrackingBitmap);
}

void LcdMCP23008_MCP23017::setContrast(byte /* unused */)
{
  // Nothing to do?
}

LcdSSD1306::LcdSSD1306(byte /* unused */, byte /* unused */)
{
  _lcd.begin();
  _lcd.setPowerSave(0);
  _lcd.clear();
  _lcd.setFont(u8x8_font_7x14_1x2_f);   // Each 7x14 character takes up 2 8-pixel rows
}

void LcdSSD1306::setContrast(byte contrast)
{
  _lcd.setContrast(contrast);
}

// Print a single character at the current cursor location and advance cursor by one. Substitutes special chars.
void LcdSSD1306::printChar(char ch)
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
    _lcd.drawGlyph(_lcd.tx++,_lcd.ty,ch);
  }
}

// Class that drives the LCD screen with a menu
// You add a string and an id item and this class handles the display and navigation
// Create a new menu, using the given number of LCD display columns and rows

template <class T> LcdMenuBase<T>::LcdMenuBase(byte cols, byte rows, int maxItems)
: _columns(cols), _maxItems(maxItems), _lcd(cols, rows) 
{
}

template <class T> void LcdMenuBase<T>::startup()
{
  LOGV1(DEBUG_INFO, F("LcdMenu:: startup"));

  _brightness = EEPROMStore::getBrightness();
  LOGV2(DEBUG_INFO, F("LCD: Brightness from EEPROM is %d"), _brightness);
  setBacklightBrightness(_brightness, false);

  _numMenuItems = 0;
  _activeMenuIndex = 0;
  _longestDisplay = 0;
  _activeRow = -1;
  _activeCol = -1;
  _lastDisplay[0] = "";
  _lastDisplay[1] = "";
  _menuItems = new MenuItem *[_maxItems];
}

// Find a menu item by its ID
template <class T> MenuItem *LcdMenuBase<T>::findById(byte id)
{
  for (byte i = 0; i < _numMenuItems; i++)
  {
    if (_menuItems[i]->id() == id)
    {
      return _menuItems[i];
    }
  }
  return NULL;
}

// Add a new menu item to the list (order matters)
template <class T> void LcdMenuBase<T>::addItem(const char *disp, byte id)
{
  _menuItems[_numMenuItems++] = new MenuItem(disp, id);
  _longestDisplay = max((size_t)_longestDisplay, strlen(disp));
}

// Get the currently active item ID
template <class T> byte LcdMenuBase<T>::getActive()
{
  return _menuItems[_activeMenuIndex]->id();
}

// Set the active menu item
template <class T> void LcdMenuBase<T>::setActive(byte id)
{
  for (byte i = 0; i < _numMenuItems; i++)
  {
    if (_menuItems[i]->id() == id)
    {
      _activeMenuIndex = i;
      break;
    }
  }
}

// Pass thru utility function
template <class T> void LcdMenuBase<T>::setCursor(byte col, byte row)
{
  _activeRow = row;
  _activeCol = col;
}

// Pass thru utility function
template <class T> void LcdMenuBase<T>::clear()
{
  _lcd.clear();
}

// Set the brightness of the backlight
template <class T> void LcdMenuBase<T>::setBacklightBrightness(int level, bool persist)
{
  _brightness = level;
  _lcd.setContrast(_brightness);
  if (persist)
  {
    LOGV2(DEBUG_INFO, F("LCD: Saving %d as brightness"), _brightness);
    EEPROMStore::storeBrightness(_brightness);
  }
}

// Get the current brightness
template <class T> int LcdMenuBase<T>::getBacklightBrightness()
{
  return _brightness;
}

// Go to the next menu item from currently active one
template <class T> void LcdMenuBase<T>::setNextActive()
{

  _activeMenuIndex = adjustWrap(_activeMenuIndex, 1, 0, _numMenuItems - 1);

  // Update the display
  updateDisplay();

  // Clear submenu line, in case new menu doesn't print anything.
  _lcd.setCursor(0, 1);
  for (byte i = 0; i < _columns; i++)
  {
    _lcd.print(" ");
  }
}

// Update the display of the LCD with the current menu settings
// This iterates over the menu items, building a menu string by concatenating their display string.
// It also places the selector arrows around the active one.
// It then sends the string to the LCD, keeping the selector arrows centered in the same place.
template <class T> void LcdMenuBase<T>::updateDisplay()
{

  char bufMenu[17];
  char *pBufMenu = &bufMenu[0];
  String menuString = "";
  byte offsetToActive = 0;
  byte offset = 0;

  char scratchBuffer[12];
  // Build the entire menu string
  for (byte i = 0; i < _numMenuItems; i++)
  {
    MenuItem *item = _menuItems[i];
    bool isActive = i == _activeMenuIndex;
    sprintf(scratchBuffer, "%c%s%c", isActive ? '>' : ' ', item->display(), isActive ? '<' : ' ');

    // For the active item remember where it starts in the string and insert selector arrows
    offsetToActive = isActive ? offset : offsetToActive;
    menuString += String(scratchBuffer);
    offset += strlen(scratchBuffer);
  }

  _lcd.setCursor(0, 0);
  _activeRow = 0;
  _activeCol = 0;
  int usableColumns = _columns - 1; // Leave off last one to have distance to tracking indicator

  // Determine where to place the active menu item. (empty space around longest item divided by two).
  int margin = (usableColumns - (_longestDisplay)) / 2;
  int offsetIntoString = offsetToActive - margin;

  // Pad the front if we don't have enough to offset the string to the arrow locations (happens on first item(s))
  while (offsetIntoString < 0)
  {
    *(pBufMenu++) = ' ';
    offsetIntoString++;
  }

  // Display the actual menu string
  while ((pBufMenu < bufMenu + usableColumns) && (offsetIntoString < (int)menuString.length()))
  {
    *(pBufMenu++) = menuString[offsetIntoString++];
  }

  // Pad the end with spaces so the display is cleared when getting to the last item(s).
  while (pBufMenu < bufMenu + _columns)
  {
    *(pBufMenu++) = ' ';
  }
  *(pBufMenu++) = 0;

  printMenu(String(bufMenu));

  setCursor(0, 1);
}

// Print a character at a specific position
template <class T> void LcdMenuBase<T>::printAt(int col, int row, char ch)
{
  _lcd.setCursor(col, row);
  _lcd.printChar(ch);
}

template <class T> uint8_t LcdMenuBase<T>::readButtons()
{
  // return _lcd.readButtons();    // TODO: fix this
  return 0;
}

// Print a string to the LCD at the current cursor position, substituting the special arrows and padding with spaces to the end
template <class T> void LcdMenuBase<T>::printMenu(String line)
{
  if ((_lastDisplay[_activeRow] != line) || (_activeCol != 0))
  {

    _lastDisplay[_activeRow] = line;

    _lcd.setCursor(_activeCol, _activeRow);
    int spaces = _columns - line.length();
    for (unsigned int i = 0; i < line.length(); i++)
    {
      _lcd.printChar(line[i]);
    }

    // Clear the rest of the display
    while (spaces > 0)
    {
      _lcd.print(" ");
      spaces--;
    }
  }
}


