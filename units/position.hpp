struct POSITION {
    bool operator()(double latitude, double longitude, double altitude) const {
        return LOCATION{}(latitude, longitude) && std::isfinite(altitude);
    }
};
