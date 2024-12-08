#include <M5Unified.h>
#include <M5_Ethernet.h>
#include "M5_Ethernet_NtpClient.h"

#include "main.h"
#include "main_HTTP_UI.h"
#include "main_HTTP_UI_ChartJS.h"
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

void HTTP_UI_JSON_sensorValueNow(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "application/json");
  client.print("{");
  client.print("\"distance\":");
  client.print(String(SensorValueString.toInt()));
  client.println("}");
}

void HTTP_UI_JSON_unitTimeNow(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "application/json");
  client.print("{");
  client.print("\"unitTime\":");
  client.printf("\"%s\"", NtpClient.convertTimeEpochToString().c_str());
  client.println("}");
}

void HTTP_UI_PAGE_view(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<ul id=\"sensorData\">");
  client.println("<li>Distance: <span id=\"distance\"></span> mm</li>");
  client.println("</ul>");

  client.println("<script>");
  client.println("function fetchData() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var data = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('distance').innerText = data.distance;");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.open('GET', '/sensorValueNow.json', true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("setInterval(fetchData, 1000);");
  client.println("fetchData();");
  client.println("</script>");

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_chart(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<ul id=\"sensorData\">");
  client.println("<li>Distance: <span id=\"distance\"></span> mm</li>");
  client.println("</ul>");

  client.println("<canvas id=\"distanceChart\" width=\"400\" height=\"200\"></canvas>");

  client.println("<script src=\"/chart.js\"></script>");
  client.println("<script>");
  client.printf("var distanceData = Array(%s).fill(0);\n", chartShowPointCount.c_str());
  client.println("var myChart = null;");
  client.println("function fetchData() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var data = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('distance').innerText = data.distance;");
  client.println("      distanceData.push(data.distance);");
  client.printf("      if (distanceData.length > %s) { distanceData.shift(); }", chartShowPointCount.c_str());
  client.println("      updateChart();");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.open('GET', '/sensorValueNow.json', true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("function updateChart() {");
  client.println("  var ctx = document.getElementById('distanceChart').getContext('2d');");

  client.println("  if (myChart) {");
  client.println("    myChart.destroy();");
  client.println("  }");

  client.println("  myChart = new Chart(ctx, {");
  client.println("    type: 'line',");
  client.println("    data: {");
  client.println("      labels: Array.from({length: distanceData.length}, (_, i) => i + 1),");
  client.println("      datasets: [{");
  client.println("        label: 'Distance',");
  client.println("        data: distanceData,");
  client.println("        borderColor: 'rgba(75, 192, 192, 1)',");
  client.println("        borderWidth: 1");
  client.println("      }]");
  client.println("    },");
  client.println("    options: {");
  client.println("      animation: false,");
  client.println("      scales: {");
  client.println("        x: { beginAtZero: true },");
  client.println("        y: { beginAtZero: true }");
  client.println("      }");
  client.println("    }");
  client.println("  });");
  client.println("}");
  client.printf("setInterval(fetchData, %s);", chartUpdateInterval.c_str());
  client.println("fetchData();");
  client.println("</script>");

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
  HTML_PUT_LI_WIDEINPUT(deviceName);
  HTML_PUT_LI_WIDEINPUT(deviceIP_String);
  HTML_PUT_LI_WIDEINPUT(ntpSrvIP_String);
  HTML_PUT_LI_WIDEINPUT(ftpSrvIP_String);
  HTML_PUT_LI_WIDEINPUT(ftp_user);
  HTML_PUT_LI_WIDEINPUT(ftp_pass);
  HTML_PUT_LI_INPUT(ftpSaveInterval);
  HTML_PUT_LI_INPUT(chartShowPointCount);
  HTML_PUT_LI_INPUT(chartUpdateInterval);
  HTML_PUT_LI_INPUT(timeZoneOffset);

  client.println("<li class=\"button\">");
  client.println("<button type=\"submit\">Save</button>");
  client.println("</li>");
  client.println("</ul>");
  client.println("</form>");

  client.println("<br />");
  client.printf("<a href=\"http://%s/top.html\">Return Top</a><br>", deviceIP_String.c_str());

  HTTP_UI_PART_HTMLFooter(client);
}

void TaskRestart(void *arg)
{
  delay(5000);
  ESP.restart();
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
  HTTP_GET_PARAM_FROM_POST(ftpSaveInterval);
  HTTP_GET_PARAM_FROM_POST(chartShowPointCount);
  HTTP_GET_PARAM_FROM_POST(chartUpdateInterval);
  HTTP_GET_PARAM_FROM_POST(timeZoneOffset);

  PutEEPROM();

  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);
  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");
  client.println("SUCCESS PARAMETER UPDATE.");

  client.println("<br />");
  client.printf("<a href=\"http://%s/top.html\">Return Top</a><br>", deviceIP_String.c_str());

  HTTP_UI_PART_HTMLFooter(client);

  xTaskCreatePinnedToCore(TaskRestart, "TaskRestart", 4096, NULL, 11, NULL, 1);
}

void HTTP_UI_PAGE_configTime(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<ul id=\"unitTime\">");
  client.println("<li>unitTime: <span id=\"unitTime\"></span></li>");
  client.println("</ul>");

  client.println("<form action=\"/configTimeSuccess.html\" method=\"post\">");
  client.println("<ul>");

  String currentLine = "";
  String timeString = "";
  HTML_PUT_LI_WIDEINPUT(timeString);

  client.println("<li class=\"button\">");
  client.println("<button type=\"submit\">Save</button>");
  client.println("</li>");
  client.println("</ul>");
  client.println("</form>");

  client.println("<br />");
  client.printf("<a href=\"http://%s/top.html\">Return Top</a><br>", deviceIP_String.c_str());

  client.println("<script>");

  client.println("function updateTimeString() {");
  client.println(" document.getElementById('timeString').value = new Date().toLocaleString();");
  client.println("}");
  client.println("setInterval(updateTimeString, 1000);");

  client.println("function fetchData() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var data = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('unitTime').innerText = data.unitTime;");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.open('GET', '/unitTimeNow.json', true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("setInterval(fetchData, 1000);");

  client.println("fetchData();");
  client.println("updateTimeString();");
  client.println("</script>");

  HTTP_UI_PART_HTMLFooter(client);
}

String urlDecode(String input)
{
  String decoded = "";
  char temp[] = "0x00";
  unsigned int i, j;
  for (i = 0; i < input.length(); i++)
  {
    if (input[i] == '%')
    {
      temp[2] = input[i + 1];
      temp[3] = input[i + 2];
      decoded += (char)strtol(temp, NULL, 16);
      i += 2;
    }
    else if (input[i] == '+')
    {
      decoded += ' ';
    }
    else
    {
      decoded += input[i];
    }
  }
  return decoded;
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

  timeString = urlDecode(timeString);
  M5_LOGI("posted timeString = %s", timeString.c_str());
  NtpClient.updateTimeFromString(timeString);

  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);
  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");
  client.println("SUCCESS TIME UPDATE.");

  client.println("<ul id=\"unitTime\">");
  client.println("<li>unitTime: <span id=\"unitTime\"></span></li>");
  client.println("</ul>");

  client.printf("<a href=\"http://%s/top.html\">Return Top</a><br>", deviceIP_String.c_str());

  client.println("<script>");
  client.println("function fetchData() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var data = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('unitTime').innerText = data.unitTime;");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.open('GET', '/unitTimeNow.json', true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("setInterval(fetchData, 1000);");
  client.println("fetchData();");
  client.println("</script>");

  HTTP_UI_PART_HTMLFooter(client);
  return;
}

void HTTP_UI_PAGE_unitTime(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<br />");

  client.println("<ul id=\"unitTime\">");
  client.println("<li>unitTime: <span id=\"unitTime\"></span></li>");
  client.println("</ul>");

  client.println("<br>");
  client.printf("<a href=\"http://%s/top.html\">Return Top</a><br>", deviceIP_String.c_str());

  client.println("<script>");
  client.println("function fetchData() {");
  client.println("  var xhr = new XMLHttpRequest();");
  client.println("  xhr.onreadystatechange = function() {");
  client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
  client.println("      var data = JSON.parse(xhr.responseText);");
  client.println("      document.getElementById('unitTime').innerText = data.unitTime;");
  client.println("    }");
  client.println("  };");
  client.println("  xhr.open('GET', '/unitTimeNow.json', true);");
  client.println("  xhr.send();");
  client.println("}");
  client.println("setInterval(fetchData, 1000);");
  client.println("fetchData();");
  client.println("</script>");

  HTTP_UI_PART_HTMLFooter(client);
}

void HTTP_UI_PAGE_top(EthernetClient client)
{
  HTTP_UI_PART_ResponceHeader(client, "text/html");
  HTTP_UI_PART_HTMLHeader(client);

  client.println("<h1>" + deviceName + "</h1>");
  client.println("<a href=\"/view.html\">View Page</a><br>");
  client.println("<a href=\"/chart.html\">Chart Page</a><br>");
  client.println("<a href=\"/unitTime.html\">Unit Time</a><br>");

  client.println("<br>");
  client.println("<hr>");
  client.println("<br>");

  client.println("<a href=\"/configParam.html\">Config Parameter Page</a><br>");
  client.println("<a href=\"/configTime.html\">Config Time Page</a><br>");

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



PageHandler pageHandlers[] = {
    {HTTP_UI_MODE_GET, "sensorValueNow.json", HTTP_UI_JSON_sensorValueNow},
    {HTTP_UI_MODE_GET, "unitTimeNow.json", HTTP_UI_JSON_unitTimeNow},
    {HTTP_UI_MODE_GET, "view.html", HTTP_UI_PAGE_view},
    {HTTP_UI_MODE_GET, "chart.js", HTTP_UI_JS_ChartJS},
    {HTTP_UI_MODE_GET, "chart.html", HTTP_UI_PAGE_chart},
    {HTTP_UI_MODE_GET, "configParam.html", HTTP_UI_PAGE_configParam},

    {HTTP_UI_MODE_GET, "configTime.html", HTTP_UI_PAGE_configTime},
    {HTTP_UI_MODE_GET, "unitTime.html", HTTP_UI_PAGE_unitTime},

    {HTTP_UI_MODE_GET, "top.html", HTTP_UI_PAGE_top},
    {HTTP_UI_MODE_POST, "configParamSuccess.html", HTTP_UI_POST_configParam},

    {HTTP_UI_MODE_POST, "configTimeSuccess.html", HTTP_UI_POST_configTime},
    {HTTP_UI_MODE_GET, " ", HTTP_UI_PAGE_top} // default handler
};


void sendPage(EthernetClient httpClient, String page)
{
  M5_LOGI("page = %s", page.c_str());

  size_t numPages = sizeof(pageHandlers) / sizeof(pageHandlers[0]);

  for (size_t i = 0; i < numPages; ++i)
  {
    if (page == pageHandlers[i].page)
    {
      pageHandlers[i].handler(httpClient);
      return;
    }
  }
  HTTP_UI_PAGE_notFound(httpClient);
}

void HTTP_UI()
{
  EthernetClient httpClient = HttpUIServer.available();
  if (httpClient)
  {
    if (xSemaphoreTake(mutex_Ethernet, portMAX_DELAY) == pdTRUE)
    {
      M5_LOGD("mutex take success");

      unsigned long millis0 = millis();
      unsigned long millis1 = millis0;

      // unsigned long millisStart = millis0;
      unsigned long clientStart = millis0;

      M5_LOGI("new httpClient");
      boolean currentLineIsBlank = true;
      boolean currentLineIsNotGetPost = false;
      boolean currentLineHaveEnoughLength = false;

      String currentLine = "";
      String page = "";
      bool getRequest = false;

      size_t numPages = sizeof(pageHandlers) / sizeof(pageHandlers[0]);

      unsigned long saveTimeout = httpClient.getTimeout();
      M5_LOGI("default Timeout = %u", saveTimeout);
      httpClient.setTimeout(1000);

      unsigned long loopCount = 0;
      unsigned long charCount = 0;

      while (httpClient.connected())
      {
        loopCount++;
        if (httpClient.available())
        {
          char c = httpClient.read();
          charCount++;

          if (c == '\n' && currentLineIsBlank) // Request End Check ( request end line = "\r\n")
          {
            // Request End task
            if (getRequest)
            {
              sendPage(httpClient, page);
            }
            else
            {
              HTTP_UI_PAGE_notFound(httpClient);
            }
            M5_LOGD("break from request line end.");
            break;
          }

          if (c == '\n') // Line End
          {
            M5_LOGV("%s", currentLine.c_str());
            currentLineIsBlank = true, currentLine = "";
            currentLineIsNotGetPost = false;
            currentLineHaveEnoughLength = false;
            millis0 = millis();
          }
          else if (c != '\r') // Line have request char
          {
            currentLineIsBlank = false, currentLine += c;
          }

          if (!currentLineHaveEnoughLength && currentLine.length() > 6)
          {
            currentLineHaveEnoughLength = true;
          }

          if (!currentLineIsNotGetPost && currentLineHaveEnoughLength && (!currentLine.startsWith("GET /") && !currentLine.startsWith("POST /")))
          {
            currentLineIsNotGetPost = true;
          }

          if (currentLineHaveEnoughLength && !currentLineIsNotGetPost && !getRequest)
          {
            for (size_t i = 0; i < numPages; i++)
            {
              String pageName = String(pageHandlers[i].page);
              String CheckLine = (pageHandlers[i].mode == HTTP_UI_MODE_GET ? String("GET /") : String("POST /")) + pageName;

              if (currentLine.startsWith(CheckLine.c_str()))
              {
                page = (pageHandlers[i].mode == HTTP_UI_MODE_GET ? CheckLine.substring(5) : CheckLine.substring(6));
                M5_LOGI("currentLine = [%s] : CheckLine = [%s]: page = [%s]", currentLine.c_str(), CheckLine.c_str(), page.c_str());
                getRequest = true;
                break;
              }
            }
          }
        }

        if (millis0 - millis1 >= 500)
        {
          M5_LOGE("%s : %u", currentLine.c_str(), millis0 - millis1);
        }
        millis1 = millis0;
        delay(1);
      }

      httpClient.setTimeout(saveTimeout);
      httpClient.stop();

      M5_LOGI("httpClient disconnected : httpClient alived time =  %u ms", millis() - clientStart);
      M5_LOGI("loopCount =  %u ,charCount = %u", loopCount, charCount);

      xSemaphoreGive(mutex_Ethernet);
      M5_LOGI("mutex give");
    }
    else
    {
      M5_LOGW("mutex can not take");
    }
  }
}
