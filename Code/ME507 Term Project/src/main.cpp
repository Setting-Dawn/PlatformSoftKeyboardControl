
#include <Arduino.h>
#include "PrintStream.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ADC128D818.h"
#include "CD74HC4067SM.h"
#include "PCA9956.h"

// LED pin
const uint8_t LED_PIN = 2;

// Multiplexer control pins
const uint8_t s0_PIN = 26;
const uint8_t s1_PIN = 25;
const uint8_t s2_PIN = 33;
const uint8_t s3_PIN = 32;
const uint8_t MultiEnable_PIN = 35;

// ADC I2C Addresses
const uint8_t ADC_1ADDRESS = 0x1D;
const uint8_t ADC_2ADDRESS = 0x1F;

// PCA9956BTWY Addresses
const uint8_t PCA9956_ADDRESS = 0x01;

// Motor control pins
const uint8_t FAULT_PIN = 4;
const uint8_t NSLEEP_PIN = 2;
const uint8_t MOTOR_X_1 = 31;
const uint8_t MOTOR_X_2 = 30;
const uint8_t MOTOR_Y_1 = 28;
const uint8_t MOTOR_Y_2 = 27;

// Assign Wifi access
const char* ssid = "Soft Keyboard";   // SSID, network name seen on LAN lists
const char* password = "Access Code";   // ESP32 WiFi password (min. 8 characters)

/* Put IP Address details */
IPAddress local_ip (192, 168, 5, 1); // Address of ESP32 on its own network
IPAddress gateway (192, 168, 5, 1);  // The ESP32 acts as its own gateway
IPAddress subnet (255, 255, 255, 0); // Network mask; just leave this as is

/** @brief   The web server object for this project.
 *  @details This server is responsible for responding to HTTP requests from
 *           other computers, replying with useful information.
 *
 *           It's kind of clumsy to have this object as a global, but that's
 *           the way Arduino keeps things simple to program, without the user
 *           having to write custom classes or other intermediate-level 
 *           structures. 
*/
WebServer server (80);

/*! @brief function to cycle the built-in LED in a set patern to aid debugging
* @details Blinks a pre-set desired morse code message to crashes externally diagnosable.
*
* @param p_params unused void pointer
*/ 
void heartbeat(void* p_params) {
    digitalWrite (LED_PIN, LOW);

    // define base dot length as 225ms
    const uint8_t sPulse = 225;

    // dots and dashes for "42" in morse
    const uint8_t pips[11] = {1,1,1,1,3,0,1,1,3,1,1};
    uint8_t n = 0;

    for (;;) {
        // Check if the message has been run, if so: delay before looping
        if (n == 11) {
            n = 0;
            digitalWrite (LED_PIN, LOW);
            vTaskDelay(sPulse*6);
            continue;
        }
        // If there should not be a dot or dash, turn off for a short pulse
        else if (pips[n]==0) {
            digitalWrite (LED_PIN, LOW);
            vTaskDelay(sPulse);
        }
        // Otherwise, turn the LED on for the appropriate length of time
        else {
            digitalWrite (LED_PIN, HIGH);
            vTaskDelay(sPulse*pips[n]);
        }
        
        // Write the LED to LOW for a short pulse between each increment
        digitalWrite (LED_PIN, LOW);
        vTaskDelay(sPulse);
        n++;
    }
}

void task_ReadMaterial(void* p_params) {
    ADC128D818 ADC_1 (ADC_1ADDRESS);
    ADC128D818 ADC_2 (ADC_2ADDRESS);
    CD74HC4067SM Multiplex (s0_PIN,s1_PIN,s2_PIN,s3_PIN,MultiEnable_PIN);
    PCA9956 CurrCtrl (&Wire);
    CurrCtrl.init(PCA9956_ADDRESS,0xFF); // Initialize current control address and max brightnes
}

/** @brief   Get the WiFi running so we can serve some web pages.
 */
void setup_wifi(void) {
    Serial << "Setting up WiFi access point...";
    WiFi.mode (WIFI_AP);
    WiFi.softAPConfig (local_ip, gateway, subnet);
    WiFi.softAP (ssid, password);
    Serial << "done." << endl;
}

/** @brief   Put a web page header into an HTML string. 
 *  @details This header may be modified if the developer wants some actual
 *           @a style for her or his web page. It is intended to be a common
 *           header (and stylle) for each of the pages served by this server.
 *  @param   a_string A reference to a string to which the header is added; the
 *           string must have been created in each function that calls this one
 *  @param   page_title The title of the page
*/
void HTML_header (String& a_string, const char* page_title)
{
    a_string += "<!DOCTYPE html> <html>\n";
    a_string += "<head><meta name=\"viewport\" content=\"width=device-width,";
    a_string += " initial-scale=1.0, user-scalable=no\">\n<title> ";
    a_string += page_title;
    a_string += "</title>\n";
    a_string += "<style>html { font-family: Helvetica; display: inline-block;";
    a_string += " margin: 0px auto; text-align: center;}\n";
    a_string += "body{margin-top: 50px;} h1 {color: #4444AA;margin: 50px auto 30px;}\n";
    a_string += "p {font-size: 24px;color: #222222;margin-bottom: 10px;}\n";
    a_string += "</style>\n</head>\n";
}


/** @brief   Callback function that responds to HTTP requests without a subpage
 *           name.
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           (the insecure Web port) with a request for the main web page, this
 *           callback function is run. It sends the main web page's text to the
 *           requesting machine.
 */
void handle_DocumentRoot ()
{
    Serial << "HTTP request from client #" << server.client () << endl;

    String a_str;
    HTML_header (a_str, "ESP32 Web Server Test");
    a_str += "<body>\n<div id=\"webpage\">\n";
    a_str += "<h1>ESP32 EIT Reading Home Page</h1>\n";
    a_str += "<p><p> <a href=\"/data\">Show some data in CSV format</a>\n";
    a_str += "</div>\n</body>\n</html>\n";

    server.send (200, "text/html", a_str); 
}

/** @brief   Respond to a webpage request with arguments for the x,y setpoints
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           with a url requesting /set? arguments, this
 *           callback function is run. It provides the ESP32 with the requested float values. the main web page's text to the
 *           requesting machine.
 */
void handleSetValues() {
    // Expecting: /set?val1=123&val2=456

    if (!server.hasArg("x") || !server.hasArg("y")) {
        server.send(400, "text/plain", "Missing x or y");
        return;
    }

    String val1Str = server.arg("x");
    String val2Str = server.arg("y");

    float value1 = val1Str.toFloat();
    float value2 = val2Str.toFloat();

    Serial.print("Got x = ");
    Serial.print(value1);
    Serial.print(", y = ");
    Serial.println(value2);

    // Respond to the client
    String response = "OK. Received x=" + String(value1) + " y=" + String(value2);
    server.send(200, "text/plain", response);
}

/** @brief   Respond to a webpage request with arguments for communication via flags
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           with a url requesting /flag? arguments, this
 *           callback function is run. This allows the client PC to communicate about what information has been received.
 */
void handleFlags() {
    bool value;

    if (server.hasArg("initializeFLG")) {
        String val = server.arg("initializeFLG");
        Serial << "Got arg: " << "initializeFLG";
        Serial << "with val: " << val << endl;
        value = bool(val);

        // Respond to the client
        String response = "OK. Received initializeFLG=" + value;
        server.send(200, "text/plain", response);
    }
    if (server.hasArg("readFLG")) {
        server.send(400, "text/plain", "Flag is Read-Only");
        return;
    };

}

/** @brief   Respond to a request for an HTTP page that doesn't exist.
 *  @details This function produces the Error 404, Page Not Found error. 
 */
void handle_NotFound (void)
{
    server.send (404, "text/plain", "Not found");
}


/** @brief   Show some simulated data when asked by the web server.
 *  @details The contrived data is sent in a relatively efficient Comma
 *           Separated Variable (CSV) format which is easily read by Matlab(tm)
 *           and Python and spreadsheets.
 */

void handle_data (void)
{
    // Page will consist of one line of comma separated voltage values
    String csv_str = "Voltage Readings,";

    // Create some fake data and put it into a String object.
    //PLACEHOLDER FOR PULLING FROM QUEUE
    for (uint8_t index = 0; index < 208; index++)
    {
        csv_str += index;
        csv_str += ",";
        csv_str += String (sin (index / 5.4321), 3); // 3 decimal places
    }
    csv_str += "\n";

    // Page will also consist of lines of comma separated flag labels and bool values
    csv_str += "initializeFLG,";
    csv_str += "True"; // PLACEHOLDER FOR PULLING FROM SHARE
    csv_str += "\n";
    csv_str += "readFLG,";
    csv_str += "False"; // PLACEHOLDER FOR PULLING FROM SHARE
    csv_str += "\n";

    // Send the CSV file as plain text so it can be easily interpretted
    server.send (200, "text/plain", csv_str);
}


/** @brief   Task which sets up and runs a web server.
 *  @details After setup, function @c handleClient() must be run periodically
 *           to check for page requests from web clients. One could run this
 *           task as the lowest priority task with a short or no delay, as there
 *           generally isn't much rush in replying to web queries.
 *  @param   p_params Pointer to unused parameters
 */
void task_webserver (void* p_params)
{
    // The server has been created statically when the program was started and
    // is accessed as a global object because not only this function but also
    // the page handling functions referenced below need access to the server
    server.on ("/", handle_DocumentRoot);
    server.on ("/data", handle_data);
    server.on ("/set", handleSetValues);
    server.on ("/flags", handleFlags);
    server.onNotFound (handle_NotFound);

    // Get the web server running
    server.begin ();
    Serial.println ("HTTP server started");

    for (;;)
    {
        // The web server must be periodically run to watch for page requests
        server.handleClient ();
        vTaskDelay (100);
    }
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay (100);
    while (!Serial) { }                   // Wait for serial port to be working
    delay(1000);
    Serial << "Would you like to play a game? [y/n]" << endl;

    // Call function which gets the WiFi working
    setup_wifi ();
    
    // Set up the pin for the blue LED on the ESP32 board
    pinMode (LED_PIN, OUTPUT);
    digitalWrite (LED_PIN, LOW);

    // Task which produces the blinking LED
    xTaskCreate (heartbeat, "Pulse", 1024, NULL, 2, NULL);

    // Task which runs the web server.
    xTaskCreate (task_webserver, "Web Server", 8192, NULL, 11, NULL);
}

void loop() {
    // put your main code here, to run repeatedly:
    vTaskDelay(300000);
}
