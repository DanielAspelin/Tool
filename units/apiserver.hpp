class APISERVER : public SERVER {
public:
    explicit APISERVER(DISPATCHER& d): SERVER(d,"API") {}
};
