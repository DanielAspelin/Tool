std::string invoke(const TOOL::Arguments& args) {
#if defined(TOOL_CRYPTO) && defined(__linux__) && !defined(__wasi__)
    if ((args.size() == 4 || args.size() == 5) && args[0] == "pack") return pack(args);
    if (args.size() == 2 && args[0] == "inspect") return unpack(args, true);
    if ((args.size() == 3 || args.size() == 4) && args[0] == "unpack") return unpack(args, false);
    throw std::invalid_argument("Usage: artifact pack input output text|hex|binary [key-file] | inspect capsule | unpack capsule output [key-file]");
#else
    (void)args;
    throw std::runtime_error("Build the Linux native target with OpenSSL to use artifact capsules");
#endif
}
