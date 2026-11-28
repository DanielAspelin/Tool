class LAUNCHCONTROL {
    using Registry = std::map<std::string, std::map<std::string, std::string>>;
    const Registry commands{
        {"inventory", {{"usage", "inventory"}}},
        {"managed-start", {{"usage", "managed-start [auto|desktop|tablet|mobile] [port]"}}},
        {"plan", {{"usage", "plan"}}},
        {"ports", {{"usage", "ports"}}},
        {"preflight", {{"usage", "preflight [auto|desktop|tablet|mobile] [port]"}}},
        {"start", {{"usage", "start [auto|desktop|tablet|mobile] [port]"}}},
        {"status", {{"usage", "status"}}},
    };
    static constexpr std::string_view modes[]{"auto", "desktop", "tablet", "mobile"};

public:
    std::string operator()(const Arguments& args) const {
        if (args.empty() || (args.size() == 1 && args[0] == "plan")) return SEQUENCE{}();
        if (args.size() == 1 && args[0] == "inventory") {
            std::string result;
            for (const auto& item : INVENTORY::entries) {
                if (!result.empty()) result += '\n';
                result += std::string(item.domain) + '\t' + std::string(item.subject) + '\t' +
                    std::string(item.topic) + '\t' + std::string(item.title);
            }
            return result;
        }
        if (args.size() == 1 && args[0] == "ports") {
            return "Gateway 80/443 → local TCP/UDP 9966 → local handoff TCP/UDP 6699";
        }
        if (args.size() == 1 && args[0] == "status") return status(DATAMODE::passive, DATASTATE::ready, "listener disabled until launch start");
        if (args.size() <= 3 && args[0] == "preflight") return preflight(initiation(args));
        if (args.size() <= 3 && args[0] == "start") return start(initiation(args));
        if (args.size() <= 3 && args[0] == "managed-start") return start(initiation(args), true);
        std::string usage = "Usage: launch";
        for (const auto& command : commands) usage += " | " + command.second.at("usage");
        throw std::invalid_argument(usage);
    }

private:
#if defined(__linux__) && !defined(__wasi__)
    static std::string launchHost() {
        char hostname[256]{};
        if (::gethostname(hostname, sizeof(hostname) - 1) != 0 || hostname[0] == '\0') {
            throw std::runtime_error("Cannot identify the launch host");
        }
        const std::string detected(hostname);
        if (const char* expected = std::getenv("TOOL_LAUNCH_HOST"); expected && detected != expected) {
            throw std::runtime_error("Launch host does not match TOOL_LAUNCH_HOST");
        }
        return detected;
    }

    static std::string prepare() {
        auto host = launchHost();
        PRELAUNCH{}();
        return host;
    }

    static void serveReadySockets(const pollfd (&listeners)[4], unsigned port, bool referral,
                                  const JOURNAL& journal, unsigned workers, std::string_view arrangement,
                                  HANDOFF& sessions) {
        pollfd sockets[]{listeners[0], listeners[1], listeners[2], listeners[3]};
        if (::poll(sockets, 4, 200) <= 0 || stopRequested.load(std::memory_order_relaxed)) return;
        for (unsigned index = 0; index < 4; ++index) {
            if (!(sockets[index].revents & POLLIN)) continue;
            const auto localPort = index % 2 == 0 ? port : lockedPort;
            const bool refer = referral && index % 2 == 0;
            try {
                if (index >= 2) {
                    discovery(sockets[index].fd, localPort, refer, sessions);
                    continue;
                }
                const DESCRIPTOR client(::accept4(sockets[index].fd, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK));
                if (client.get() >= 0) {
                    HTTP::serve(client.get(), localPort, journal, workers, arrangement, refer, sessions);
                }
            } catch (const std::exception&) {
                // A failed request leaves the listeners available.
            }
        }
    }
#endif

    static INITIATION initiation(const Arguments& args) {
        const auto arrangement = ARRANGEMENT{}(args.size() > 1 ? args[1] : "auto");
        const auto port = args.size() == 3 ? languages::number(args[2]) : entrancePort;
        if (port > 65535) throw std::invalid_argument("Invalid TCP port");
        return {arrangement, static_cast<unsigned>(port), port == entrancePort};
    }

    static std::string preflight(const INITIATION& request) {
#if defined(__linux__) && !defined(__wasi__)
        const auto host = prepare();
        const auto mode = modes[static_cast<unsigned>(request.arrangement)];
        std::string report = status(DATAMODE::passive, DATASTATE::ready, "launch parameters and inventory verified");
        report += "\nLaunch host: " + host;
        report += "\nMode: " + std::string(mode);
        report += "\nBind: 127.0.0.1:" + std::to_string(request.port);
        if (request.port == 0) report += " (assigned on start)";
        report += request.referral ? "\nHandoff: 127.0.0.1:6699" : "\nHandoff: disabled";
        return report;
#else
        (void)request;
        throw std::runtime_error("The local workspace listener is available on Linux");
#endif
    }

    static std::string start(const INITIATION& request, bool managed = false) {
#if defined(__linux__) && !defined(__wasi__)
        static std::mutex launchGuard;
        const std::unique_lock<std::mutex> ownership(launchGuard, std::try_to_lock);
        if (!ownership) throw std::runtime_error("Workspace already running");
        const auto host = prepare();
        const auto arrangement = modes[static_cast<unsigned>(request.arrangement)];
        const auto policy = std::getenv("TOOL_BASE_MANAGED");
        if (policy && std::string_view(policy) != "1") throw std::runtime_error("Invalid TOOL_BASE_MANAGED policy");
        if (managed || policy) BASELAUNCH{}.authorize(host, arrangement, request);
        std::cout << NARRATION::NARRATIONS[0] << std::endl;
        stopRequested.store(false, std::memory_order_relaxed);
        const SIGNALS signals;
        auto listener = bindSocket(SOCK_STREAM, request.port);
        sockaddr_in address{};
        socklen_t addressSize = sizeof(address);
        if (::getsockname(listener.get(), reinterpret_cast<sockaddr*>(&address), &addressSize) != 0) {
            throw std::runtime_error("Cannot determine listening port");
        }
        const unsigned port = ntohs(address.sin_port);
        auto udp = bindSocket(SOCK_DGRAM, port);
        auto handoff = request.referral ? bindSocket(SOCK_STREAM, lockedPort) : DESCRIPTOR(-1);
        auto handoffUdp = request.referral ? bindSocket(SOCK_DGRAM, lockedPort) : DESCRIPTOR(-1);
        HANDOFF sessions;
        const unsigned workers = std::clamp(std::thread::hardware_concurrency(), 1u, 4u);
        JOURNAL journal;
        const pollfd listeners[]{{listener.get(), POLLIN, 0}, {handoff.get(), POLLIN, 0},
                                 {udp.get(), POLLIN, 0}, {handoffUdp.get(), POLLIN, 0}};
        std::vector<std::thread> pool;
        std::mutex startGate;
        std::unique_lock<std::mutex> starting(startGate);
        try {
            for (unsigned index = 0; index < workers; ++index) {
                pool.emplace_back([&]{
                    { const std::lock_guard<std::mutex> admitted(startGate); }
                    while (!stopRequested.load(std::memory_order_relaxed)) {
                        serveReadySockets(listeners, port, request.referral, journal, workers, arrangement, sessions);
                    }
                });
            }
            if (!stopRequested.load(std::memory_order_relaxed)) {
                LIFTOFF{}(journal);
                std::cout << NARRATION::NARRATIONS[1] << std::endl;
                journal.observe(std::string(NARRATION::NARRATIONS[2]));
                std::cout << POSTLAUNCH{}(workers) << "\nLaunch host: " << host
                          << "\nhttp://127.0.0.1:" << port << '/' << std::endl;
            }
        } catch (...) {
            stopRequested.store(true, std::memory_order_relaxed);
            starting.unlock();
            for (auto& worker : pool) worker.join();
            throw;
        }
        starting.unlock();
        for (auto& worker : pool) worker.join();
        listener.reset();
        udp.reset();
        handoff.reset();
        handoffUdp.reset();
        journal.commit("Stop loopback workspace runtime", {"Listener closed"});
        return status(DATAMODE::passive, DATASTATE::closed, "listener closed") +
            "\n\nTransactions\n" + journal.read("transaction") +
            "\nLogs\n" + journal.read("log") + "\nReports\n" + journal.read("report");
#else
        (void)request;
        (void)managed;
        throw std::runtime_error("The local workspace listener is available on Linux");
#endif
    }
};
