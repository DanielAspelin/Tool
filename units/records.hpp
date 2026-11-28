const std::vector<Record>& records() {
    static const auto data = [] {
        std::vector<Record> result;
        for (auto block : snapshots) {
            while (!block.empty()) {
                auto line = take(block, '\n');
                Record row;
                row.source = take(line, '\t');
                row.id = take(line, '\t');
                row.name = take(line, '\t');
                row.category = take(line, '\t');
                row.aliases = take(line, '\t');
                row.metadata = line;
                result.push_back(row);
            }
        }
        return result;
    }();
    return data;
}
