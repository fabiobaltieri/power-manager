#define INA2XX_CONFIG                   0x00
#define INA2XX_SHUNT_VOLTAGE            0x01 /* readonly */
#define INA2XX_BUS_VOLTAGE              0x02 /* readonly */
#define INA2XX_POWER                    0x03 /* readonly */
#define INA2XX_CURRENT                  0x04 /* readonly */
#define INA2XX_CALIBRATION              0x05

#define INA219_CONFIG_DEFAULT		0x399f

#define INA219_CALIBRATION_FACT		40960000
#define INA219_SHUNT_DIV		100
#define INA219_BUS_VOLTAGE_SHIFT	3
#define INA219_BUS_VOLTAGE_LSB		4
#define INA219_POWER_LSB		20
