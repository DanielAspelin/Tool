struct IR {
    enum class Mode { bind, serial, parallel };
    struct Step {
        Mode mode;
        std::string name, command;
        std::vector<TOKEN> parameters;
        bool writeBack;
    };
    static bool name(std::string_view value) {
        return !value.empty() && value.size() <= 40 &&
            value.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") == std::string_view::npos;
    }
    static bool allowed(std::string_view command) {
        constexpr std::string_view methods[]{"echo", "language", "languages", "measure",
            "position", "packages", "catalogue", "timestamp"};
        return std::find(std::begin(methods), std::end(methods), command) != std::end(methods);
    }
    std::vector<Step> operator()(std::istream& source) const {
        std::vector<Step> steps;
        std::set<std::string> defined;
        std::set<std::string> pending;
        std::string line;
        std::size_t lines = 0;
        while (std::getline(source, line)) {
            if (++lines > 256 || line.size() > 4096) throw std::length_error("Script line or file limit exceeded");
            auto tokens = TOKENISER{}(line);
            if (tokens.empty()) continue;
            const auto operation = tokens[0].text;
            const bool bind = operation == "let";
            const bool parallel = operation == "par";
            if ((!bind && !parallel && operation != "run") || tokens.size() < 3 ||
                (bind && (tokens.size() != 3 || tokens[1].reference)) || tokens[0].reference ||
                !name(tokens[1].text) || (!bind && (tokens[2].reference || !allowed(tokens[2].text)))) {
                throw std::invalid_argument("Invalid script instruction at line " + std::to_string(lines));
            }
            if (!parallel) {
                defined.insert(pending.begin(), pending.end());
                pending.clear();
            }
            const auto first = bind ? 2u : 3u;
            for (std::size_t index = first; index < tokens.size(); ++index) {
                if (tokens[index].reference && !defined.count(tokens[index].text)) {
                    throw std::invalid_argument("Reference unavailable at line " + std::to_string(lines));
                }
            }
            auto& destination = parallel ? pending : defined;
            const auto& output = tokens[1];
            if (output.reference != (defined.count(output.text) != 0)) {
                throw std::invalid_argument(output.reference ?
                    "Output reference unavailable at line " + std::to_string(lines) :
                    "Duplicate script binding at line " + std::to_string(lines));
            }
            if ((parallel || !output.reference) && !destination.insert(output.text).second) {
                throw std::invalid_argument("Duplicate parallel output at line " + std::to_string(lines));
            }
            if (parallel && pending.size() > 4) throw std::invalid_argument("A parallel group allows four calls");
            steps.push_back({bind ? Mode::bind : parallel ? Mode::parallel : Mode::serial,
                             tokens[1].text, bind ? "" : tokens[2].text,
                             {tokens.begin() + first, tokens.end()}, output.reference});
            if (steps.size() > 64) throw std::length_error("A script allows 64 instructions");
        }
        return steps;
    }
};
