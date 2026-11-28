std::string folded(std::string_view value) {
    std::string result(value);
    for (auto& ch : result) {
        if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch - 'A' + 'a');
    }
    return result;
}
