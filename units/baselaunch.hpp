class BASELAUNCH {
    using Clock = std::chrono::steady_clock;
    using Registry = std::map<std::string_view, std::map<std::string_view, std::size_t>>;
    const Registry configuration{
        {"TOOL_BASE_SOCKET", {{"limit", sizeof(sockaddr_un::sun_path) - 1}}},
        {"TOOL_BASE_PEER_UID", {{"limit", 20}}},
        {"TOOL_BASE_TARGET", {{"limit", 128}}},
        {"TOOL_BASE_ENVIRONMENT", {{"limit", 64}}},
        {"TOOL_BASE_RELEASE_SHA256", {{"limit", 64}}},
        {"TOOL_BASE_GENERATION", {{"limit", 20}}},
        {"TOOL_BASE_FENCE", {{"limit", 20}}},
        {"TOOL_BASE_COMMAND_ID", {{"limit", 128}}},
    };

public:
    void authorize(std::string_view host, std::string_view mode, const INITIATION& request) const {
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        const auto path = setting("TOOL_BASE_SOCKET");
        if (path.front() != '/') reject("Base socket must use an absolute path");
        std::copy(path.begin(), path.end(), address.sun_path);
        const auto uid = number(setting("TOOL_BASE_PEER_UID"), true);
        if (uid >= std::numeric_limits<uid_t>::max()) reject("Invalid Base peer UID");
        const auto target = identifier(setting("TOOL_BASE_TARGET"));
        const auto environment = identifier(setting("TOOL_BASE_ENVIRONMENT"));
        const auto digest = setting("TOOL_BASE_RELEASE_SHA256");
        if (digest.size() != 64 || digest.find_first_not_of("0123456789abcdef") != std::string_view::npos) {
            reject("Invalid Base release digest");
        }
        const auto generation = number(setting("TOOL_BASE_GENERATION"));
        const auto fence = number(setting("TOOL_BASE_FENCE"));
        const auto command = identifier(setting("TOOL_BASE_COMMAND_ID"));

        std::ostringstream packet;
        packet.imbue(std::locale::classic());
        packet << "tool-base/1\tauthorize-launch\t" << command << '\t' << target << '\t' << environment
               << '\t' << identifier(host) << '\t' << digest << '\t' << generation << '\t' << fence
               << '\t' << mode << '\t' << request.port << '\t' << (request.referral ? "handoff" : "direct");
        exchange(address, static_cast<uid_t>(uid), packet.str(), command);
    }

private:
    [[noreturn]] static void reject(const char* reason) { throw std::runtime_error(reason); }

    std::string_view setting(const char* name) const {
        const auto value = std::getenv(name);
        if (!value || !*value) throw std::runtime_error(std::string("Missing Base configuration: ") + name);
        const std::string_view text(value);
        if (text.size() > configuration.at(name).at("limit")) reject("Base configuration exceeds field limit");
        return text;
    }

    static std::string_view identifier(std::string_view value) {
        constexpr std::string_view allowed =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-";
        if (value.empty() || value.find_first_not_of(allowed) != std::string_view::npos) {
            reject("Invalid Base identifier");
        }
        return value;
    }

    static std::uint64_t number(std::string_view text, bool allowZero = false) {
        std::uint64_t value{};
        const auto end = text.data() + text.size();
        const auto parsed = std::from_chars(text.data(), end, value);
        if (parsed.ec != std::errc{} || parsed.ptr != end || (!allowZero && !value)) {
            reject("Invalid Base numeric binding");
        }
        return value;
    }

    static void waitFor(int socket, short event, Clock::time_point deadline) {
        pollfd descriptor{socket, event, 0};
        const auto remaining = std::chrono::ceil<std::chrono::milliseconds>(deadline - Clock::now()).count();
        if (remaining <= 0 || ::poll(&descriptor, 1, static_cast<int>(remaining)) <= 0 ||
            !(descriptor.revents & event) || Clock::now() >= deadline) {
            reject("Base authorization endpoint did not complete the exchange");
        }
    }

    static void exchange(const sockaddr_un& address, uid_t uid, const std::string& request,
                         std::string_view command) {
        const auto deadline = Clock::now() + std::chrono::seconds(2);
        const DESCRIPTOR socket(::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0));
        if (socket.get() < 0) reject("Cannot create Base authorization socket");
        if (::connect(socket.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
            if (errno != EINPROGRESS) reject("Cannot connect to Base authorization endpoint");
            waitFor(socket.get(), POLLOUT, deadline);
            int error{};
            socklen_t size = sizeof(error);
            if (::getsockopt(socket.get(), SOL_SOCKET, SO_ERROR, &error, &size) != 0 || error != 0) {
                reject("Cannot connect to Base authorization endpoint");
            }
        }
        ucred peer{};
        socklen_t size = sizeof(peer);
        if (::getsockopt(socket.get(), SOL_SOCKET, SO_PEERCRED, &peer, &size) != 0 ||
            size != sizeof(peer) || peer.uid != uid) {
            reject("Base authorization endpoint identity mismatch");
        }
        waitFor(socket.get(), POLLOUT, deadline);
        if (::send(socket.get(), request.data(), request.size(), MSG_NOSIGNAL) !=
            static_cast<ssize_t>(request.size())) {
            reject("Cannot send Base authorization request");
        }
        waitFor(socket.get(), POLLIN, deadline);
        char reply[256];
        const auto received = ::recv(socket.get(), reply, sizeof(reply), MSG_TRUNC);
        const auto expected = "tool-base/1\tallow\t" + std::string(command);
        if (received != static_cast<ssize_t>(expected.size()) ||
            std::string_view(reply, expected.size()) != expected || Clock::now() >= deadline) {
            reject("Base launch authorization denied or invalid");
        }
    }
};
