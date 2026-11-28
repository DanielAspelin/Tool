class LINGUIST {
public:
    static std::string invoke(const TOOL::Arguments& args) {
        if (args.empty() || (args.size() == 1 && args[0] == "status")) {
            return "Linguist local language and UTF-8 checks; tool-linguist/1; peer opt-in";
        }
        if (args.size() == 2 && args[0] == "language") {
            const auto& tag = args[1];
            if (tag.empty() || tag.size() > 35 || tag.front() == '-' || tag.back() == '-' ||
                tag.find("--") != std::string::npos ||
                tag.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-") != std::string::npos) {
                return "invalid-language-tag-syntax";
            }
            return "valid-language-tag-syntax";
        }
        if (args.size() == 2 && args[0] == "utf8") {
            try { (void)STRING(args[1]); }
            catch (const std::invalid_argument&) { return "invalid-utf8"; }
            return "valid-utf8";
        }
#if defined(__linux__) && !defined(__wasi__)
        if (args.size() == 4 && args[0] == "peer" && (args[2] == "language" || args[2] == "utf8")) {
            if (!std::filesystem::path(args[1]).is_absolute()) {
                throw std::invalid_argument("Expected an absolute path to the trusted Linguist executable");
            }
            const auto peer = native::regularFile(args[1]).string();
            const native::TemporaryDirectory temporary;
            const auto status = temporary.path() / "status";
            native::run({peer, "status"}, "5s", status);
            std::string handshake;
            std::getline(std::ifstream(status), handshake);
            if (handshake.rfind("Linguist experimental; protocol=tool-linguist/1 (proposed); instance=", 0) != 0) {
                throw std::runtime_error("Linguist protocol mismatch");
            }
            const auto output = temporary.path() / "result";
            native::run({peer, args[2], args[3]}, "5s", output);
            std::string result;
            std::ifstream response(output);
            std::getline(response, result);
            const auto suffix = args[2] == "language" ? "language-tag-syntax" : "utf8";
            if ((result != std::string("valid-") + suffix && result != std::string("invalid-") + suffix) ||
                response.peek() != std::char_traits<char>::eof()) {
                throw std::runtime_error("Unexpected Linguist response");
            }
            return result;
        }
#endif
        throw std::invalid_argument("Usage: linguist status | language tag | utf8 text | peer absolute-binary language|utf8 input");
    }
};
