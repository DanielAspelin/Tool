double numeric(const std::string& value) {
    std::size_t end{};
    const auto parsed = std::stod(value, &end);
    if (end != value.size() || !std::isfinite(parsed)) throw std::invalid_argument("Expected a finite number");
    return parsed;
}
