class SSHSERVER : public SERVER {
public:
    explicit SSHSERVER(DISPATCHER& d): SERVER(d,"SSH") {}
};
