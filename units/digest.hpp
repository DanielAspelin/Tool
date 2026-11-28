std::string digest(const unsigned char* payload, std::size_t size) {
    unsigned char hash[32];
    unsigned length{};
    if (EVP_Digest(payload, size, hash, &length, EVP_sha256(), nullptr) != 1 || length != sizeof(hash)) {
        throw std::runtime_error("Cannot compute SHA-256");
    }
    return hex(hash, sizeof(hash));
}
