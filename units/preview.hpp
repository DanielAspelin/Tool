std::string preview(const std::vector<IR::Step>& steps) {
    std::string result;
    for (const auto& step : steps) {
        if (!result.empty()) result += '\n';
        const char* mode = step.mode == IR::Mode::bind ? "let" : step.mode == IR::Mode::parallel ? "par" : "run";
        result += std::string(mode) + ' ';
        if (step.writeBack) result += '&';
        result += step.name;
        if (!step.command.empty()) result += ' ' + step.command;
        if (!step.parameters.empty()) result += ' ' + DETOKENISER{}(step.parameters);
    }
    return result;
}
