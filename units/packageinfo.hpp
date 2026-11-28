std::string packageInfo(const Arguments& args) {
    if (args.size() > 2) throw std::invalid_argument("Usage: packages [gps|nfc|gsm|tcpip|rfid] [abbreviation]");
    std::string result;
    for (const auto& item : packages) {
        if (!args.empty() && args[0] != item.name) continue;
        if (args.size() == 2 && !ABBREVIATION{}(item, args[1])) {
            throw std::invalid_argument("Abbreviation not registered in this package");
        }
        if (!result.empty()) result += "\n\n";
        result += std::string(item.name) + "  " + std::string(item.abbreviations) + "\n" +
            std::string(item.status) + "\n" + std::string(item.source);
    }
    if (result.empty()) throw std::invalid_argument("Unknown protocol package");
    return result;
}
