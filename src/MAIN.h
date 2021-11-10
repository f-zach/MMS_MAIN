#include <Arduino.h>
#include <SPI.h>
#include <i2c_t3.h>
#include <BME280_no_delay.h>
#include <max31865_no_delay.h>

class MAINmodule
{
private:
    int _errLED; 
    int _busyLED; 
    int _CSpinT;
    int _I2CaddressP;
    long _tMeasurementStart;
    long _tErrLED = 1000;
    bool _errLEDon = false;
    bool _arrayConfigured = false;

public:
    MAINmodule(int CSpinT, int I2CaddressP, int errLED = 5, int busyLED = 6);
    void config(int amountSensorValues, int mode = 1);
    bool faultDetection();
    void busy();
    void notBusy();
    void errorBlink(bool error);
    void startTmeasurement();
    String makeDataString(float dataArray[], String seperator = "\t");
    float readEnvT();
    float readEnvP();
    float envTemperature;
    float envPressure;
    float SensorData[]
    bool Tmeasuring;
    bool fault;
    String dataString
    BME280 sensorP;
    MAX31865 sensorT;

};



