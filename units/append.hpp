void append(std::string& output, const Record& row, bool details) {
    if (!output.empty()) output += '\n';
    output += row.source;
    output += ':';
    output += row.id;
    output += '\t';
    output += row.name;
    output += '\t';
    output += row.category;
    if (details) {
        if (!row.aliases.empty()) {
            output += "\naliases\t";
            output += row.aliases;
        }
        auto metadata = row.metadata;
        while (!metadata.empty()) {
            output += '\n';
            output += take(metadata, '\t');
        }
        output += "\nsource\t";
        output += sourceFor(row.source).url;
    }
}
