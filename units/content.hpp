struct CONTENT {
    static void validateInventory() {
        for (const auto& entry : INVENTORY::entries) {
            if (!INTEGRITY{}(entry)) throw std::logic_error("Incomplete content item");
            for (const auto& other : INVENTORY::entries) {
                if (&entry != &other && !CONSISTENCY{}(entry, other)) {
                    throw std::logic_error("Duplicate content identity");
                }
            }
        }
    }

    std::string operator()(const Arguments& args) const {
        if (args.empty() || !languages::validDomain(args[0])) {
            throw std::invalid_argument("Usage: language human|programming|machine subject [topic]");
        }
        if (args.size() == 1) return languages::invoke({"sources", args[0]});
        if (args.size() > 3) throw std::invalid_argument("Expected a subject and an optional topic");
        const auto subject = languages::folded(args[1]);
        const auto topic = args.size() == 3 ? languages::folded(args[2]) : "overview";
        static const std::map<std::string, std::string> aliases{{"c++","cpp"},{"js","javascript"},{"webassembly","wasm"}};
        const auto alias = aliases.find(subject);
        const auto& canonical = alias == aliases.end() ? subject : alias->second;
        std::string topics;
        for (const auto& item : INVENTORY::entries) {
            if (item.domain != args[0] || item.subject != canonical) continue;
            if (item.topic == topic) return FINALISATION{}(item);
            if (!topics.empty()) topics += ", ";
            topics += item.topic;
        }
        if (args.size() == 3) {
            throw std::invalid_argument("Topic unavailable" + (topics.empty() ? std::string{} : ". Available: " + topics));
        }
        auto matches = languages::invoke({"search", args[0], args[1], "0", "12"});
        if (matches.empty() && topics.empty()) throw std::invalid_argument("No matching language records");
        if (!topics.empty()) return "Available topics: " + topics + (matches.empty() ? "" : "\n\n" + matches);
        return matches;
    }
};
