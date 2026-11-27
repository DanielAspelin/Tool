struct ARRANGEMENT {
    enum class Device { automatic, desktop, tablet, mobile };
    Device operator()(std::string_view value) const {
        if (value == "auto") return Device::automatic;
        if (value == "desktop") return Device::desktop;
        if (value == "tablet") return Device::tablet;
        if (value == "mobile") return Device::mobile;
        throw std::invalid_argument("Expected auto, desktop, tablet or mobile");
    }
};
