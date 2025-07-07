#include "weather_report.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "mongoose.h"
#include "FreeRTOS.h"
#include "task.h"

using namespace std;

#define TLS_CA \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFqjCCBJKgAwIBAgIQfYd70RQkwiYMcCxd6zOrFzANBgkqhkiG9w0BAQsFADBM\n" \
"MSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEGA1UEChMKR2xv\n" \
"YmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0yMjExMTYwMzM1MDhaFw0y\n" \
"OTAzMTgwMDAwMDBaMG8xCzAJBgNVBAYTAkhLMRIwEAYDVQQIEwlIb25nIEtvbmcx\n" \
"EjAQBgNVBAcTCUhvbmcgS29uZzEWMBQGA1UEChMNSG9uZ2tvbmcgUG9zdDEgMB4G\n" \
"A1UEAxMXSG9uZ2tvbmcgUG9zdCBSb290IENBIDMwggIiMA0GCSqGSIb3DQEBAQUA\n" \
"A4ICDwAwggIKAoICAQCziNfqzg8gTr7m1gNt7ln8wlffKWihgw4+aMdoWJwcYEuJ\n" \
"Qwy51BWy7sFOdem1p+/l6TWZ5Mwc50tfjTMwIDNT2aa71T4Tjukfh0mtUC1Qyhi+\n" \
"AViiE3CWu4mIVoBc+L0sPOFMV4i707mV78vH9toxdCim5lSJ9UExyuUmGs2C4HDa\n" \
"Oym71QP1mbpV9WTRYA6ziUm4ii8F0oRFKHyPaFASePwLtVPLwpgchKOesL4jpNrc\n" \
"yCse2m5FHomY2vkALgbpDDtw1VAliJnLzXNg99X/NWfFobxeq81KuEXryGgeDQ0U\n" \
"RhLj0mRiikKYvLTGCAj4/ahMZJx2Ab0vqWwzD9g/KLg8aQFChn5pwckGyuV6RmXp\n" \
"wtZQQS4/t+TtbNe/JgERohYpSms0BpDsE9K2+2p20jzt8NYt3eEV7KObLyzJPivk\n" \
"aTv/ciWxNoZbx39ri1UbSsUgYT2uy1DhCDq+sI9jQVMwCFk8mB13umOResoQUGC/\n" \
"8Ne8lYePl8X+l2oBlKN8W4UdKjk60FSh0Tlxnf0h+bV78OLgAo9uliQlLKAeLKjE\n" \
"iafv7ZkGL7YKTE/bosw3Gq9HhS2KX8Q0NEwA/RiTZxPRN+ZItIsGxVd7GYYKecsA\n" \
"yVKvQv83j+GjHno9UKtjBucVtT+2RTeUN7F+8kjDf8V1/peNRY8apxpyKBpADwID\n" \
"AQABo4IBYzCCAV8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMB\n" \
"BggrBgEFBQcDAjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBQXnc0ei9Y5K3DT\n" \
"XNSguB+wAPzFYTAfBgNVHSMEGDAWgBSP8Et/qC5FJK5NUPpjmove4t0bvDB6Bggr\n" \
"BgEFBQcBAQRuMGwwLQYIKwYBBQUHMAGGIWh0dHA6Ly9vY3NwLmdsb2JhbHNpZ24u\n" \
"Y29tL3Jvb3RyMzA7BggrBgEFBQcwAoYvaHR0cDovL3NlY3VyZS5nbG9iYWxzaWdu\n" \
"LmNvbS9jYWNlcnQvcm9vdC1yMy5jcnQwNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDov\n" \
"L2NybC5nbG9iYWxzaWduLmNvbS9yb290LXIzLmNybDApBgNVHSAEIjAgMAcGBWeB\n" \
"DAEBMAgGBmeBDAECAjALBgkrBgEEAaAyAQEwDQYJKoZIhvcNAQELBQADggEBAEA0\n" \
"F9QNS0HJRzP9ui8n0ZSxxzLvddbFpQmbXAiPoOiGdy9DQVf+ZneXUKl7SN55eOUX\n" \
"lvHkE9FvlfBgrGAuDlfvlfQCXVQc/2VWYqEivAH0a3/wEUWxP0+vQb2cUexWcTDy\n" \
"dDxPiyaGKevJPXN68Qm8HiBW+I+jZn4NIh0+oAEwfatwa4PdKKixEETHZWL7/Ij+\n" \
"X9rMiMGMSb61BoszkVh3kd5j9O69E/qQRZFJ7XbJoCz6tLEtA35tMX4gv/CUFs20\n" \
"fffSf6k0eSNeMADAvclGMoPb/MH2ULptZBER/Q6bh3UE26yva2MwXUCXV8kUnSDX\n" \
"xrNpi9utIhHtiIiGgO4=\n" \
"-----END CERTIFICATE-----"

#define HOST "data.weather.gov.hk"

std::string weather_url = "https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=fnd&lang=en";

WeatherReport::WeatherReport(MgWrapper* mg_wrapper, pimoroni::InkyFrame* inky) {
  this->mg_wrapper = mg_wrapper;
  this->inky = inky;
  this->data = vector<WeatherData>();
}

void WeatherReport::fetch_weather() {
  std::cout << "Fetching weather data from: " << weather_url << std::endl;
  mg_http_connect(this->mg_wrapper->mgr, weather_url.c_str(), WeatherReport::requestEventHandler, (void *) this);
}

optional<vector<WeatherData>> WeatherReport::get_data() {
  optional<vector<WeatherData>> result;
  if (this->data.size() > 0) {
    result = this->data;
    this->data.clear();
  } else {
    result = nullopt;
  }
  return result;
}

bool WeatherReport::update() {
  if (this->data.size() > 0) {
    xTaskCreate(WeatherReport::start_draw_report, "drawr_eport", 1024, this, 1, NULL);
    return true;
  } else {
    return false;
  }
}

void WeatherReport::draw_report() {
  pimoroni::InkyFrame* inky = this->inky;
  auto data = this->get_data();

  if (!data.has_value()) {
     cout << "No weather data available to draw." << std::endl;
    return;
  }

  auto forcasts = data.value();

  this->data.clear();

  cout << "Drawing weather report with " << forcasts.size() << " forecasts." << std::endl;

  inky->color = pimoroni::InkyFrame::WHITE;
  inky->clear();
  inky->set_font("sans");
  inky->set_thickness(2);
 
  auto today_forcase = forcasts.front();
  forcasts.erase(forcasts.begin());

  inky->set_pen(pimoroni::InkyFrame::BLACK);
  int w = inky->width / 4;
  int x = 0;
  int y = 250;
  const int h = 100;
  const int padding = 4;
  int i = 0;

  inky->text(today_forcase.date.substr(2), {10, 50}, 0);

  ostringstream temperature_stream;
  temperature_stream << fixed << setprecision(1) << today_forcase.temperature_low << "-" << today_forcase.temperature_high << " C";
  inky->text(temperature_stream.str(), {10, 100}, 0, 1.2);

  for (auto& forecast : forcasts) {
    if (i >= 8) {
      break; // Limit to 8 forecasts
    }

    inky->set_pen(inky->create_pen_hsv(0, 0.1, 0.85));
    inky->rectangle({x + padding, y + padding, w - padding * 2, 100 - padding * 2});

    inky->set_thickness(4);
    inky->set_pen(pimoroni::InkyFrame::BLACK);
    inky->text(forecast.date.substr(6), {x + padding * 2, y + 16 + padding * 2}, 0, 1);


    inky->set_thickness(2);
    inky->set_pen(pimoroni::InkyFrame::RED);
    ostringstream temperature_stream;
    temperature_stream << fixed << setprecision(1) << forecast.temperature_high << " C";
    inky->text(temperature_stream.str(), {x + padding, y + 50}, 0, 0.6);
    
    inky->set_pen(pimoroni::InkyFrame::GREEN);
    temperature_stream.str("");
    temperature_stream.clear();
    temperature_stream << fixed << setprecision(1) << forecast.temperature_low << " C";
    inky->text(temperature_stream.str(), {x + padding, y + 75}, 0, 0.6);
    x += w;
    i += 1;

    if (x >= inky->width) {
      x = 0;
      y += h;
    }
  }

  inky->update();
}

void WeatherReport::start_draw_report(void* pvParameters) {
  WeatherReport* obj = static_cast<WeatherReport*>(pvParameters);
  obj->draw_report();
  vTaskDelete(NULL);
}

void WeatherReport::requestEventHandler(struct mg_connection *c, int ev, void *ev_data) {
  auto obj = static_cast<WeatherReport *>(c->fn_data);
  // std::shared_ptr<RequestData> request_data = *req_data;

  MG_INFO(("Request event %d\n", ev));
  if (ev == MG_EV_CONNECT) {
    // const char* url = req_data->url.c_str();
    struct mg_str host = mg_url_host(HOST);
      // Send request
    printf("Host connected: %.*s\r\n", (int) host.len, host.buf);
    const struct mg_tls_opts opts = {.ca = mg_str(TLS_CA), .name = host, .skip_verification = 1};
    mg_tls_init(c, &opts);
  }

  if (ev == MG_EV_TLS_HS) {
      const char* url = weather_url.c_str();
      struct mg_str host = mg_url_host(url);
      // Send request
      printf("Host: %.*s\n", (int) host.len, host.buf);

      mg_printf(c,
                "GET %s HTTP/1.0\r\n"
                "Host: %.*s\r\n"
                "\r\n",
                mg_url_uri(url), (int) host.len, host.buf);
  }

  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    // printf("%.*s\n", (int) hm->message.len, hm->message.buf);
    
    struct mg_str json = hm->body;
    struct mg_str json_weather_forecast = mg_json_get_tok(json, "$.weatherForecast");
    // printf("json: %.*s\n", (int) json_weather_forecast.len, json_weather_forecast.buf);
    struct mg_str key, val;

    size_t ofs = 0;
    vector<WeatherData> data = vector<WeatherData>();

    while ((ofs = mg_json_next(json_weather_forecast, ofs, &key, &val)) > 0) {
      double num;
      WeatherData forcast = {
        .date = string(mg_json_get_str(val, "$.forecastDate")),
        .temperature_high = mg_json_get_num(val, "$.forecastMaxtemp.value", &num) ? num : 0.0,
        .temperature_low = mg_json_get_num(val, "$.forecastMintemp.value", &num) ? num : 0.0,
        .condition = ""
      };
    
      data.push_back(forcast);
      cout << "Forcast Date: " << forcast.date << "Temp: " << forcast.temperature_high << "-" << forcast.temperature_low << endl;
    }
    
    obj->data = data;

    c->is_closing = 1;
  }
}