const Source& sourceFor(std::string_view id) {
    for (const auto& source : sources) {
        if (source.id == id) return source;
    }
    throw std::invalid_argument("Unknown language catalogue source");
}
