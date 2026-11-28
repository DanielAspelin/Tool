struct TOKENISER {
    std::vector<TOKEN> operator()(std::string_view input) const {
        std::vector<TOKEN> result;
        std::size_t next = 0;
        const auto space = [](char ch){ return ch == ' ' || ch == '\t' || ch == '\r'; };
        while (next < input.size()) {
            while (next < input.size() && space(input[next])) ++next;
            if (next == input.size() || input[next] == '#') break;
            if (result.size() == 67) throw std::invalid_argument("Too many tokens on one line");
            TOKEN token;
            if (input[next] == '&') { token.reference = true; ++next; }
            const bool quoted = next < input.size() && input[next] == '"';
            if (quoted) ++next;
            bool terminated = !quoted;
            while (next < input.size()) {
                char ch = input[next];
                if (!quoted && space(ch)) break;
                ++next;
                if (quoted && ch == '"') { terminated = true; break; }
                if (ch == '\\' && quoted) {
                    if (next == input.size()) throw std::invalid_argument("Incomplete token escape");
                    ch = input[next++];
                    if (ch == 'n') ch = '\n';
                    else if (ch == 'r') ch = '\r';
                    else if (ch == 't') ch = '\t';
                    else if (ch != '"' && ch != '\\') throw std::invalid_argument("Unknown token escape");
                }
                token.text += ch;
            }
            if (!terminated || (next < input.size() && !space(input[next]))) {
                throw std::invalid_argument("Unterminated or joined quoted token");
            }
            if (token.reference && token.text.empty()) throw std::invalid_argument("Empty reference name");
            result.push_back(std::move(token));
        }
        return result;
    }
};
