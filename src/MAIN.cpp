#include <MAIN.h>

MAINmodule::MAINmodule(int CSpinT, int I2CaddressP, int errLED, int busyLED)
    : sensorT(CSpinT), sensorP(Wire, I2CaddressP)
{
    _errLED = errLED;
    _busyLED = busyLED;

    _I2CaddressP = I2CaddressP;
}

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

void MAINmodule::LANsetup(byte mac, IPAddress ip, int port, int csPinLAN)
    : server(port)
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

void MAINmodule::busy()
{
    digitalWriteFast(_busyLED, HIGH);
}

void MAINmodule::notBusy()
{
    digitalWriteFast(_busyLED, LOW);
}

void MAINmodule::startTmeasurement()
{
    if (!Tmeasuring)
    {
        sensorT.startMeasurement();

        Tmeasuring = true;
        _tMeasurementStart = millis();
    }
}

float MAINmodule::readEnvT()
{
    if ((millis() - _tMeasurementStart) > 65)
    {
        envTemperature = sensorT.read();

        Tmeasuring = false;
    }

    return envTemperature;
}

float MAINmodule::readEnvP()
{
    sensorP.readSensor();
    envPressure = sensorP.getPressure_Pa() / 100;

    return envPressure;
}

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

bool MAINmodule::listenForClient()
{
    /*
        This function listens for incoming clients over Ethernet
        Returns 'true' if a client requests a connection and 'false' in any other case
    */
    _client = server.available()
    if(_client)
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
    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            // if you've gotten to the end of the line (received a newline
            // character) and the line is blank, the http request has ended,
            // so you can send a reply
            if (c == '\n' && currentLineIsBlank)
            {
                // send a standard http response header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connection: close"); // the connection will be closed after completion of the response
                client.println("Refresh: 5");        // refresh the page automatically every 5 sec
                client.println();
                client.println("<!DOCTYPE HTML>");
                client.println("<html>");

                client.println(dataString)
                client.println("<br />");
            }
            client.println("</html>");
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