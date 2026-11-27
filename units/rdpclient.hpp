class RDPCLIENT : public CLIENT {
public:
    explicit RDPCLIENT(RDPSERVER& s): CLIENT(s,"RDP") {}
};
