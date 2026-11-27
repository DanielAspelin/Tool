class DESCRIPTOR {
    int value;
public:
    explicit DESCRIPTOR(int value) : value(value) {}
    DESCRIPTOR(const DESCRIPTOR&) = delete;
    DESCRIPTOR(DESCRIPTOR&& other) noexcept : value(std::exchange(other.value, -1)) {}
    ~DESCRIPTOR() { reset(); }
    int get() const { return value; }
    void reset() {
        if (value >= 0) ::close(value);
        value = -1;
    }
};
