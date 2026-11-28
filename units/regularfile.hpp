std::filesystem::path regularFile(const std::string& value) {
    auto path = std::filesystem::absolute(value);
    if (!std::filesystem::is_regular_file(path)) throw std::invalid_argument("Expected a regular local file");
    return path;
}
