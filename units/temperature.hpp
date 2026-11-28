struct TEMPERATURE { bool operator()(double v) const { return std::isfinite(v) && v >= -273.15; } };
