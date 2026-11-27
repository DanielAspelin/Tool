struct COMPASS { bool operator()(double v) const { return std::isfinite(v) && v >= 0 && v < 360; } };
