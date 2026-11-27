struct LAUNCH {
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
        if (args.size() > 3 || args[0] != "start") throw std::invalid_argument("Usage: launch plan | status | ports | inventory | start [auto|desktop|tablet|mobile] [port]");
        const auto arrangement = args.size() > 1 ? args[1] : std::string("auto");
        const auto port = args.size() == 3 ? languages::number(args[2]) : entrancePort;
        if (port > 65535) throw std::invalid_argument("Invalid TCP port");
        return start({ARRANGEMENT{}(arrangement), static_cast<unsigned>(port), port == entrancePort});
    }

private:
    static std::string start(const INITIATION& request) {
#if defined(__linux__) && !defined(__wasi__)
        static std::mutex launchGuard;
        const std::unique_lock<std::mutex> ownership(launchGuard, std::try_to_lock);
        if (!ownership) throw std::runtime_error("Workspace already running");
        PRELAUNCH{}();
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
        constexpr std::string_view modes[]{"auto", "desktop", "tablet", "mobile"};
        const auto arrangement = modes[static_cast<unsigned>(request.arrangement)];
        const unsigned workers = std::clamp(std::thread::hardware_concurrency(), 1u, 4u);
        JOURNAL journal;
        LIFTOFF{}(journal);
        stopRequested.store(false, std::memory_order_relaxed);
        const SIGNALS signals;
        std::vector<std::thread> pool;
        std::mutex startGate;
        std::unique_lock<std::mutex> starting(startGate);
        try {
            for (unsigned index = 0; index < workers; ++index) {
                pool.emplace_back([&]{
                    { const std::lock_guard<std::mutex> admitted(startGate); }
                    while (!stopRequested.load(std::memory_order_relaxed)) {
                        pollfd sockets[]{{listener.get(), POLLIN, 0}, {handoff.get(), POLLIN, 0},
                                         {udp.get(), POLLIN, 0}, {handoffUdp.get(), POLLIN, 0}};
                        if (::poll(sockets, 4, 200) <= 0) continue;
                        for (unsigned index = 0; index < 4; ++index) {
                            if (!(sockets[index].revents & POLLIN)) continue;
                            const auto localPort = index % 2 == 0 ? port : lockedPort;
                            const bool refer = request.referral && index % 2 == 0;
                            if (index >= 2) {
                                try { discovery(sockets[index].fd, localPort, refer, sessions); }
                                catch (const std::exception&) { /* A failed datagram does not stop the listener. */ }
                                continue;
                            }
                            const DESCRIPTOR client(::accept4(sockets[index].fd, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK));
                            if (client.get() < 0) continue;
                            try { HTTP::serve(client.get(), localPort, journal, workers, arrangement, refer, sessions); }
                            catch (const std::exception&) { /* A failed request closes its own socket. */ }
                        }
                    }
                });
            }
            journal.observe(std::string(NARRATION::NARRATIONS[2]));
        } catch (...) {
            stopRequested.store(true, std::memory_order_relaxed);
            starting.unlock();
            for (auto& worker : pool) worker.join();
            throw;
        }
        starting.unlock();
        std::cout << POSTLAUNCH{}(workers) << "\nhttp://127.0.0.1:" << port << '/' << std::endl;
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
        throw std::runtime_error("The local workspace listener is available on Linux");
#endif
    }
};
