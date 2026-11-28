void discovery(int socket, unsigned port, bool refer, HANDOFF& handoff) {
    sockaddr_in client{};
    socklen_t addressSize = sizeof(client);
    char request[64];
    const auto size = ::recvfrom(socket, request, sizeof(request), MSG_TRUNC,
                                 reinterpret_cast<sockaddr*>(&client), &addressSize);
    if (size <= 0 || static_cast<std::size_t>(size) > sizeof(request) ||
        client.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) return;
    const std::string_view command(request, static_cast<std::size_t>(size));
    std::string reply;
    if (command == "TOOL/1") {
        reply = std::string("TOOL/1 ") + (refer ? "REFERRAL " : "LOCAL ") +
            std::to_string(refer ? lockedPort : port);
    } else if (refer && command == "TOOL/1 OPEN") {
        const auto ticket = handoff.issue(Transport::udp);
        reply = ticket.empty() ? "TOOL/1 BUSY" : "TOOL/1 REFERRAL " + std::to_string(lockedPort) + " " + ticket;
    } else if (port == lockedPort) {
        constexpr std::string_view lockCommand = "TOOL/1 LOCK ";
        constexpr std::string_view statusCommand = "TOOL/1 STATUS ";
        const bool acquire = command.substr(0, lockCommand.size()) == lockCommand;
        const bool inspect = command.substr(0, statusCommand.size()) == statusCommand;
        if (!acquire && !inspect) return;
        const auto key = command.substr(acquire ? lockCommand.size() : statusCommand.size());
        reply = handoff.check(key, Transport::udp, acquire) ? "TOOL/1 LOCKED" : "TOOL/1 INVALID";
    } else {
        return;
    }
    ::sendto(socket, reply.data(), reply.size(), MSG_NOSIGNAL,
             reinterpret_cast<sockaddr*>(&client), addressSize);
}
