class RDPSERVER : public SERVER {
public:
    explicit RDPSERVER(DISPATCHER& d): SERVER(d,"RDP") {}
};
