struct CONSISTENCY {
    bool operator()(const ITEM& a, const ITEM& b) const {
        return a.domain != b.domain || a.subject != b.subject || a.topic != b.topic;
    }
};
