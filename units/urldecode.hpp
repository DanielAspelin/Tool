std::string urlDecode(std::string_view input) {
    std::string result;
    for (std::size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '%') {
            if (i + 2 >= input.size()) throw std::invalid_argument("Incomplete URL encoding");
            unsigned value{};
            const auto digits = input.substr(i + 1, 2);
            const auto parsed = std::from_chars(digits.data(), digits.data() + 2, value, 16);
            if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + 2) throw std::invalid_argument("Invalid URL encoding");
            ch = static_cast<char>(value);
            i += 2;
        } else if (ch == '+') {
            ch = ' ';
        }
        if (static_cast<unsigned char>(ch) < 32 || ch == 127) throw std::invalid_argument("Invalid query characters");
        result += ch;
    }
    return result;
}
