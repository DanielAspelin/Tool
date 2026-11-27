Bytes decode(std::string_view data, std::string_view format) {
    if (format == "binary") return {data.begin(), data.end()};
    if (format == "hex") return unhex(data);
    if (data.empty()) return {};
    if (data.size() % 4) throw std::invalid_argument("Invalid capsule text length");
    Bytes result(data.size() / 4 * 3);
    const auto length = EVP_DecodeBlock(result.data(), reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()));
    if (length < 0) throw std::invalid_argument("Invalid capsule text");
    const unsigned padding = (data.back() == '=') + (data[data.size() - 2] == '=');
    result.resize(static_cast<std::size_t>(length) - padding);
    if (encode(result, "text") != data) throw std::invalid_argument("Noncanonical capsule text");
    return result;
}
