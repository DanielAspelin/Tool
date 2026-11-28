Bytes read(const std::filesystem::path& path, std::size_t limit) {
    if (!std::filesystem::is_regular_file(path) || std::filesystem::file_size(path) > limit) {
        throw std::invalid_argument("Expected a bounded regular input file");
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open input file");
    Bytes result;
    char block[8192];
    while (input.read(block, sizeof(block)) || input.gcount()) {
        const auto count = static_cast<std::size_t>(input.gcount());
        if (count > limit - result.size()) throw std::length_error("File exceeds capsule limit");
        result.insert(result.end(), block, block + count);
    }
    if (!input.eof() || input.bad()) throw std::runtime_error("Cannot read input file");
    return result;
}
