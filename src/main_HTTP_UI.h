#include <M5Unified.h>
#include <M5_Ethernet.h>
#include "main_EEPROM_handler.h"

#ifndef MAIN_HTTP_UI_H
#define MAIN_HTTP_UI_H

EthernetServer HttpServer(80);

#define HTTP_GET_PARAM_FROM_POST(paramName)                                              \
  {                                                                                      \
    int start##paramName = currentLine.indexOf(#paramName "=") + strlen(#paramName "="); \
    int end##paramName = currentLine.indexOf("&", start##paramName);                     \
    if (end##paramName == -1)                                                            \
    {                                                                                    \
      end##paramName = currentLine.length();                                             \
    }                                                                                    \
    paramName = currentLine.substring(start##paramName, end##paramName);                 \
    M5.Log.println(#paramName ": " + paramName);                                         \
  }

#define HTML_PUT_INFOWITHLABEL(labelString) \
  client.print(#labelString ": ");          \
  client.print(labelString);                \
  client.println("<br />");

#define HTML_PUT_LI_INPUT(inputName)                                                             \
  {                                                                                              \
    client.println("<li>");                                                                      \
    client.println("<label for=\"" #inputName "\">" #inputName "</label>");                      \
    client.print("<input type=\"text\" id=\"" #inputName "\" name=\"" #inputName "\" value=\""); \
    client.print(inputName);                                                                     \
    client.println("\" required>");                                                              \
    client.println("</li>");                                                                     \
  }

void HTTP_UI_LoadPost(EthernetClient *client)
{
  String currentLine = "";
  // Load post data
  while (client->available())
  {
    char c = client->read();
    if (c == '\n' && currentLine.length() == 0)
    {
      break;
    }
    currentLine += c;
  }

  HTTP_GET_PARAM_FROM_POST(deviceName);
  HTTP_GET_PARAM_FROM_POST(deviceIP_String);
  HTTP_GET_PARAM_FROM_POST(ntpSrvIP_String);
  HTTP_GET_PARAM_FROM_POST(ftpSrvIP_String);
  HTTP_GET_PARAM_FROM_POST(ftp_user);
  HTTP_GET_PARAM_FROM_POST(ftp_pass);
  HTTP_GET_PARAM_FROM_POST(shotInterval);

  PutEEPROM();
  delay(1000);
  ESP.restart();
}

void HTTP_UI_PrintConfigPage(EthernetClient client)
{
  String currentLine = "";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<body>");
  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<form action=\"/\" method=\"post\">");
  client.println("<ul>");

  HTML_PUT_LI_INPUT(deviceName);
  HTML_PUT_LI_INPUT(deviceIP_String);
  HTML_PUT_LI_INPUT(ntpSrvIP_String);
  HTML_PUT_LI_INPUT(ftpSrvIP_String);
  HTML_PUT_LI_INPUT(ftp_user);
  HTML_PUT_LI_INPUT(ftp_pass);
  HTML_PUT_LI_INPUT(shotInterval);

  client.println("<li class=\"button\">");
  client.println("<button type=\"submit\">Save</button>");
  client.println("</li>");

  client.println("</ul>");
  client.println("</form>");
  client.println("</body>");
  client.println("</html>");
}
// used to image stream
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

void sendPage(EthernetClient client, String page)
{
  M5_LOGV("page = %s", page);

  if (page == "view.html")
  {
    // valuejpegStream(&client, &page);
  }
  else if (page == "config.html")
  {
    HTTP_UI_PrintConfigPage(client);
  }
  else if (page == "top.html")
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<h1>" + deviceName + "</h1>");
    client.println("<a href=\"/view.html\">View Page</a><br>");
    client.println("<a href=\"/config.html\">Config Page</a>");
    client.println("</body>");
    client.println("</html>");
  }
  else
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<h1>404 Not Found</h1>");
    client.println("</body>");
    client.println("</html>");
  }
}

void HTTP_UI()
{
  EthernetClient client = HttpServer.available();
  if (client)
  {
    M5_LOGV("new client");

    boolean currentLineIsBlank = true;
    String currentLine = "";
    bool isPost = false;
    bool getRequest = false;
    String page = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        M5.Log.printf("%c",c); //Serial.write(c);
        if (c == '\n' && currentLineIsBlank)
        {
          if (isPost)
          {
            HTTP_UI_LoadPost(&client);
          }

          if (getRequest)
          {
            sendPage(client, page);
            break;
          }
          else
          {
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<body>");
            client.println("<h1>404 Not Found</h1>");
            client.println("</body>");
            client.println("</html>");
            break;
          }

          break;
        }
        if (c == '\n')
        {
          currentLineIsBlank = true;
          currentLine = "";
        }
        else if (c != '\r')
        {
          currentLineIsBlank = false;
          currentLine += c;
        }

        if (currentLine.startsWith("POST /"))
        {
          isPost = true;
        }
        // Check if the current line starts with "GET /view.html"
        if (currentLine.endsWith("GET /view.html"))
        {
          getRequest = true;
          page = "view.html";
        }
        else if (currentLine.endsWith("GET /config.html"))
        {
          getRequest = true;
          page = "config.html";
        }
        else if (currentLine.endsWith("GET /top.html") || currentLine.endsWith("GET /"))
        {
          getRequest = true;
          page = "top.html";
        }
      }
    }
    delay(1);
    client.stop();
    M5_LOGV("client disconnected");
  }
}

#endif