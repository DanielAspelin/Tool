std::string read(const std::vector<std::string>& arguments) {
    if (arguments.size() > 1) throw std::invalid_argument("Usage: catalogue [branch]");
    const std::string branch = arguments.empty() ? "-" : arguments.front();
    std::string result;
    for (const auto& entry : entries) {
        if (branch == entry.id || branch == entry.parent) {
            if (!result.empty()) result += '\n';
            result += std::string(entry.id) + " | " + entry.title + " | " + entry.evidence;
        }
    }
    if (result.empty()) throw std::invalid_argument("Unknown catalogue branch");
    return result;
}
