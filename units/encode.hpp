std::string encode(const Bytes& data, std::string_view format) {
    if (format == "hex") return hex(data.data(), data.size());
    if (data.empty()) return {};
    if (format == "binary") return {reinterpret_cast<const char*>(data.data()), data.size()};
    std::string encoded(((data.size() + 2) / 3) * 4 + 1, '\0');
    const auto length = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), data.data(), static_cast<int>(data.size()));
    if (length < 0) throw std::runtime_error("Cannot encode capsule");
    encoded.resize(static_cast<std::size_t>(length));
    return encoded;
}
