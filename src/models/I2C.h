// Dependencies
#include <Wire.h>

#include <vector>
using namespace std;

namespace SensorNode {
    class I2C {
        public:
        /// Empty constructor
        I2C() {}

        static void begin();

        //-- Writing of data --//
        /** 
         * Sends data to an I2C device 
         * @param address The address of the I2C device on the I2C Bus
         * @param command The address of the register in the I2C device
         * @param data The the data to be sent to the I2C device
        */
        static void writeDataByte(uint8_t address, uint8_t command, uint8_t data);

        static void writeData();

        //-- Reading of data --//
        /** 
         * Retreives an unsigned short from an I2C device
         * @param address The address of the I2C device on the I2C Bus
         * @param command The address of the register in the I2C device
         * @return The requested bytes
        */
        static vector<uint8_t> readData(uint8_t address, vector<uint8_t> &command, uint8_t responseLenght);
    };
}