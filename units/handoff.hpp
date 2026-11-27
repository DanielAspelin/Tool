class HANDOFF {
    using Clock = std::chrono::steady_clock;
    struct Record {
        Transport transport;
        Clock::time_point deadline;
        bool locked = false;
    };
    std::mutex guard;
    std::map<std::string, Record, std::less<>> records;

    static std::string token() {
        unsigned char bytes[16]{};
        for (std::size_t offset = 0; offset < sizeof(bytes);) {
            const auto count = ::getrandom(bytes + offset, sizeof(bytes) - offset, 0);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) throw std::runtime_error("Cannot generate a handoff ticket");
            offset += static_cast<std::size_t>(count);
        }
        constexpr char hex[] = "0123456789abcdef";
        std::string value;
        for (unsigned char byte : bytes) {
            value += hex[byte >> 4];
            value += hex[byte & 15];
        }
        return value;
    }

public:
    std::string issue(Transport transport) {
        const auto now = Clock::now();
        std::lock_guard<std::mutex> lock(guard);
        for (auto item = records.begin(); item != records.end();) {
            if (item->second.deadline <= now) item = records.erase(item);
            else ++item;
        }
        if (records.size() >= 128) return {};
        const auto key = token();
        if (!records.emplace(key, Record{transport, now + std::chrono::seconds(30)}).second)
            throw std::runtime_error("Cannot allocate a handoff ticket");
        return key;
    }

    bool check(std::string_view key, Transport transport, bool acquire) {
        if (key.size() != 32) return false;
        const auto now = Clock::now();
        std::lock_guard<std::mutex> lock(guard);
        const auto item = records.find(key);
        if (item == records.end() || item->second.transport != transport || item->second.deadline <= now)
            return false;
        if (!acquire) return item->second.locked;
        if (item->second.locked) return false;
        item->second.locked = true;
        item->second.deadline = now + std::chrono::minutes(5);
        return true;
    }
};
