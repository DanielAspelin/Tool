struct DETOKENISER {
    std::string operator()(const std::vector<TOKEN>& tokens) const {
        std::string line;
        for (const auto& token : tokens) {
            if (!line.empty()) line += ' ';
            if (token.reference) line += '&';
            line += '"';
            for (char ch : token.text) {
                switch (ch) {
                case '\\': line += "\\\\"; break;
                case '"': line += "\\\""; break;
                case '\n': line += "\\n"; break;
                case '\r': line += "\\r"; break;
                case '\t': line += "\\t"; break;
                default: line += ch;
                }
            }
            line += '"';
        }
        return line;
    }
};
