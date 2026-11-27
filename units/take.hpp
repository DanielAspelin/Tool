std::string_view take(std::string_view& line, char separator) {
    const auto position = line.find(separator);
    const auto part = line.substr(0, position);
    line.remove_prefix(position == std::string_view::npos ? line.size() : position + 1);
    return part;
}
