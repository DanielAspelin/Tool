class BASECOVERAGE {
    struct Requirement {
        std::string_view code;
        std::string_view owner;
        std::string_view control;
        std::string_view requiredEvidence;
    };

    struct Perimeter {
        std::string_view name;
        std::string_view classification;
    };

    const std::array<Requirement, 8> requirements{{
        {"BCC-01", "Information Control", "Inventory provenance", "Source, observation time and verification state for each asset field"},
        {"BCC-02", "Information Control", "Classification retention", "Preserved perimeter designation and recorded approval for any change"},
        {"BCC-03", "Systems Control", "Target enrollment", "Registered target and environment matched to independently verified host identity"},
        {"BCC-04", "Systems Control", "Release integrity", "Approved signing identity, verified release digest and native preflight result"},
        {"BCC-05", "Systems Control", "Prepositioning lock and version count", "Lease, deployment generation, fencing token and result of same-attempt retry"},
        {"BCC-06", "Systems Control", "Managed launch and routing", "Base verifier decision and Traffic Control policy bound to the same generation"},
        {"BCC-07", "Systems Control", "Acceptance and recovery", "Serving release, health checks, recovery release and routing policy"},
        {"BCC-08", "Information Control", "Audit traceability", "Correlated approval, policy revision, deployment generation and outcome"},
    }};

    const std::array<Perimeter, 4> perimeters{{
        {"Native staging", "Installation and verification perimeter"},
        {"Existing Tool Base", "Reference installation"},
        {"Browser production", "Browser deployment perimeter"},
        {"Project System Headquarters", "Intended native deployment perimeter. Registered target pending identification"},
    }};

public:
    std::string operator()(const TOOL::Arguments& args) const {
        if (args.size() == 1) return report({});
        if (args.size() != 2) throw std::invalid_argument(usage());
        const auto& selector = args[1];
        if (selector == "classification") return classifications();
        if (selector == "information") return report("Information Control");
        if (selector == "systems") return report("Systems Control");
        for (const auto& requirement : requirements) {
            if (requirement.code == selector) return report(selector);
        }
        throw std::invalid_argument(usage());
    }

private:
    static const char* usage() {
        return "Usage: coverage base [information|systems|classification|BCC-01..BCC-08]";
    }

    std::string report(std::string_view selector) const {
        std::ostringstream output;
        output << "Code\tOwner\tControl\tRequired evidence\n";
        for (const auto& requirement : requirements) {
            if (selector.empty() || selector == requirement.owner || selector == requirement.code) {
                output << requirement.code << '\t' << requirement.owner << '\t' << requirement.control
                       << '\t' << requirement.requiredEvidence << '\n';
            }
        }
        return output.str();
    }

    std::string classifications() const {
        std::ostringstream output;
        output << "Perimeter\tBase Designated Classification\n";
        for (const auto& perimeter : perimeters) {
            output << perimeter.name << '\t' << perimeter.classification << '\n';
        }
        return output.str();
    }
};
