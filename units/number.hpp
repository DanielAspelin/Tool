std::size_t number(std::string_view input) {
    std::size_t value{};
    const auto parsed = std::from_chars(input.data(), input.data() + input.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != input.data() + input.size()) {
        throw std::invalid_argument("Expected a nonnegative integer");
    }
    return value;
}
