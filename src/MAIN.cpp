#include <MAIN.h>

/*
    Constructor for the main module. Is used to define the CS pin of the Pt100 module and I²C address of the pressure sensor of the main module

    Inputs:     CSpinT          Pin number of the CS pin of the MAX31865 module (Default pin: 24)
                I2CaddressP     I²C address of the BME280 pressure sensor (Default address: 0x76)
                port            Port nummer of the ethernet mdoule. set to 0 if not used
                errLED          Pin number of the "error" indicator LED
                busyLED         Pin number of the "busy" indicator LED
*/
MAINmodule::MAINmodule(int CSpinT, int I2CaddressP, int port, int errLED, int busyLED)
    : sensorT(CSpinT), sensorP(Wire, I2CaddressP), server(port)
{
    _errLED = errLED;
    _busyLED = busyLED;

    _I2CaddressP = I2CaddressP;

    _port = port;
}

/*
    Initialize the main module and configure the measurement mode for the enviromental sensors

    Inputs:     mode            Enable the environmental sensor if 1
*/
void MAINmodule::config(int mode)
{
    pinMode(_errLED, OUTPUT);
    pinMode(_busyLED, OUTPUT);

    if (mode == 1)
    {
        sensorT.config(430, 1);
        if (sensorP.begin() != 1)
        {
            Serial.println("Der Umgebungsdrucksensor konnte nich initalisiert werden. Verkableung überprüfen");
            fault = true;
        }
    }
    else if (mode == 0)
    {
    }
}


/*
    Initialize the Ethernet connection for use of the module with LAN

    Inputs:     mac         Mac address of the LAN server
                ip          IP address of the LAN server
                csPinLAN    CS pin used for the LAN module (Default pin: 15)
*/
void MAINmodule::LANsetup(byte *mac, IPAddress ip, int csPinLAN)
{
    if (_port == 0)
    {
        Serial.println("Kein Ethernet-Port definiert. Definiere Ethernet-Port über den constructor für MAINmodule");
    }
    else
    {
        Ethernet.init(csPinLAN);
        Ethernet.begin(mac, ip);

        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true)
            {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("Ethernet cable is not connected.");
        }

        server.begin();
        Serial.print("server is at ");
        Serial.println(Ethernet.localIP());
    }
}


/*
    Turn on "busy" LED
*/
void MAINmodule::busy()
{
    digitalWriteFast(_busyLED, HIGH);
}


/*
    Turn off "busy" LED
*/
void MAINmodule::notBusy()
{
    digitalWriteFast(_busyLED, LOW);
}


/*
    Start the temperature with the MAX31865 temperature module
*/
void MAINmodule::startTmeasurement()
{
    if (!Tmeasuring)
    {
        sensorT.startMeasurement();

        Tmeasuring = true;
        _tMeasurementStart = millis();
    }
}


/*
    Read the temperature from the MAX31865
*/
float MAINmodule::readEnvT()
{
    if ((millis() - _tMeasurementStart) > 65)
    {
        envTemperature = sensorT.read();

        Tmeasuring = false;
    }

    return envTemperature;
}


/*
    Read the prressure from the BME280 module
*/
float MAINmodule::readEnvP()
{
    sensorP.readSensor();
    envPressure = sensorP.getPressure_Pa() / 100;

    return envPressure;
}

/*
    Detect possible faults and print out error messages
*/
bool MAINmodule::faultDetection()
{
    if (sensorP.begin() != 1)
    {
        Serial.println("Der Umgebungsdrucksensor konnte nich initalisiert werden. Verkableung überprüfen");
        fault = true;
    }
    else
    {
        fault = false;
    }
}

/*
    Blink the error led if an error ocurred

    Inputs:        error        Boolean for if an error occurred. 'true' to blink "error" LED
*/
void MAINmodule::errorBlink(bool error)
{
    if (error && (millis() - _tErrLED >= 1000))
    {
        if (_errLEDon)
        {
            digitalWriteFast(_errLED, LOW);
            _tErrLED = millis();
            _errLEDon = false;
        }
        else if (!_errLEDon)
        {
            digitalWriteFast(_errLED, HIGH);
            _tErrLED = millis();
            _errLEDon = true;
        }
    }
    else if (!error)
    {
        digitalWriteFast(_errLED, LOW);
    }
}

bool MAINmodule::clientConnected()
{
    /*
        This function listens for incoming clients over Ethernet
        Returns 'true' if a client requests a connection and 'false' in any other case
    */
    _client = server.available();
    if (_client)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MAINmodule::printDataLAN(String dataString)
{
    /*
        This function sends a data string to the connected client via Ethernet
    */
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (_client.connected())
    {
        if (_client.available())
        {
            char c = _client.read();
            Serial.print(c);
            // if you've gotten to the end of the line (received a newline
            // character) and the line is blank, the http request has ended,
            // so you can send a reply
            if (c == '\n' && currentLineIsBlank)
            {
                // send a standard http response header
                
                _client.println(dataString);
                break;
            }
            if (c == '\n')
            {
                // you're starting a new line
                currentLineIsBlank = true;
            }
            else if (c != '\r')
            {
                // you've gotten a character on the current line
                currentLineIsBlank = false;
            }
        }
    }
    delay(1);
    _client.stop();
}