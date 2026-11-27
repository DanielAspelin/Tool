class SSHCLIENT : public CLIENT {
public:
    explicit SSHCLIENT(SSHSERVER& s): CLIENT(s,"SSH") {}
};
