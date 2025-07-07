#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "libraries/inky_frame_7/inky_frame_7.hpp"
#include "web_server.hpp"

struct WeatherData {
    std::string date;
    double temperature_high;
    double temperature_low;
    std::string condition; // e.g., "Sunny", "Rainy", etc.
};

class WeatherReport {
public:
    WeatherReport(MgWrapper* mg_wrapper, pimoroni::InkyFrame* inky);
    void fetch_weather();
    bool update();
    std::optional<std::vector<WeatherData>> get_data();
    void draw_report();
private:
    MgWrapper* mg_wrapper;
    pimoroni::InkyFrame* inky;
    std::vector<WeatherData> data;
    static void requestEventHandler(struct mg_connection *c, int ev, void *ev_data);
    static void start_draw_report(void* pvParameters);
};