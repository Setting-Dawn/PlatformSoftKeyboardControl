/*!
* @file EITwebhost.cpp
* This library allows the Softkeyboard project to host values and communicate
* with an external program to determine tilt position.
* 
* @author Setting-Dawn
* Based on an example for CalPoly SLO's ME 507 course by JR Ridgely
* @author JR Ridgely
* subsequently based on an examples by A. Sinha at 
* @c https://github.com/hippyaki/WebServers-on-ESP32-Codes
* @author A. Sinha
* @version 1.0.0
* @date 2022-Mar-28 Original stuff by Sinha
* @date 2022-Nov-04 Modified for ME507 use by Ridgely
* @date 2025-Dec-03 Modified for Softkeyboard project by Setting-Dawn
* @copyright 2022 by the authors, released under the MIT License.
*/

#include "EITwebhost.h"
#include "shares.h"

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
    }
    if (server.hasArg("adc1Map") && server.hasArg("adc2Map") && server.hasArg("currMap")) {
        String data_str = server.arg("adc1Map");
        // Parse comma-separated values
        uint8_t values[16] = {0};
        int index = 0;
        int start = 0;
        
        for (int i = 0; i <= data_str.length(); i++) {
            if (data_str[i] == ',' || i == data_str.length()) {
                values[index++] = atoi(data_str.substring(start, i).c_str());
                start = i + 1;
            }
        }
        adc1PinMap.put(values);

        data_str = server.arg("adc2Map");
        // Parse comma-separated values
        values[16] = {0};
        index = 0;
        start = 0;
        
        for (int i = 0; i <= data_str.length(); i++) {
            if (data_str[i] == ',' || i == data_str.length()) {
                values[index++] = atoi(data_str.substring(start, i).c_str());
                start = i + 1;
            }
        }
        adc2PinMap.put(values);
        
        data_str = server.arg("currMap");
        // Parse comma-separated values
        values[16] = {0};
        index = 0;
        start = 0;
        
        for (int i = 0; i <= data_str.length(); i++) {
            if (data_str[i] == ',' || i == data_str.length()) {
                values[index++] = atoi(data_str.substring(start, i).c_str());
                start = i + 1;
            }
        }
        currPinMap.put(values);

        reMapCompleteFLG.put(true);

        server.send(200, "text/plain", "Array received");

    };

}

/** @brief   Respond to a request for an HTTP page that doesn't exist.
 *  @details This function produces the Error 404, Page Not Found error. 
 */
void handle_NotFound (void)
{
    server.send (404, "text/plain", "Not found");
}

/** @brief   Return data when requested.
 *  @details The measured data is sent in comma seperated value (CSV) format 
 *           which is easily read by Matlab(tm), Python, and spreadsheets.
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


