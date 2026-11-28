std::string measure(const Arguments& args) {
    if (args.size() != 2) throw std::invalid_argument("Usage: measure quantity value");
    static const std::map<std::string_view, std::pair<std::string_view, std::function<bool(double)>>> quantities{
        {"length", {"m", LENGTH{}}}, {"width", {"m", WIDTH{}}}, {"height", {"m", HEIGHT{}}},
        {"altitude", {"m", ALTITUDE{}}}, {"latitude", {"deg", LATITUDE{}}},
        {"longitude", {"deg", LONGITUDE{}}}, {"decibel", {"dB", DECIBEL{}}},
        {"hertz", {"Hz", HERTZ{}}}, {"frequency", {"Hz", HERTZ{}}}, {"volume", {"m³", LENGTH{}}},
        {"pressure", {"Pa", LENGTH{}}}, {"temperature", {"°C", TEMPERATURE{}}},
        {"rpm", {"rpm", RPM{}}}, {"humidity", {"% RH", HUMIDITY{}}},
        {"compass", {"deg", COMPASS{}}}, {"winddirection", {"deg", WINDDIRECTION{}}},
        {"windpressure", {"Pa", WINDPRESSURE{}}}
    };
    const auto item = quantities.find(args[0]);
    if (item == quantities.end()) throw std::invalid_argument("Unknown measurement");
    const auto value = numeric(args[1]);
    if (!item->second.second(value)) throw std::invalid_argument("Measurement outside accepted range");
    std::ostringstream result;
    result << value << ' ' << item->second.first;
    return result.str();
}
