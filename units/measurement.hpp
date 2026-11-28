template<int Minimum, int Maximum>
struct MEASUREMENT {
    bool operator()(double v) const { return std::isfinite(v) && v >= Minimum && v <= Maximum; }
};
