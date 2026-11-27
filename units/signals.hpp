class SIGNALS {
    using Handler = void(*)(int);
    Handler interrupt, terminate;
public:
    SIGNALS() : interrupt(std::signal(SIGINT, stop)), terminate(std::signal(SIGTERM, stop)) {}
    ~SIGNALS() {
        if (interrupt != SIG_ERR) std::signal(SIGINT, interrupt);
        if (terminate != SIG_ERR) std::signal(SIGTERM, terminate);
    }
};
