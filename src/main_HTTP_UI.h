#include <M5Unified.h>
#include <M5_Ethernet.h>
#include "main_EEPROM_handler.h"

#ifndef MAIN_HTTP_UI_H
#define MAIN_HTTP_UI_H

EthernetServer HttpServer(80);
String tofDeviceValueString = "";

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

void HTTP_UI_LoadPostParam(EthernetClient *client)
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

void HTTP_UI_LoadPostTime(EthernetClient *client)
{
  String currentLine = "";
  String timeString = "";
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

  HTTP_GET_PARAM_FROM_POST(timeString);

  NtpClient.updateTimeFromString(timeString);
}

void HTTP_UI_PrintConfigParamPage(EthernetClient client)
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

  client.println("<form action=\"/configParamSuccess.html\" method=\"post\">");
  client.println("<ul>");

  HTML_PUT_LI_INPUT(deviceName);
  HTML_PUT_LI_INPUT(deviceIP_String);
  HTML_PUT_LI_INPUT(ntpSrvIP_String);
  HTML_PUT_LI_INPUT(ftpSrvIP_String);
  HTML_PUT_LI_INPUT(ftp_user);
  HTML_PUT_LI_INPUT(ftp_pass);
  HTML_PUT_LI_INPUT(shotInterval);

  client.println("<li>");
  client.println("<label for=\"timestamp\">Timestamp</label>");
  client.println("<input type=\"text\" id=\"timestamp\" name=\"timestamp\" value=\"\" required>");
  client.println("</li>");

  client.println("<li class=\"button\">");
  client.println("<button type=\"submit\">Save</button>");
  client.println("</li>");

  client.println("</ul>");
  client.println("</form>");

  client.println("<script>");
  client.println("function updateTimestamp() {");
  client.println(" document.getElementById('timestamp').value = new Date().toLocaleString();");
  client.println("}");
  client.println("setInterval(updateTimestamp, 1000);");
  client.println("updateTimestamp();");
  client.println("</script>");

  client.println("</body>");
  client.println("</html>");
}

void HTTP_UI_PrintConfigTimePage(EthernetClient client)
{
  String timeString = "";

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

  client.println("<form action=\"/configTimeSuccess.html\" method=\"post\">");
  client.println("<ul>");

  HTML_PUT_LI_INPUT(timeString);

  client.println("<li class=\"button\">");
  client.println("<button type=\"submit\">Save</button>");
  client.println("</li>");

  client.println("</ul>");
  client.println("</form>");

  client.println("<script>");
  client.println("function updateTimeString() {");
  client.println(" document.getElementById('timeString').value = new Date().toLocaleString();");
  client.println("}");
  client.println("setInterval(updateTimeString, 1000);");
  client.println("updateTimeString();");
  client.println("</script>");

  client.println("</body>");
  client.println("</html>");
}

void HTTP_UI_PrintSensorData(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<body>");
  client.println("<h1>M5Stack Sensor Data</h1>");
  client.println("<br />");
  client.println("<ul id=\"sensorData\">");
  client.println("<li>distance: <span id=\"distance\"></span> mm</li>");
  client.println("</ul>");
  client.println("<script>");
  client.println("function fetchData() {");
  client.println(" var xhr = new XMLHttpRequest();");
  client.println(" xhr.onreadystatechange = function() {");
  client.println(" if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println(" var data = JSON.parse(xhr.responseText);");
  client.println(" document.getElementById('distance').innerText = data.distance;");
  client.println(" }");
  client.println(" };");
  client.println(" xhr.open('GET', '/data', true);");
  client.println(" xhr.send();");
  client.println("}");
  client.println("setInterval(fetchData, 1000);");
  client.println("fetchData();");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
}

void HTTP_UI_SendSensorData(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.print("{");
  client.print("\"distance\":");
  client.print(tofDeviceValueString);
  client.println("}");
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
    HTTP_UI_PrintSensorData(client);
    // valuejpegStream(&client, &page);
  }
  else if (page == "configParam.html")
  {
    HTTP_UI_PrintConfigParamPage(client);
  }
  else if (page == "configTime.html")
  {
    HTTP_UI_PrintConfigTimePage(client);
  }else if (page == "data")
  {
    HTTP_UI_SendSensorData(client);
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
    client.println("<a href=\"/configParam.html\">Config Parameter Page</a><br>");
    client.println("<a href=\"/configTime.html\">Config Time Page</a><br>");
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
    bool isPostConfigParam = false;
    bool isPostConfigTime = false;
    bool getRequest = false;
    String page = "";
    uint16_t intervalCount = 0;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        M5.Log.printf("%c", c); // Serial.write(c);
        if (c == '\n' && currentLineIsBlank)
        {
          if (isPostConfigParam)
          {
            HTTP_UI_LoadPostParam(&client);
          }
          if (isPostConfigTime)
          {
            HTTP_UI_LoadPostTime(&client);
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

        if (currentLine.startsWith("POST /configParam"))
        {
          isPostConfigParam = true;
        }
        if (currentLine.startsWith("POST /configTime"))
        {
          isPostConfigTime = true;
        }
        // Check if the current line starts with "GET /view.html"
        if (currentLine.endsWith("GET /view.html"))
        {
          getRequest = true;
          page = "view.html";
        }
        else if (currentLine.endsWith("GET /configParam.html"))
        {
          getRequest = true;
          page = "configParam.html";
        }
        else if (currentLine.endsWith("GET /configTime.html"))
        {
          getRequest = true;
          page = "configTime.html";
        }else if (currentLine.endsWith("GET /data"))
        {
          getRequest = true;
          page = "data";
        }
        else if (currentLine.endsWith("GET /top.html") || currentLine.endsWith("GET /"))
        {
          getRequest = true;
          page = "top.html";
        }
      }

      if (intervalCount > 2000)
      {
        HTTP_UI_SendSensorData(client);
        intervalCount = 0;
      }
      intervalCount++;
    }
    delay(1);
    client.stop();
    M5_LOGV("client disconnected");
  }
}

#endif