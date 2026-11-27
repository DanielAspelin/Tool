class TemporaryDirectory {
    std::filesystem::path path_;
public:
    explicit TemporaryDirectory(const std::filesystem::path& parent = std::filesystem::temp_directory_path()) {
        auto pattern = (parent / "tool-XXXXXX").string();
        if (!::mkdtemp(pattern.data())) throw std::runtime_error("Cannot create private workspace");
        path_ = std::move(pattern);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }
    const std::filesystem::path& path() const { return path_; }
};
