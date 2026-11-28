struct SEQUENCE {
    std::string operator()() const {
        std::string result;
        for (auto line : NARRATION::NARRATIONS) {
            if (!result.empty()) result += '\n';
            result += line;
        }
        return result;
    }
};
