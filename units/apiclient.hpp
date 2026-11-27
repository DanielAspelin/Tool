class APICLIENT : public CLIENT {
public:
    explicit APICLIENT(APISERVER& s): CLIENT(s,"API") {}
};
