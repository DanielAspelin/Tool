DESCRIPTOR bindSocket(int type, unsigned port) {
    DESCRIPTOR sock(::socket(AF_INET, type | SOCK_CLOEXEC | SOCK_NONBLOCK, 0));
    if (sock.get() < 0) throw std::runtime_error("Cannot create loopback socket");
    if (type == SOCK_STREAM) {
        const int allowReuse = 1;
        ::setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, &allowReuse, sizeof(allowReuse));
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<uint16_t>(port));
    if (::bind(sock.get(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
        (type == SOCK_STREAM && ::listen(sock.get(), 16) != 0)) {
        throw std::runtime_error("Cannot start loopback listener on port " + std::to_string(port));
    }
    return sock;
}
