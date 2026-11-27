struct INTEGRITY {
    bool operator()(const ITEM& item) const { return !item.title.empty() && !item.body.empty(); }
};
