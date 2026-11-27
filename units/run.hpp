void run(const std::vector<std::string>& command, const char* limit,
         const std::filesystem::path& output = {}, bool useProxy = false) {
    std::vector<std::string> arguments{"/usr/bin/timeout", "--kill-after=2s", limit};
    arguments.insert(arguments.end(), command.begin(), command.end());
    std::vector<char*> argv;
    for (auto& argument : arguments) argv.push_back(argument.data());
    argv.push_back(nullptr);
    std::vector<std::string> variables{"PATH=/usr/bin:/bin", "LC_ALL=C"};
    if (useProxy) {
        for (const char* name : {"https_proxy", "HTTPS_PROXY", "all_proxy", "ALL_PROXY", "no_proxy", "NO_PROXY"}) {
            if (const char* value = std::getenv(name)) variables.push_back(std::string(name) + "=" + value);
        }
    }
    std::vector<char*> environment;
    for (auto& variable : variables) environment.push_back(variable.data());
    environment.push_back(nullptr);
    posix_spawn_file_actions_t actions;
    if (::posix_spawn_file_actions_init(&actions) != 0) throw std::runtime_error("Cannot initialize process");
    int error = 0;
    if (!output.empty()) {
        error = ::posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, output.c_str(),
                                                   O_WRONLY | O_CREAT | O_EXCL, 0600);
    }
    pid_t pid{};
    if (error == 0) error = ::posix_spawn(&pid, argv.front(), &actions, nullptr, argv.data(), environment.data());
    ::posix_spawn_file_actions_destroy(&actions);
    if (error != 0) throw std::runtime_error("Cannot start " + command.front());

    int status{};
    pid_t waited;
    do { waited = ::waitpid(pid, &status, 0); } while (waited == -1 && errno == EINTR);
    if (waited == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        const auto result = waited != -1 && WIFEXITED(status) ? std::to_string(WEXITSTATUS(status)) : "interrupted";
        throw std::runtime_error(command.front() + " failed (" + result + ")");
    }
}
