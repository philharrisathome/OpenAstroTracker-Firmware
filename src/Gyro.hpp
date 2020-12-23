#pragma once

/**
 * @brief The abstract interface for the Gyro specialized classes. 
 * This class does not have an implementation. Any specialized Gyro classes are expected
 * to derived from it.
 */
class GyroInterface
{
public:
  /**
   * @brief Convenient structure to hold pitch & roll tilt values.
   */
  struct angle_t 
  { 
    float pitchAngle = 0; ///! Pitch angle in degrees
    float rollAngle = 0;  ///! Roll angle in degrees
  };

public:
  // The interface specification - all of the following methods should be implemented by any specilaized class
  
  /**
   * @brief Start the gyro device.
   * 
   */
  void startup();

  /**
   * @brief Shutdown the gyro device.
   * 
   */
  void shutdown();

  /**
   * @brief Return the current pitch & roll tilt values in degrees from the gyro device.
   * 
   * @return angle_t
   * 
   * @retval {pitch,roll} The current pitch & roll tilt values in degrees.
   * @retval {0,0}  If the gyro is unavailable.
   */
  angle_t getCurrentAngles();

  /**
   * @brief Return the current temperature measured by the gyro device
   * 
   * @return float 
   * 
   * @retval temperature The current temperature in degrees C.
   * @retval 0.0 If the gyro is unavailable.
   */
  float getCurrentTemperature();

  /**
   * @brief Returns whether the gyro device is present or not. Often the gyro is externally mounted,
   * and therefore can become disconnected or unplugged.
   * 
   * @return bool
   * 
   * @retval true The gyro is present (and working correctly)
   * @retval false The gyro is not present (or could not be detected)
   */
  bool isPresent();
};

/**
 * @brief A null Gyro device that complies with GyroInterface and always returns safe values.
 */
class GyroNull : public GyroInterface
{
public:
  // Implementation of the interface specification
  void startup() {}   ///< Does nothing
  void shutdown() {}  ///< Does nothing
  angle_t getCurrentAngles() { return angle_t(); }  ///< Always returns {0,0} for pitch & roll
  float getCurrentTemperature() { return 0; }       ///< Always returns 0.0
  bool isPresent() { return false; }                ///< Always returns false (gyro is not present)
};

/**
 * @brief A Gyro device implementation for the MPU-6050 MEMS gyro that complies with GyroInterface.
 * 
 * Provides tilt, roll, and temperature measurements using the MPU-6050 MEMS gyro.
 * 
 * @see See: https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/
 * @see Datasheet: https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
 * @see Register descriptions: https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
 * */
class GyroMPU6050 : public GyroInterface
{
public:
  // Implementation of the interface specification
  void startup();
  void shutdown();
  angle_t getCurrentAngles();
  float getCurrentTemperature();
  bool isPresent();

private:

  /**
   * @brief MPU6050 constants
   * 
   */
  enum {
    MPU6050_I2C_ADDR = 0x68,          ///< I2C address of the MPU6050 accelerometer

    // Register addresses
    MPU6050_REG_CONFIG = 0x1A,        ///< Internal address of the MPU6050 CONFIG register
    MPU6050_REG_ACCEL_XOUT_H = 0x3B,  ///< Internal address of the MPU6050 ACCEL_XOUT_H register
    MPU6050_REG_TEMP_OUT_H = 0x41,    ///< Internal address of the MPU6050 TEMP_OUT_H register
    MPU6050_REG_PWR_MGMT_1 = 0x6B,    ///< Internal address of the MPU6050 PWR_MGMT_1 register
    MPU6050_REG_WHO_AM_I = 0x75       ///< Internal address of the MPU6050 WHO_AM_I register
  };

  bool _isPresent = false;  ///< True if gyro correctly detected on startup, false otherwise
};

/**
 * @brief Use USE_GYRO_LEVEL to map a concrete implementation of the GyroInterface to the public Gyro type
 * 
 */
#if USE_GYRO_LEVEL == 1
  typedef GyroMPU6050 Gyro;
#else
  typedef GyroNull Gyro;
#endif

