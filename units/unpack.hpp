std::string unpack(const TOOL::Arguments& args, bool inspect) {
    const auto bytes = read(args[1], 2 * payloadLimit + 1024);
    if (bytes.empty()) throw std::invalid_argument("Empty capsule");
    const std::string_view contents(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    auto rest = contents;
    const auto take = [&rest](std::string_view key) {
        const auto line = languages::take(rest, '\n');
        if (line.substr(0, key.size()) != key) throw std::invalid_argument("Invalid capsule metadata");
        return line.substr(key.size());
    };
    if (take("") != magic) throw std::invalid_argument("Unknown capsule version");
    const auto format = take("encoding=");
    const auto protection = take("protection=");
    const auto payloadSize = languages::number(take("bytes="));
    const auto nonceText = take("nonce=");
    const auto aad = contents.substr(0, contents.size() - rest.size());
    const auto checksum = take("sha256=");
    const auto tagText = take("tag=");
    if (!take("").empty() || !validFormat(format) || payloadSize > payloadLimit ||
        (protection != "none" && protection != "aes-256-gcm")) throw std::invalid_argument("Invalid capsule metadata");
    const bool sealed = protection == "aes-256-gcm";
    if (nonceText.size() != (sealed ? 24 : 0) || tagText.size() != (sealed ? 32 : 0) || checksum.size() != 64) {
        throw std::invalid_argument("Invalid capsule authentication fields");
    }
    const auto nonce = unhex(nonceText);
    auto tag = unhex(tagText);
    auto payload = decode(rest, format);
    if (payload.size() != payloadSize || digest(payload.data(), payload.size()) != checksum) {
        throw std::runtime_error("Capsule size or checksum mismatch");
    }
    if (inspect) return std::string(contents.substr(0, contents.size() - rest.size())) +
        (sealed ? "Ciphertext checksum verified; authentication requires the key." : "Payload checksum verified.");
    if (sealed != (args.size() == 4)) throw std::invalid_argument("Supply a key file only for a sealed capsule");
    if (sealed) payload = crypt(false, payload, aad, Key(args[3]), nonce, tag);
    const auto data = payload.empty() ? std::string_view{} :
        std::string_view(reinterpret_cast<const char*>(payload.data()), payload.size());
    publish(args[2], "", data);
    return "Capsule extracted";
}
