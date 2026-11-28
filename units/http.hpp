struct HTTP {
    static Reply plain(std::string_view status, std::string body) {
        return {status, "text/plain; charset=utf-8", std::move(body)};
    }

    static Reply route(std::string_view target, const JOURNAL& journal, unsigned workers,
                       std::string_view arrangement, bool refer) {
        const auto path = languages::take(target, '?');
        if (path == "/handoff" && refer) {
            return {"307 Temporary Redirect", "text/plain; charset=utf-8", "Continue at Tool's local handoff port.",
                    "http://127.0.0.1:" + std::to_string(lockedPort) + "/"};
        }
        if (path == "/") return {"200 OK", "text/html; charset=utf-8", std::string(workspaceHtml)};
        if (path == "/workspace.css") return {"200 OK", "text/css; charset=utf-8", std::string(workspaceCss)};
        if (path == "/workspace.js") return {"200 OK", "text/javascript; charset=utf-8", std::string(workspaceJs)};
        if (path == "/api/runtime") return plain("200 OK", status(DATAMODE::runtime, DATASTATE::running, arrangement));
        if (path == "/api/launch") return plain("200 OK", SEQUENCE{}() + "\nRequest workers: " + std::to_string(workers));
        if (path == "/api/packages") return plain("200 OK", packageInfo({}));
        if (path != "/api/language" && path != "/api/journal") return plain("404 Not Found", "Route unavailable");
        std::map<std::string_view, std::string> params;
        while (!target.empty()) {
            auto parameter = languages::take(target, '&');
            auto key = languages::take(parameter, '=');
            if (!params.emplace(key, urlDecode(parameter)).second) throw std::invalid_argument("Duplicate parameter");
        }
        if (path == "/api/journal") return plain("200 OK", journal.read(params["kind"]));
        Arguments arguments{params["domain"], params["subject"]};
        if (!params["topic"].empty()) arguments.push_back(params["topic"]);
        return plain("200 OK", CONTENT{}(arguments));
    }

    static Reply handle(std::string_view request, unsigned port, const JOURNAL& journal,
                        unsigned workers, std::string_view arrangement, bool refer, HANDOFF& handoff) {
        const auto lineEnd = request.find("\r\n");
        auto line = request.substr(0, lineEnd);
        request.remove_prefix(lineEnd + 2);
        const auto method = languages::take(line, ' ');
        const auto target = languages::take(line, ' ');
        if (line != "HTTP/1.1" || target.empty() || target[0] != '/' || target.size() > 1024) {
            return plain("400 Bad Request", "Invalid request");
        }
        const auto address = "127.0.0.1:" + std::to_string(port);
        const auto localhost = "localhost:" + std::to_string(port);
        bool host = false;
        while (!request.empty()) {
            const auto end = request.find("\r\n");
            if (end == 0 || end == std::string_view::npos) break;
            auto header = request.substr(0, end);
            request.remove_prefix(end + 2);
            const auto name = languages::folded(languages::take(header, ':'));
            while (!header.empty() && (header.front() == ' ' || header.front() == '\t')) header.remove_prefix(1);
            if (name == "host") {
                if (host || (header != address && header != localhost)) return plain("403 Forbidden", "Host unavailable");
                host = true;
            }
        }
        if (!host) return plain("403 Forbidden", "Host unavailable");
        if (method == "POST" && refer && target == "/session/referral") {
            const auto ticket = handoff.issue(Transport::tcp);
            if (ticket.empty()) return plain("503 Service Unavailable", "Handoff capacity reached");
            return plain("200 OK", "TOOL/1 REFERRAL " + std::to_string(lockedPort) + " " + ticket);
        }
        if (port == lockedPort && !refer) {
            constexpr std::string_view lockPath = "/session/lock?ticket=";
            constexpr std::string_view statusPath = "/session/status?ticket=";
            const bool acquire = method == "POST" && target.substr(0, lockPath.size()) == lockPath;
            const bool inspect = method == "GET" && target.substr(0, statusPath.size()) == statusPath;
            if (acquire || inspect) {
                const auto key = target.substr(acquire ? lockPath.size() : statusPath.size());
                if (handoff.check(key, Transport::tcp, acquire)) return plain("200 OK", "TOOL/1 LOCKED");
                return plain("403 Forbidden", "Handoff ticket unavailable");
            }
        }
        if (method != "GET") return plain("405 Method Not Allowed", "Read-only workspace");
        try { return route(target, journal, workers, arrangement, refer); }
        catch (const std::invalid_argument& error) { return plain("400 Bad Request", error.what()); }
    }

    static bool wait(int fd, short events, std::chrono::steady_clock::time_point deadline) {
        using namespace std::chrono;
        const auto time = duration_cast<milliseconds>(deadline - steady_clock::now()).count();
        if (time <= 0) return false;
        pollfd state{fd, events, 0};
        return ::poll(&state, 1, static_cast<int>(time)) > 0 && (state.revents & events);
    }

    static void serve(int client, unsigned port, const JOURNAL& journal,
                      unsigned workers, std::string_view arrangement, bool refer, HANDOFF& handoff) {
        using namespace std::chrono;
        const auto deadline = steady_clock::now() + seconds(2);
        std::string input;
        char buffer[2048];
        while (input.size() < 8192 && input.find("\r\n\r\n") == std::string::npos) {
            if (!wait(client, POLLIN, deadline)) return;
            const auto count = ::recv(client, buffer, sizeof(buffer), 0);
            if (count <= 0) return;
            input.append(buffer, static_cast<std::size_t>(count));
        }
        if (input.size() > 8192 || input.find("\r\n\r\n") == std::string::npos) return;
        const auto reply = handle(input, port, journal, workers, arrangement, refer, handoff);
        const std::string message = "HTTP/1.1 " + std::string(reply.status) + "\r\nContent-Type: " +
            std::string(reply.type) + "\r\nContent-Length: " + std::to_string(reply.body.size()) +
            "\r\nConnection: close\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff"
            "\r\nContent-Security-Policy: default-src 'self'; frame-ancestors 'none'; base-uri 'none'" +
            (reply.location.empty() ? std::string{} : "\r\nLocation: " + reply.location) +
            "\r\n\r\n" + reply.body;
        std::size_t sent = 0;
        while (sent < message.size() && wait(client, POLLOUT, deadline)) {
            const auto count = ::send(client, message.data() + sent, message.size() - sent, MSG_NOSIGNAL);
            if (count <= 0) return;
            sent += static_cast<std::size_t>(count);
        }
    }
};
