class JOURNAL {
    std::vector<TRANSACTION> transactions;
    std::vector<LOG> logs;
    std::vector<REPORT> reports;
    std::size_t next = 1;
public:
    void commit(std::string action, std::initializer_list<std::string_view> events) {
        if (transactions.size() >= 16) throw std::length_error("Invocation journal is full");
        const auto tx = next++;
        transactions.push_back({tx, reports.empty() ? 0 : reports.back().id, std::move(action)});
        std::vector<std::size_t> links;
        for (const auto event : events) {
            const auto id = next++;
            logs.push_back({id, tx, std::string(event)});
            links.push_back(id);
        }
        const auto id = next++;
        reports.push_back({id, tx, std::move(links)});
    }

    void observe(std::string message) {
        const auto id = next++;
        auto report = reports.back();
        logs.push_back({id, report.transaction, std::move(message)});
        report.logs.push_back(id);
        report.id = next++;
        reports.push_back(std::move(report));
    }

    std::string read(std::string_view kind) const {
        std::ostringstream result;
        if (kind == "transaction") {
            for (const auto& tx : transactions) {
                result << tx.id << "  " << tx.action;
                if (tx.priorReport) result << "  ← prior report " << tx.priorReport;
                result << '\n';
            }
        } else if (kind == "log") {
            for (const auto& log : logs) result << log.id << "  " << log.event << "  ← transaction " << log.transaction << '\n';
        } else if (kind == "report") {
            for (const auto& report : reports) {
                result << report.id << "  transaction " << report.transaction << " · logs";
                for (const auto id : report.logs) result << ' ' << id;
                result << '\n';
            }
        } else {
            throw std::invalid_argument("Expected transaction, log or report");
        }
        auto output = result.str();
        return output.empty() ? "No recorded events in this invocation." : output;
    }
};
