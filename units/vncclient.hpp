class VNCCLIENT : public CLIENT {
public:
    explicit VNCCLIENT(VNCSERVER& s): CLIENT(s,"VNC") {}
};
