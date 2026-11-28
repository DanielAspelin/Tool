class CLISERVER : public SERVER {
public:
    explicit CLISERVER(DISPATCHER& d): SERVER(d,"CLI") {}
};
