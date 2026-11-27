std::vector<IR::Step> file(const std::string& path) {
    if (!std::filesystem::is_regular_file(path) || std::filesystem::file_size(path) > 65536) {
        throw std::invalid_argument("Expected a regular script file of at most 64 KiB");
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open script");
    std::string bytes(65537, '\0');
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    const auto size = static_cast<std::size_t>(input.gcount());
    if (input.bad() || size > 65536) throw std::runtime_error("Cannot read script within 64 KiB");
    bytes.resize(size);
    if (bytes.find('\0') != std::string::npos) throw std::invalid_argument("NUL is unavailable in script source");
    std::istringstream source(bytes);
    return IR{}(source);
}
