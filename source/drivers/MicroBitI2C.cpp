#include "MicroBitConfig.h"
#include "MicroBitI2C.h"
#include "ErrorNo.h"
#include "twi_master.h"
#include "nrf_delay.h"

/**
  * Constructor.
  *
  * Create an instance of MicroBitI2C for I2C communication.
  *
  * @param sda the Pin to be used for SDA
  *
  * @param scl the Pin to be used for SCL
  *
  * @code
  * MicroBitI2C i2c(I2C_SDA0, I2C_SCL0);
  * @endcode
  *
  * @note This class presents a wrapped mbed call to capture failed I2C operations caused by a known silicon bug in the nrf51822.
  * Attempts to automatically reset and restart the I2C hardware if this case is detected.
  *
  * For reference see PAN56 in:
  *
  * https://www.nordicsemi.com/eng/nordic/Products/nRF51822/PAN-nRF51822/24634
  *
  * v2.0 through to v2.4
  */
MicroBitI2C::MicroBitI2C(PinName sda, PinName scl) : I2C(sda,scl)
{
    this->retries = 0;
}

/**
  * Performs a complete read transaction. The bottom bit of the address is forced to 1 to indicate a read.
  *
  * @param address 8-bit I2C slave address [ addr | 1 ]
  *
  * @param data A pointer to a byte buffer used for storing retrieved data.
  *
  * @param length Number of bytes to read.
  *
  * @param repeated if true, stop is not sent at the end. Defaults to false.
  *
  * @return MICROBIT_OK on success, MICROBIT_I2C_ERROR if an unresolved read failure is detected.
  */
int MicroBitI2C::read(int address, char *data, int length, bool repeated)
{
    int result = I2C::read(address,data,length,repeated);

    //0 indicates a success, presume failure
    while(result != 0 && retries < MICROBIT_I2C_MAX_RETRIES)
    {
        _i2c.i2c->EVENTS_ERROR = 0;
        _i2c.i2c->ENABLE       = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
        _i2c.i2c->POWER        = 0;
        nrf_delay_us(5);
        _i2c.i2c->POWER        = 1;
        _i2c.i2c->ENABLE       = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
        twi_master_init_and_clear();
        result = I2C::read(address,data,length,repeated);
        retries++;
    }

    if(result != 0)
        return MICROBIT_I2C_ERROR;

    retries = 0;
    return MICROBIT_OK;
}

/**
  * Performs a complete write transaction. The bottom bit of the address is forced to 0 to indicate a write.
  *
  * @param address 8-bit I2C slave address [ addr | 0 ]
  *
  * @param data A pointer to a byte buffer containing the data to write.
  *
  * @param length Number of bytes to write
  *
  * @param repeated if true, stop is not sent at the end. Defaults to false.
  *
  * @return MICROBIT_OK on success, MICROBIT_I2C_ERROR if an unresolved write failure is detected.
  */
int MicroBitI2C::write(int address, const char *data, int length, bool repeated)
{
    int result = I2C::write(address,data,length,repeated);

    //0 indicates a success, presume failure
    while(result != 0 && retries < MICROBIT_I2C_MAX_RETRIES)
    {
        _i2c.i2c->EVENTS_ERROR = 0;
        _i2c.i2c->ENABLE       = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
        _i2c.i2c->POWER        = 0;
        nrf_delay_us(5);
        _i2c.i2c->POWER        = 1;
        _i2c.i2c->ENABLE       = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;

        twi_master_init_and_clear();
        result = I2C::write(address,data,length,repeated);
        retries++;
    }

    if(result != 0)
        return MICROBIT_I2C_ERROR;

    retries = 0;
    return MICROBIT_OK;
}