struct BENCHMARK {
    std::string operator()() const {
        const auto start = std::chrono::steady_clock::now();
        std::size_t tokens = 0;
        for (unsigned index = 0; index < 1000; ++index) tokens += TOKENISER{}("run step echo \"hello\" &value").size();
        const auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
        return std::to_string(tokens) + " tokens in " + std::to_string(time.count()) + " µs";
    }
};
