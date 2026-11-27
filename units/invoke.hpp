std::string invoke(const TOOL::Arguments& args) {
    const std::string_view command = args.empty() ? std::string_view("summary") : args[0];
    if (command == "summary" && args.size() <= 1) {
        std::string output;
        for (const std::string_view domain : {"human", "programming", "machine"}) {
            std::size_t count = 0;
            for (const auto& source : sources) if (source.domain == domain) count += source.count;
            if (!output.empty()) output += '\n';
            output += std::string(domain) + '\t' + std::to_string(count);
        }
        return output;
    }
    if (command == "sources" && args.size() <= 2) {
        if (args.size() == 2 && !validDomain(args[1])) throw std::invalid_argument("Unknown language domain");
        std::string output;
        for (const auto& source : sources) {
            if (args.size() == 2 && args[1] != source.domain) continue;
            if (!output.empty()) output += '\n';
            output += std::string(source.id) + '\t' + std::string(source.domain) + '\t' +
                std::to_string(source.count) + '\t' + std::string(source.snapshot) + '\t' + std::string(source.url);
        }
        return output;
    }
    if (command == "get" && args.size() == 2) {
        std::string_view key = args[1];
        const auto source = take(key, ':');
        if (key.empty()) throw std::invalid_argument("Expected a source:id key");
        sourceFor(source);
        const auto id = folded(key);
        for (const auto& row : records()) {
            if (row.source == source && folded(row.id) == id) {
                std::string output;
                append(output, row, true);
                return output;
            }
        }
        throw std::invalid_argument("Unknown language catalogue identifier");
    }
    if ((command == "search" || command == "list") && args.size() >= 3 && args.size() <= 5) {
        const auto& domain = args[1];
        if (!validDomain(domain)) throw std::invalid_argument("Unknown language domain");
        const auto offset = args.size() >= 4 ? number(args[3]) : 0;
        const auto limit = args.size() == 5 ? number(args[4]) : 25;
        if (limit == 0 || limit > 100) throw std::invalid_argument("Expected a limit between 1 and 100");
        std::string output;
        const auto& filter = args[2];
        if (command == "list") {
            if (filter != "*" && sourceFor(filter).domain != domain) {
                throw std::invalid_argument("Source belongs to another language domain");
            }
            std::size_t seen = 0, emitted = 0;
            for (const auto& row : records()) {
                if ((filter != "*" && filter != row.source) || sourceFor(row.source).domain != domain) continue;
                if (seen++ < offset) continue;
                append(output, row, false);
                if (++emitted == limit) break;
            }
        } else {
            if (filter.empty() || filter.size() > 256) throw std::invalid_argument("Expected a search term of 1–256 bytes");
            const auto term = folded(filter);
            std::vector<const Record*> matches[3];
            for (const auto& row : records()) {
                if (sourceFor(row.source).domain != domain) continue;
                if (const auto rank = relevance(row, term)) matches[rank - 1].push_back(&row);
            }
            std::size_t seen = 0, emitted = 0;
            for (int rank = 2; rank >= 0; --rank) {
                for (const auto* row : matches[rank]) {
                    if (seen++ < offset) continue;
                    append(output, *row, false);
                    if (++emitted == limit) return output;
                }
            }
        }
        return output;
    }
    throw std::invalid_argument("Usage: languages summary | sources [domain] | get source:id | "
                                "search domain term [offset] [limit] | list domain source-or-* [offset] [limit]");
}
