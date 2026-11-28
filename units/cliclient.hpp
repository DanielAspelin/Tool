class CLICLIENT : public CLIENT {
public:
    explicit CLICLIENT(CLISERVER& s): CLIENT(s,"CLI") {}
};
