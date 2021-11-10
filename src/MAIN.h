#include <Arduino.h>
#include <SPI.h>
#include <i2c_t3.h>
#include <BME280_no_delay.h>
#include <max31865_no_delay.h>
#include <Ethernet.h>

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
    EthernetClient _client;
    

public:
    MAINmodule(int CSpinT, int I2CaddressP, int errLED = 5, int busyLED = 6);
    void config(int mode = 1);
    void LANsetup(byte mac[], IPAddress ip, int port, int csPinLAN = 15);
    bool faultDetection();
    void busy();
    void notBusy();
    void errorBlink(bool error);
    void startTmeasurement();
    float readEnvT();
    float readEnvP();
    bool listenForClient();
    void printDataLAN(String dataString)
    float envTemperature;
    float envPressure;
    bool Tmeasuring;
    bool fault;
    BME280 sensorP;
    MAX31865 sensorT;
    EthernetServer server;
    byte mac[6];
    IPAddress ip;

};



