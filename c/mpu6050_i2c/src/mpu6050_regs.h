/// @file mpu6050.h
/// @author Johannes Ylinen
/// @brief Register addresses and flags for MPU6050.
/// @see https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf

#define MPU6050_REG_CONFIG 0x1A

#define MPU6050_CONFIG_DLPF_CFG_260HZ 0
#define MPU6050_CONFIG_DLPF_CFG_184HZ 1
#define MPU6050_CONFIG_DLPF_CFG_94HZ  2
#define MPU6050_CONFIG_DLPF_CFG_44HZ  3
#define MPU6050_CONFIG_DLPF_CFG_21HZ  4
#define MPU6050_CONFIG_DLPF_CFG_10HZ  5
#define MPU6050_CONFIG_DLPF_CFG_5HZ   6

#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_PWR_MGMT_1_DEVICE_RESET (1u << 7)
#define MPU6050_PWR_MGMT_1_SLEEP (1u << 6)
#define MPU6050_PWR_MGMT_1_CYCLE (1u << 5)
#define MPU6050_PWR_MGMT_1_TEMP_DIS (1u << 3)

#define MPU6050_REG_PWR_MGMT_2 0x6C
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_MASK (3u << 6) // Clear field
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_0 0u        // 1.25 Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_1 (1u << 6) // 5 Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_2 (1u << 7) // 20 Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_3 (3u << 6) // 40 Hz

// Put accelerometer to standby
#define MPU6050_PWR_MGMT_2_STBY_XA (1u << 5) // Bit 5
#define MPU6050_PWR_MGMT_2_STBY_YA (1u << 4) // Bit 4
#define MPU6050_PWR_MGMT_2_STBY_ZA (1u << 3) // Bit 3

// Put gyro to standby
#define MPU6050_PWR_MGMT_2_STBY_XG (1u << 2) // Bit 2
#define MPU6050_PWR_MGMT_2_STBY_YG (1u << 1) // Bit 1
#define MPU6050_PWR_MGMT_2_STBY_ZG (1u << 0) // Bit 0

#define MPU6050_REG_WHO_AM_I   0x75

#define MPU6050_REG_ACCEL_XOUT 0x3B
#define MPU6050_REG_ACCEL_YOUT 0x3D
#define MPU6050_REG_ACCEL_ZOUT 0x3F

#define MPU6050_REG_ACCEL_XOUT_H MPU6050_REG_ACCEL_XOUT
#define MPU6050_REG_ACCEL_XOUT_L 0x3C
#define MPU6050_REG_ACCEL_YOUT_H MPU6050_REG_ACCEL_YOUT
#define MPU6050_REG_ACCEL_YOUT_L 0x3E
#define MPU6050_REG_ACCEL_ZOUT_H MPU6050_REG_ACCEL_ZOUT
#define MPU6050_REG_ACCEL_ZOUT_L 0x40
