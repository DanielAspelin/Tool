bool validFormat(std::string_view value) {
    return value == "text" || value == "hex" || value == "binary";
}
