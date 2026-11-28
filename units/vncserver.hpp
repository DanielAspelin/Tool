class VNCSERVER : public SERVER {
public:
    explicit VNCSERVER(DISPATCHER& d): SERVER(d,"VNC") {}
};
