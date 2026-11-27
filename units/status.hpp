std::string status(DATAMODE mode, DATASTATE state, std::string_view detail) {
    constexpr std::string_view modes[]{"passive", "runtime"};
    constexpr std::string_view states[]{"ready", "running", "closed"};
    return std::string(modes[static_cast<unsigned>(mode)]) + '\t' +
        std::string(states[static_cast<unsigned>(state)]) + '\t' + std::string(detail);
}
