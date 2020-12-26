#pragma once

#include <Arduino.h>
#include "../Configuration_adv.hpp"
#include "DisplayDevice.hpp"

// TODO: Replace with enum inside KeypadDevice namespace
/**
 * @brief The current state of the keypad. Assumes that only a single key can be depressed at any time.
 * 
 */
enum KeypadState
{
    btnNONE = 0,
    btnRIGHT,
    btnUP,
    btnDOWN,
    btnLEFT,
    btnSELECT
};

/**
 * @brief The prototypical interface to the underlying hardware of a KeypadDevice
 * 
 * A new *KeypadDevice should derive and then re-implement the functions in this interface for
 * the specific hardware available. Note that the constructor expects a DisplayDeviceInterface 
 * object according to the DISPLAY_TYPE mapping in DisplayDevice.hpp. The new *KeypadDevice 
 * should then be registered against a specific DISPLAY_TYPE in the preprocessor mapping 
 * below (search for DISPLAY_TYPE).
 */
class KeypadDeviceInterface
{
protected:
    /**
     * @brief Construct the KeypadDevice using the DisplayDeviceInterface if necessary.
     */
    KeypadDeviceInterface(/* DisplayDeviceInterface& display */) {};

    /**
     * @brief Read the keypad hardware, returning the instantaneous raw key state and an analog representation of that state.
     * 
     * @param analogValue An analog representation of the current keypad state.
     * @return byte The instantaneous raw key state reported by the hardware, represented as a KeypadState.
     */
    byte readKeypadState(int16_t & analogValue);
};

/**
 * @brief A null device for headless operation.
 */
class NullKeypadDevice : public KeypadDeviceInterface
{
protected:
    NullKeypadDevice(NullDisplayDevice& /* Not used */);

    byte readKeypadState(int16_t & analogValue);
};

/**
 * @brief The combined LCD & keypad shield for the full-size Arduino form factor with parallel interface.
 * @pre Expects LCD_KEY_SENSE_PIN to be #defined in Configuration_local_*.hpp with the pin number for reading the (analog) keypad state.
 * @see https://electropeak.com/1602-lcd-keypad-shield-for-arduino
 */
class LcdKeypadShieldKeypadDevice : public KeypadDeviceInterface
{
protected:
    LcdKeypadShieldKeypadDevice(LcdKeypadShieldDisplayDevice& /* Not used */);

    byte readKeypadState(int16_t & analogValue);
};

/**
 * @brief The combined LCD & keypad shield for the full-size Arduino form factor with I2C interface
 * Uses the corresponding MCP23008_MCP23017DisplayDevice to access the keys via the I2C interface.
 * @see https://www.adafruit.com/product/716
 */
class MCP23008_MCP23017KeypadDevice : public KeypadDeviceInterface
{
protected:
    MCP23008_MCP23017KeypadDevice(MCP23008_MCP23017DisplayDevice& display);

    byte readKeypadState(int16_t & analogValue);

private:
    MCP23008_MCP23017DisplayDevice& _display;
};

/**
 * @brief A 5 button virtual keypad based around a mini joystick.
 * @pre Expects LCD_KEY_SENSE_X_PIN, LCD_KEY_SENSE_Y_PIN, LCD_KEY_SENSE_PUSH_PIN to be #defined in Configuration_local_*.hpp with the pin numbers for reading the (analog) keypad state.
 * @see https://www.banggood.com/JoyStick-Module-Shield-2_54mm-5-pin-Biaxial-Buttons-Rocker-for-PS2-Joystick-Game-Controller-Sensor-p-1566502.html
 */
class MiniJoystickKeypadDevice : public KeypadDeviceInterface
{
protected:
    MiniJoystickKeypadDevice(SSD1306DisplayDevice& /* Not used */);

    byte readKeypadState(int16_t & analogValue);
};

/**
 * @brief Use DISPLAY_TYPE to map a concrete implementation of the *KeypadDevice to the public _KeypadDevice type
 */
#if DISPLAY_TYPE == DISPLAY_TYPE_NONE
  typedef NullKeypadDevice _KeypadDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD
  typedef LcdKeypadShieldKeypadDevice _KeypadDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23008
  typedef MCP23008_MCP23017KeypadDevice _KeypadDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_KEYPAD_I2C_MCP23017
  typedef MCP23008_MCP23017KeypadDevice _KeypadDevice;
#elif DISPLAY_TYPE == DISPLAY_TYPE_LCD_JOY_I2C_SSD1306
  typedef MiniJoystickKeypadDevice _KeypadDevice;
#endif

/**
 * @brief The full keypad device, including debouncing and key change detection.
 * Inherits from a hardware-specific _KeypadDevice to interface to the actual hardware.
 */
class KeypadDevice : private _KeypadDevice
{
public:
    /**
     * @brief Construct a new Keypad Device object
     * 
     * @param display The current DisplayDevice object. This is needed to support combined display & keypad implementations
     */
    KeypadDevice(DisplayDevice& display);

    /**
     * @brief Return the debounced keypad state
     * 
     * @return byte The keypad state as per KeypadState
     */
    byte currentKey();

    /**
     * @brief Return the raw keypad state from the underlying _KeypadDevice
     * 
     * @return byte The keypad state as per KeypadState
     */
    byte currentState();

    /**
     * @brief Return the raw analog state from the underlying _KeypadDevice
     * 
     * @return int16_t The raw analog keypad state
     */
    int16_t currentAnalogState();

    /**
     * @brief Check whether the key state has changed since the last call.
     * 
     * @param pNewKey   If the key state has chnaged, contains the new key state otherwise remains unchanged
     * @return true     If the key state has changed since the last call
     * @return false    Otherwise
     */
    bool keyChanged(byte* pNewKey); 

private:
    /**
     * @brief Read and debounce the underlying _KeypadDevice object.
     */
    void debounceKeypad();

    static const int32_t DEBOUNCE_PERIOD = 5;      ///< Wait for a key change to settle for at least this long before reporting it (ms)

    byte _currentKey;           ///< The most recent raw keypad state read from the underlying _KeypadDevice
    int16_t _analogKeyValue;    ///< The most recent raw analog state read from the underlying _KeypadDevice

    uint32_t _lastKeyChange;    ///< The time of the most recent state change in _rawKey (ms)
    byte _lastKey;              ///< The most recent state change in _rawKey
    byte _newKey;               ///< The debounced key state
    byte _lastNewKey;           ///< Used to ensure that a new key chnage is only sent once to the application
};


