#include "inc/Globals.hpp"
#include "../Configuration.hpp"
#include "Utility.hpp"
#include "EPROMStore.hpp"
#include "LcdMenu.hpp"

#if DISPLAY_TYPE != DISPLAY_TYPE_NONE

// Class that drives the LCD screen with a menu
// You add a string and an id item and this class handles the display and navigation
// Create a new menu, using the given number of LCD display columns and rows

FullLcdMenu::FullLcdMenu(int maxItems)
: _maxItems(maxItems), _numMenuItems(0),
  _activeMenuIndex(0), _longestDisplay(0),
  _activeRow(-1), _activeCol(-1),
  _display(), _keypad(_display)
{
  for(int i(0); i<DisplayDevice::NUM_ROWS; i++)
    _lastDisplay[i] = "";
  _menuItems = new MenuItem *[_maxItems];
}

void FullLcdMenu::startup()
{
  LOGV1(DEBUG_INFO, F("LcdMenu:: startup"));

  _brightness = EEPROMStore::getBrightness();
  LOGV2(DEBUG_INFO, F("LCD: Brightness from EEPROM is %d"), _brightness);
  setBacklightBrightness(_brightness, false);
}

// Find a menu item by its ID
MenuItem *FullLcdMenu::findById(byte id)
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
void FullLcdMenu::addItem(const char *disp, byte id)
{
  _menuItems[_numMenuItems++] = new MenuItem(disp, id);
  _longestDisplay = max((size_t)_longestDisplay, strlen(disp));
}

// Get the currently active item ID
byte FullLcdMenu::getActive()
{
  return _menuItems[_activeMenuIndex]->id();
}

// Set the active menu item
void FullLcdMenu::setActive(byte id)
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
void FullLcdMenu::setCursor(byte col, byte row)
{
  _activeRow = row;
  _activeCol = col;
}

// Pass thru utility function
void FullLcdMenu::clear()
{
  _display.clear();
}

// Set the brightness of the backlight
void FullLcdMenu::setBacklightBrightness(int level, bool persist)
{
  _brightness = level;
  _display.setContrast(_brightness);
  if (persist)
  {
    LOGV2(DEBUG_INFO, F("LCD: Saving %d as brightness"), _brightness);
    EEPROMStore::storeBrightness(_brightness);
  }
}

// Get the current brightness
int FullLcdMenu::getBacklightBrightness()
{
  return _brightness;
}

// Go to the next menu item from currently active one
void FullLcdMenu::setNextActive()
{

  _activeMenuIndex = adjustWrap(_activeMenuIndex, 1, 0, _numMenuItems - 1);

  // Update the display
  updateDisplay();

  // Clear submenu line, in case new menu doesn't print anything.
  _display.setCursor(0, 1);
  for (byte i = 0; i < DisplayDevice::NUM_COLUMNS ; i++)
  {
    _display.printChar(' ');
  }
}

// Update the display of the LCD with the current menu settings
// This iterates over the menu items, building a menu string by concatenating their display string.
// It also places the selector arrows around the active one.
// It then sends the string to the LCD, keeping the selector arrows centered in the same place.
void FullLcdMenu::updateDisplay()
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

  _display.setCursor(0, 0);
  _activeRow = 0;
  _activeCol = 0;
  int usableColumns = DisplayDevice::NUM_COLUMNS - 1; // Leave off last one to have distance to tracking indicator

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
  while (pBufMenu < bufMenu + DisplayDevice::NUM_COLUMNS)
  {
    *(pBufMenu++) = ' ';
  }
  *(pBufMenu++) = 0;

  printMenu(String(bufMenu));

  setCursor(0, 1);
}

// Print a character at a specific position
void FullLcdMenu::printAt(int col, int row, char ch)
{
  _display.setCursor(col, row);
  _display.printChar(ch);
}

uint8_t FullLcdMenu::readButtons()
{
  // return _display.readButtons();    // TODO: fix this
  return 0;
}

// Print a string to the LCD at the current cursor position, substituting the special arrows and padding with spaces to the end
void FullLcdMenu::printMenu(String line)
{
  if ((_lastDisplay[_activeRow] != line) || (_activeCol != 0))
  {

    _lastDisplay[_activeRow] = line;

    _display.setCursor(_activeCol, _activeRow);
    int spaces = DisplayDevice::NUM_COLUMNS - line.length();
    for (unsigned int i = 0; i < line.length(); i++)
    {
      _display.printChar(line[i]);
    }

    // Clear the rest of the display
    while (spaces > 0)
    {
      _display.printChar(' ');
      spaces--;
    }
  }
}


#endif

