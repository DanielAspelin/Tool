std::string pack(const TOOL::Arguments& args) {
    if (!validFormat(args[3])) throw std::invalid_argument("Expected text, hex or binary");
    auto payload = read(args[1], payloadLimit);
    const bool sealed = args.size() == 5;
    Bytes nonce(sealed ? 12 : 0);
    Bytes tag(sealed ? 16 : 0);
    if (sealed && RAND_bytes(nonce.data(), static_cast<int>(nonce.size())) != 1) throw std::runtime_error("Cannot generate capsule nonce");
    auto header = std::string(magic) + "\nencoding=" + args[3] + "\nprotection=" + (sealed ? "aes-256-gcm" : "none") +
        "\nbytes=" + std::to_string(payload.size()) + "\nnonce=" + hex(nonce.data(), nonce.size()) + "\n";
    if (sealed) payload = crypt(true, payload, header, Key(args[4]), nonce, tag);
    header += "sha256=" + digest(payload.data(), payload.size()) + "\ntag=" + hex(tag.data(), tag.size()) + "\n\n";
    publish(args[2], header, encode(payload, args[3]));
    return "Capsule created";
}
