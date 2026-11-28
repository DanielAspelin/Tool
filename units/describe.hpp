template<class Profile>
std::string describe(const Words& args) {
    if (args.empty() || (args.size() == 1 && args[0] == "status")) return Profile{}();
    throw std::invalid_argument("Expected status");
}
