struct ABBREVIATION {
    bool operator()(const Package& package, std::string_view abbreviation) const {
        constexpr std::string_view separator = " · ";
        auto remaining = package.abbreviations;
        const auto token = languages::folded(abbreviation);
        while (!remaining.empty()) {
            const auto next = remaining.find(separator);
            if (languages::folded(remaining.substr(0, next)) == token) return true;
            if (next == std::string_view::npos) break;
            remaining.remove_prefix(next + separator.size());
        }
        return false;
    }
};
