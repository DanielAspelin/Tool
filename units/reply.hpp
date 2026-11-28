struct Reply {
    std::string_view status, type;
    std::string body;
    std::string location{};
};
