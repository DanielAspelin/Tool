struct POSTLAUNCH { std::string operator()(unsigned workers) const {
    return std::string(NARRATION::NARRATIONS[2]) + "\nRequest workers: " + std::to_string(workers);
} };
