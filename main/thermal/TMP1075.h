#ifndef TMP1075_H_
#define TMP1075_H_

#define TMP1075_I2CADDR_U7 0x4A  ///< TMP1075 i2c address
#define TMP1075_I2CADDR_U8 0x4B  ///< TMP1075 i2c address
#define TMP1075_TEMP_REG 0x00         ///< Temperature register
#define TMP1075_CONFIG_REG 0x01       ///< Configuration register
#define TMP1075_LOW_LIMIT 0x02        ///< Low limit register
#define TMP1075_HIGH_LIMIT 0x03       ///< High limit register
#define TMP1075_DEVICE_ID 0x0F        ///< Device ID register

#define WEIGHTED_U7 0.35
#define WEIGHTED_U8 0.65    //This more close to the chips

bool TMP1075_installed(int);
uint8_t TMP1075_read_temperature(int);
float TMP1075_read_temperature_weighted();
esp_err_t TMP1075_init(void);

#endif /* TMP1075_H_ */
