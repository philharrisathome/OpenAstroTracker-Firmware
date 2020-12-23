#include "../Configuration_adv.hpp"
#include "Utility.hpp"
#include "Gyro.hpp"

#include <Wire.h> // I2C communication library

/**
 * @brief Start the gyro device.
 * 
 * Starts up the MPU-6050 device.
 * Reads the WHO_AM_I register to verify if the device is present.
 * Wakes device from power-down.
 * Sets accelerometers to minimum bandwidth to reduce measurement noise.
 */
void GyroMPU6050::startup()
{
    // Initialize interface to the MPU6050
    LOGV1(DEBUG_INFO, F("GYRO:: Starting"));
    Wire.begin();

    // Execute 1 byte read from MPU6050_REG_WHO_AM_I
    // This is a read-only register which should have the value 0x68
    Wire.beginTransmission(MPU6050_I2C_ADDR);
    Wire.write(MPU6050_REG_WHO_AM_I);
    Wire.endTransmission(true);
    Wire.requestFrom(MPU6050_I2C_ADDR, 1, 1);
    byte id = (Wire.read() >> 1) & 0x3F;    
    _isPresent = (id == 0x34);
    if (!_isPresent) {
        LOGV1(DEBUG_INFO, F("GYRO:: Not found!"));
        return;
    }

    // Execute 1 byte write to MPU6050_REG_PWR_MGMT_1
    Wire.beginTransmission(MPU6050_I2C_ADDR);
    Wire.write(MPU6050_REG_PWR_MGMT_1);
    Wire.write(0);      // Disable sleep, 8 MHz clock
    Wire.endTransmission(true);

    // Execute 1 byte write to MPU6050_REG_PWR_MGMT_1
    Wire.beginTransmission(MPU6050_I2C_ADDR);
    Wire.write(MPU6050_REG_CONFIG);
    Wire.write(6);      // 5Hz bandwidth (lowest) for smoothing
    Wire.endTransmission(true);

    LOGV1(DEBUG_INFO, F("GYRO:: Started"));
}

/**
 * @brief Shuts down the MPU-6050 device.
 * 
 * Currently does nothing.
 */
void GyroMPU6050::shutdown()
{
    LOGV1(DEBUG_INFO, F("GYRO: Shutdown"));
    // Nothing to do
}

/**
 * @brief Returns roll & tilt angles from MPU-6050 device in angle_t object in degrees.
 * 
 * If MPU-6050 is not found then returns {0,0}.
 * 
 * @return GyroMPU6050::angle_t 
 * 
 * @retval {pitch,roll} The current pitch & roll tilt values in degrees.
 * @retval {0,0}  If the gyro is unavailable.
 */
GyroMPU6050::angle_t GyroMPU6050::getCurrentAngles()
{
    const int windowSize = 16;
    // Read the accelerometer data
    struct angle_t result;
    result.pitchAngle = 0;
    result.rollAngle = 0;
    if (!_isPresent)
        return result;     // Gyro is not available

    for (int i = 0; i < windowSize; i++)
    {
        // Execute 6 byte read from MPU6050_REG_WHO_AM_I
        Wire.beginTransmission(MPU6050_I2C_ADDR);
        Wire.write(MPU6050_REG_ACCEL_XOUT_H); 
        Wire.endTransmission(false);
        Wire.requestFrom(MPU6050_I2C_ADDR, 6, 1);  // Read 6 registers total, each axis value is stored in 2 registers
        int16_t AcX = Wire.read() << 8 | Wire.read(); // X-axis value
        int16_t AcY = Wire.read() << 8 | Wire.read(); // Y-axis value
        int16_t AcZ = Wire.read() << 8 | Wire.read(); // Z-axis value

        // Calculating the Pitch angle (rotation around Y-axis)
        result.pitchAngle += ((atan(-1 * AcX / sqrt(pow(AcY, 2) + pow(AcZ, 2))) * 180.0 / PI) * 2.0) / 2.0;
        // Calculating the Roll angle (rotation around X-axis)
        result.rollAngle += ((atan(-1 * AcY / sqrt(pow(AcX, 2) + pow(AcZ, 2))) * 180.0 / PI) * 2.0) / 2.0;

        delay(10);  // Decorrelate measurements
    }

    result.pitchAngle /= windowSize;
    result.rollAngle /= windowSize;
#if GYRO_AXIS_SWAP == 1
    float temp = result.pitchAngle;
    result.pitchAngle = result.rollAngle;
    result.rollAngle = temp;
#endif
    return result;
}

/**
 * @brief Returns MPU-6050 device temperature in degree C.
 * 
 * If MPU-6050 is not found then returns 0 (C).
 * 
 * @return float 
 * 
 * @retval temperature The current temperature in degrees C.
 * @retval 0.0 If the gyro is unavailable.
 */
float GyroMPU6050::getCurrentTemperature()
{
    if (!_isPresent)
        return 0;     // Gyro is not available

    // Execute 2 byte read from MPU6050_REG_TEMP_OUT_H
    Wire.beginTransmission(MPU6050_I2C_ADDR);
    Wire.write(MPU6050_REG_TEMP_OUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_I2C_ADDR, 2, 1);        // Read 2 registers total, the temperature value is stored in 2 registers
    int16_t tempValue = Wire.read() << 8 | Wire.read(); // Raw Temperature value
    
    // Calculating the actual temperature value
    float result = float(tempValue) / 340 + 36.53;
    return result;
}

/**
 * @brief Returns whether the gyro device is present or not. Often the gyro is externally mounted,
 * and therefore can become disconnected or unplugged.
 * 
 * @return bool
 * 
 * @retval true The gyro is present (and working correctly)
 * @retval false The gyro is not present (or could not be detected)
 */
bool GyroMPU6050::isPresent()
{
    return _isPresent;
}

