class Key {
public:
    Bytes bytes;
    explicit Key(const std::string& path) : bytes(read(path, 32)) {
        if (bytes.size() != 32) {
            OPENSSL_cleanse(bytes.data(), bytes.size());
            throw std::invalid_argument("Expected a key file with exactly 32 raw bytes");
        }
    }
    Key(const Key&) = delete;
    ~Key() { OPENSSL_cleanse(bytes.data(), bytes.size()); }
};
