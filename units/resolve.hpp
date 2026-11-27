Words resolve(const std::vector<TOKEN>& tokens, const std::map<std::string, std::string>& values) {
    Words result;
    std::size_t size = 0;
    for (const auto& token : tokens) {
        const auto& value = token.reference ? values.at(token.text) : token.text;
        if (value.size() > 65536 - size) throw std::length_error("Resolved parameters exceed 64 KiB");
        size += value.size();
        result.push_back(value);
    }
    return result;
}
