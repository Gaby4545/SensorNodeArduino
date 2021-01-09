// Header
#include "I2C.h"

namespace SensorNode {
    void I2C::begin() {
        Wire.begin();
    };

    //-- Writing of data --//
    void I2C::writeDataByte(unsigned char address, unsigned char command, unsigned char data) {
        Wire.beginTransmission(address);
        Wire.write(data);
        Wire.endTransmission();
    };

    //-- Reading of data --//
    vector<uint8_t> I2C::readData(uint8_t address, vector<uint8_t> &command, uint8_t responseLenght) {
        vector<uint8_t> data = {};
        
        Wire.beginTransmission(address);
        // Write the command
        for (uint8_t byte : command) Wire.write(byte);
        Wire.endTransmission();

        Wire.requestFrom(address, responseLenght);
        // Receive the bytes
        for (size_t i = 0; i < responseLenght; i++) {
            data.push_back(Wire.read());
        }
        
        return data;
    };
}