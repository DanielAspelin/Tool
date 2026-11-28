Bytes crypt(bool seal, const Bytes& payload, std::string_view header, const Key& key,
            const Bytes& nonce, Bytes& tag) {
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    Bytes output(payload.size() + 16);
    int size{}, finalSize{};
    const auto* metadata = reinterpret_cast<const unsigned char*>(header.data());
    if (!context ||
        EVP_CipherInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, key.bytes.data(), nonce.data(), seal ? 1 : 0) != 1 ||
        EVP_CipherUpdate(context.get(), nullptr, &size, metadata, static_cast<int>(header.size())) != 1) {
        throw std::runtime_error("Cannot initialize capsule cipher");
    }
    size = 0;
    if (!payload.empty() && EVP_CipherUpdate(context.get(), output.data(), &size, payload.data(), static_cast<int>(payload.size())) != 1) {
        throw std::runtime_error("Capsule cipher operation failed");
    }
    if (!seal && EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_AEAD_SET_TAG, 16, tag.data()) != 1) {
        throw std::runtime_error("Cannot read capsule tag");
    }
    if (EVP_CipherFinal_ex(context.get(), output.data() + size, &finalSize) != 1) {
        throw std::runtime_error("Capsule authentication failed");
    }
    if (seal && EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_AEAD_GET_TAG, 16, tag.data()) != 1) {
        throw std::runtime_error("Cannot retrieve capsule tag");
    }
    output.resize(static_cast<std::size_t>(size + finalSize));
    return output;
}
