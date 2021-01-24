#include "inc/Globals.hpp"
#include "../Configuration.hpp"
#include "Utility.hpp"
#include "DisplayDevice.hpp"
#include "KeypadDevice.hpp"

/**
 * @brief Construct the NullKeypadDevice using the NullDisplayDevice.
 */
NullKeypadDevice::NullKeypadDevice(NullDisplayDevice& /* Not used */)
{
}

/**
 * @brief The NullKeypadDevice always states that no keys are being pressed.
 * 
 * @param analogValue 0, always
 * @return byte btnNONE, always
 */
byte NullKeypadDevice::readKeypadState(int16_t & analogValue)
{
    analogValue = 0;

    return btnNONE;
}

/**
 * @brief Construct the LcdKeypadShieldKeypadDevice using the LcdKeypadShieldDisplayDevice.
 * Initialise the LCD_KEY_SENSE_PIN for use as an analog input.
 */
LcdKeypadShieldKeypadDevice::LcdKeypadShieldKeypadDevice(LcdKeypadShieldDisplayDevice& /* Not used */)
{
    // Initialize keypad
    pinMode(LCD_KEY_SENSE_PIN, INPUT);
}

/**
 * @brief Read the keypad hardware.
 * 
 * @param analogValue   The raw analog value read at the LCD_KEY_SENSE_PIN pin.
 * @return byte         The translation of the analog value to the actual key press.
 */
byte LcdKeypadShieldKeypadDevice::readKeypadState(int16_t & analogValue)
{
    analogValue = analogRead(LCD_KEY_SENSE_PIN);

    // See "How to read the keys?" at https://electropeak.com/learn/using-1602-character-lcd-keypad-shield-arduino/
    byte keyState = btnNONE;
    if (analogValue > 1000) keyState = btnNONE;
    else if (analogValue < 50)   keyState = btnRIGHT;
    else if (analogValue < 240)  keyState = btnUP;
    else if (analogValue < 400)  keyState = btnDOWN;
    else if (analogValue < 600)  keyState = btnLEFT;
    else if (analogValue < 920)  keyState = btnSELECT;

    return keyState;
 }

/**
 * @brief Construct the LcdKeypadShieldKeypadDevice using the MCP23008_MCP23017DisplayDevice.
 */
MCP23008_MCP23017KeypadDevice::MCP23008_MCP23017KeypadDevice(MCP23008_MCP23017DisplayDevice& display)
: _display(display)
{
    // Initialize keypad
    pinMode(LCD_KEY_SENSE_PIN, INPUT);
}

/**
 * @brief Read the keypad hardware, remapping from LiquidTWI2 librray values to our values.
 * 
 * @param analogValue   A scaled version of the value returned by LiquidTWI2::readButtons().
 * @return byte         The translation of the LiquidTWI2::readButtons() value to the actual key press.
 */
byte MCP23008_MCP23017KeypadDevice::readKeypadState(int16_t & analogValue)
{
    uint8_t buttons = _display.readButtons();
    analogValue = buttons * 50; // Arbitrary scaling

    byte keyState = btnNONE;
    if (buttons)
    {
        if (buttons & BUTTON_UP) keyState = btnUP;
        if (buttons & BUTTON_DOWN) keyState = btnDOWN;  
        if (buttons & BUTTON_LEFT) keyState = btnLEFT;
        if (buttons & BUTTON_RIGHT) keyState = btnRIGHT;
        if (buttons & BUTTON_SELECT) keyState = btnSELECT;
    }

    return keyState;
}

// These pins allocations may not be available in all hardware configurations, so set them to invalid but safe values to allow compilation
#if !defined(LCD_KEY_SENSE_X_PIN) || !defined(LCD_KEY_SENSE_Y_PIN) || !defined(LCD_KEY_SENSE_PUSH_PIN)
  #define LCD_KEY_SENSE_X_PIN (-1) 
  #define LCD_KEY_SENSE_Y_PIN (-1) 
  #define LCD_KEY_SENSE_PUSH_PIN (-1) 
#endif

/**
 * @brief Construct the MiniJoystickKeypadDevice using the SSD1306DisplayDevice.
 * Initialise LCD_KEY_SENSE_X_PIN, LCD_KEY_SENSE_Y_PIN for use as an analog input.
 * Initialise LCD_KEY_SENSE_PUSH_PIN for use as a digital input (active low).
 */
MiniJoystickKeypadDevice::MiniJoystickKeypadDevice(SSD1306DisplayDevice& /* Not used */)
{
    // Initialize keypad
    pinMode(LCD_KEY_SENSE_X_PIN, INPUT);
    pinMode(LCD_KEY_SENSE_Y_PIN, INPUT);
    pinMode(LCD_KEY_SENSE_PUSH_PIN, INPUT_PULLUP);    
}

/**
 * @brief Read the keypad hardware.
 * 
 * @param analogValue   The raw analog value read at the LCD_KEY_SENSE_X_PIN pin.
 * @return byte         The translation of the analog value to the actual key press.
 */
byte MiniJoystickKeypadDevice::readKeypadState(int16_t & analogValue)
{
    uint16_t x(analogRead(LCD_KEY_SENSE_X_PIN));
    uint16_t y(analogRead(LCD_KEY_SENSE_Y_PIN));
    bool push(digitalRead(LCD_KEY_SENSE_PUSH_PIN) == LOW);  // Active low
    analogValue = x;

    // Assumes analogReadResolution(12) (the default)
    int16_t const MIDSCALE = 4096 / 2;
    int16_t const DEADBAND = 500;

    byte keyState = btnNONE;
    if (x > (MIDSCALE + DEADBAND)) keyState = btnRIGHT;
    if (x < (MIDSCALE - DEADBAND)) keyState = btnLEFT;
    if (y > (MIDSCALE + DEADBAND)) keyState = btnUP;
    if (y < (MIDSCALE - DEADBAND)) keyState = btnDOWN;
    if (push) keyState = btnSELECT;

    return keyState;
}

/**
 * @brief Construct the KeypadDevice passing the (aliased) DisplayDevice to the base class hardware interface.
 * 
 * @param display The current DisplayDevice object. This is needed to support combined display & keypad implementations
 */
KeypadDevice::KeypadDevice(DisplayDevice& display) 
: _KeypadDevice(display)
, _currentKey(-1), _analogKeyValue(0)
, _lastKeyChange(0), _lastKey(-2), _newKey(-1), _lastNewKey(-2)
{
}

byte KeypadDevice::currentKey() 
{
    debounceKeypad();
    return _newKey;
}

byte KeypadDevice::currentState() 
{
    debounceKeypad();
    return _currentKey;
}

int16_t KeypadDevice::currentAnalogState() 
{
    debounceKeypad();
    return _analogKeyValue;
}

bool KeypadDevice::keyChanged(byte* pNewKey) 
{
    debounceKeypad();
    if (_newKey != _lastNewKey) 
    {
      *pNewKey = _newKey;
      _lastNewKey = _newKey;
      return true;
    }
    return false;
}

void KeypadDevice::debounceKeypad()
{
    _currentKey = readKeypadState(_analogKeyValue);

    if (_currentKey != _lastKey) 
    {
        // The state of the hwardware key has changed - start the debounce timer
        _lastKey = _currentKey;
        _lastKeyChange = millis();
    }
    else 
    {
        // If the keys haven't changed in DEBOUNCE_PERIOD, commit the change to the new keys.
        if (millis() - _lastKeyChange > DEBOUNCE_PERIOD) 
        {
            _newKey = _currentKey;
        }
    }
}

