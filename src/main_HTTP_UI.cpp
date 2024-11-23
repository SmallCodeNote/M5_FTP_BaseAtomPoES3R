#include <M5Unified.h>
#include <M5_Ethernet.h>
#include "M5_Ethernet_NtpClient.h"

#include "main.h"
#include "main_HTTP_UI.h"
#include "main_EEPROM_handler.h"

EthernetServer HttpUIServer(80);
String SensorValueString = "";

void HTTP_UI_PART_ResponceHeader(EthernetClient client, String Content_Type)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + Content_Type);
  client.println("Connection: close");
  client.println();
}

void HTTP_UI_PART_HTMLHeader(EthernetClient client)
{
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<body>");
}

void HTTP_UI_PART_HTMLFooter(EthernetClient client)
{
  client.println("</body>");
  client.println("</html>");
}

void HTTP_UI_POST_configParam(EthernetClient client)
{
  String currentLine = "";
  // Load post data
  while (client.available())
  {
    char c = client.read();
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

  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);
  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");
  client.println("SUCCESS PARAMETER UPDATE.");
  HTTP_UI_PART_HTMLFooter(client);

  delay(1000);
  ESP.restart();
}

void HTTP_UI_POST_configTime(EthernetClient client)
{
  String currentLine = "";
  String timeString = "";
  // Load post data
  while (client.available())
  {
    char c = client.read();
    if (c == '\n' && currentLine.length() == 0)
    {
      break;
    }
    currentLine += c;
  }
  HTTP_GET_PARAM_FROM_POST(timeString);
  NtpClient.updateTimeFromString(timeString);

  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);
  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");
  client.println("SUCCESS TIME UPDATE.");
  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_notFound(EthernetClient client)
{
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>404 Not Found</h1>");

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_top(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<a href=\"/view.html\">View Page</a><br>");
  client.println("<a href=\"/configParam.html\">Config Parameter Page</a><br>");
  client.println("<a href=\"/configTime.html\">Config Time Page</a><br>");

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_configParam(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<form action=\"/configParamSuccess.html\" method=\"post\">");
  client.println("<ul>");

  String currentLine = "";
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

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_configTime(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<form action=\"/configTimeSuccess.html\" method=\"post\">");
  client.println("<ul>");

  String currentLine = "";
  String timeString = "";
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

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_view(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");

  client.print(
      R"(
<!DOCTYPE HTML>
<html>
<body>
<h1>Sensor Data</h1>
<br />
<ul id="sensorData">
<li>distance: <span id="distance"></span> mm</li>
</ul>
<script>
function fetchData() {
 var xhr = new XMLHttpRequest();
 xhr.onreadystatechange = function() {
 if (xhr.readyState == 4 && xhr.status == 200) {
 var data = JSON.parse(xhr.responseText);
 document.getElementById('distance').innerText = data.distance;
 }
 };
 xhr.open('GET', '/sensorValueNow.json', true);
 xhr.send();
}
setInterval(fetchData, 1000);
fetchData();
</script>
</body>
</html>
)");
}

void HTTP_UI_JSON_sensorValueNow(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "application/json");
  client.print("{");
  client.print("\"distance\":");
  client.print(SensorValueString);
  client.println("}");
}

void sendPage(EthernetClient client, String page)
{
  M5_LOGV("page = %s", page);

  if (page == "sensorValueNow.json")
  {
    HTTP_UI_JSON_sensorValueNow(client);
  }
  else if (page == "view.html")
  {
    HTTP_UI_PAGE_view(client);
  }
  else if (page == "top.html")
  {
    HTTP_UI_PAGE_top(client);
  }
  else if (page == "configParam.html")
  {
    HTTP_UI_PAGE_configParam(client);
  }
  else if (page == "configTime.html")
  {
    HTTP_UI_PAGE_configTime(client);
  }
  else
  {
    HTTP_UI_PAGE_notFound(client);
  }
}

void HTTP_UI()
{
  EthernetClient client = HttpUIServer.available();
  if (client)
  {
    M5_LOGV("new client");

    boolean currentLineIsBlank = true;
    String currentLine = "";
    String page = "";

    bool getRequest = false;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        M5.Log.printf("%c", c); // Serial.write(c);
        if (c == '\n' && currentLineIsBlank)
        {
          M5_LOGI("%s", currentLine);

          if (getRequest)
          {
            sendPage(client, page);
            break;
          }
          else
          {
            HTTP_UI_PAGE_notFound(client);
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

        
        if (currentLine.endsWith("GET /sensorValueNow.json"))
        {
          getRequest = true;
          page = "sensorValueNow.json";
        }
        else if (currentLine.endsWith("GET /view.html"))
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
        }
        else if (currentLine.endsWith("GET /top.html") || currentLine.endsWith("GET /"))
        {
          getRequest = true;
          page = "top.html";
        }
        else if (currentLine.startsWith("POST /configParamSuccess.html"))
        {
          getRequest = true;
          page = "configParamSuccess.html";
        }
        else if (currentLine.startsWith("POST /configTimeSuccess.html"))
        {
          getRequest = true;
          page = "configTimeSuccess.html";
        }
      }
    }
    delay(1);
    client.stop();
    M5_LOGV("client disconnected");
  }
}