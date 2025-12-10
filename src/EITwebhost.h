#ifndef __EITWEBHOST_H__
#define __EITWEBHOST_H__
/*!
* @file EITwebhost.h
* @brief This library allows the Softkeyboard project to host values and communicate
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

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "PrintStream.h"

// The web server object is defined in EITwebhost.cpp; declare it here so
// other translation units (for example `main.cpp`) can reference it.
extern WebServer server;

/** @brief   Get the WiFi running so we can serve some web pages.
 */
void setup_wifi(void);

/** @brief   Put a web page header into an HTML string. 
 *  @details This header may be modified if the developer wants some actual
 *           @a style for her or his web page. It is intended to be a common
 *           header (and stylle) for each of the pages served by this server.
 *  @param   a_string A reference to a string to which the header is added; the
 *           string must have been created in each function that calls this one
 *  @param   page_title The title of the page
*/
void HTML_header (String& a_string, const char* page_title);

/** @brief   Callback function that responds to HTTP requests without a subpage
 *           name.
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           (the insecure Web port) with a request for the main web page, this
 *           callback function is run. It sends the main web page's text to the
 *           requesting machine.
 */
void handle_DocumentRoot ();

/** @brief   Respond to a webpage request with arguments for the x,y setpoints
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           with a url requesting /set? arguments, this
 *           callback function is run. It provides the ESP32 with the requested float values. the main web page's text to the
 *           requesting machine.
 */
void handleSetValues();

/** @brief   Respond to a webpage request with arguments for communication via flags
 *  @details When another computer contacts this ESP32 through TCP/IP port 80
 *           with a url requesting /flag? arguments, this
 *           callback function is run. This allows the client PC to communicate about what information has been received.
 */
void handleFlags();

/** @brief   Respond to a request for an HTTP page that doesn't exist.
 *  @details This function produces the Error 404, Page Not Found error. 
 */
void handle_NotFound (void);

/** @brief   Return data when requested.
 *  @details The measured data is sent in comma seperated value (CSV) format 
 *           which is easily read by Matlab(tm), Python, and spreadsheets.
 */
void handle_data (void);

#endif //__EITWEBHOST_H__