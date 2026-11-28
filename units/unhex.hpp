Bytes unhex(std::string_view value) {
    if (value.size() % 2 != 0) throw std::invalid_argument("Incomplete hex pair");
    Bytes result(value.size() / 2);
    for (std::size_t i = 0; i < result.size(); ++i) {
        const auto pair = value.substr(i * 2, 2);
        unsigned octet{};
        auto parsed = std::from_chars(pair.data(), pair.data() + 2, octet, 16);
        if (parsed.ec != std::errc{} || parsed.ptr != pair.data() + 2) throw std::invalid_argument("Invalid hex payload");
        result[i] = static_cast<unsigned char>(octet);
    }
    return result;
}
