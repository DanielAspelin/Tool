std::string position(const Arguments& args) {
    if (args.size() < 2 || args.size() > 3) throw std::invalid_argument("Usage: position latitude longitude [altitude-m]");
    const auto latitude = numeric(args[0]);
    const auto longitude = numeric(args[1]);
    const auto altitude = args.size() == 3 ? numeric(args[2]) : 0;
    if (!POSITION{}(latitude, longitude, altitude)) throw std::invalid_argument("Coordinates outside accepted range");
    return "Coordinates validated";
}
