unsigned relevance(const Record& row, const std::string& query) {
    const auto id = folded(row.id);
    const auto name = folded(row.name);
    if (id == query) return 3;
    if (name == query) return 2;
    auto aliases = row.aliases;
    while (!aliases.empty()) {
        if (folded(take(aliases, '|')) == query) return 2;
    }
    if (name.find(query) != std::string::npos || id.find(query) != std::string::npos ||
        folded(row.aliases).find(query) != std::string::npos) return 1;
    return 0;
}
