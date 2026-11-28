void publish(const std::filesystem::path& target, std::string_view header, std::string_view payload) {
    const auto absolute = std::filesystem::absolute(target);
    const native::TemporaryDirectory temporary(absolute.parent_path());
    const auto staged = temporary.path() / "output";
    std::ofstream out(staged, std::ios::binary);
    out.write(header.data(), static_cast<std::streamsize>(header.size()));
    if (!payload.empty()) out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    out.close();
    if (!out) throw std::runtime_error("Cannot write capsule output");
    std::filesystem::permissions(staged, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
    std::filesystem::create_hard_link(staged, absolute);
}
