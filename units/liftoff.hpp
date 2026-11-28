struct LIFTOFF { void operator()(JOURNAL& journal) const {
    journal.commit("Start loopback workspace runtime", {NARRATION::NARRATIONS[0], NARRATION::NARRATIONS[1]});
} };
