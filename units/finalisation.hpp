struct FINALISATION {
    std::string operator()(const ITEM& item) const {
        std::string result(item.title);
        result += "\n\n";
        result += item.body;
        if (!item.source.empty()) result += "\n\nSource: " + std::string(item.source);
        return result;
    }
};
