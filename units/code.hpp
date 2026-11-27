class CODE {
public:
    using ToolLocalRegistry = std::map<std::string,std::map<std::string,std::string>>;
    class SPECIFIER {
    public:
        static bool transversion(ToolLocalRegistry& specification,const std::string& category,const std::string& identity,const std::string& revision){
            if(category.empty()||identity.empty()||revision.empty())return false;
            const auto key=category+":"+identity;
            auto& record=specification[key];
            const auto previous=record.find("version");
            if(previous!=record.end()&&previous->second==revision)return false;
            if(previous!=record.end())record["previous_version"]=previous->second;
            record["version"]=revision;
            return true;
        }
        static void generate(ToolLocalRegistry& specification,const std::string& name,const ToolLocalRegistry& array){
            for(const auto& entry:array){
                auto& record=specification["array:"+name+":"+entry.first];
                record=entry.second;
            }
        }
        static std::uint64_t index(std::atomic<std::uint64_t>& counter){
            auto current=counter.load();
            bool complete=false;
            while(!complete){advance(counter,current,complete);}
            return current+1;
        }
        static void advance(std::atomic<std::uint64_t>& counter,std::uint64_t& current,bool& complete){
            if(current==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Instance index exhausted");
            complete=counter.compare_exchange_weak(current,current+1);
        }
    };
private:
    inline static std::atomic<std::uint64_t> specificationInstanceCounter{0};
    static ToolLocalRegistry initialSpecification(){
        return {{"class",{{"identity","CODE"},{"version","0.1.0"},{"activation","invocation-bound"}}},
                {"array:ARCHITECTURE",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:DESIGN",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:VERSION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:INFORMATION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:DESCRIPTION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:MODEL",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:COMMANDS",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:SPECIFICATION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"function:COMMANDER.define",{{"version","0.1.0"}}},
                {"function:COMMANDER.inspect",{{"version","0.1.0"}}},
                {"function:SPECIFIER.transversion",{{"version","0.1.0"}}},
                {"function:SPECIFIER.generate",{{"version","0.1.0"}}},
                {"function:SPECIFIER.index",{{"version","0.1.0"}}}};
    }
    ToolLocalRegistry VERSION{{"class",{{"version","0.1.0"}}}};
    ToolLocalRegistry INFORMATION{{"lifecycle",{{"ownership","invoker"},{"runtime","optional"}}}};
    ToolLocalRegistry DESCRIPTION{{"structure",{{"architecture","reciprocal"},{"design","invocation-bound"}}}};
    ToolLocalRegistry SPECIFICATION=initialSpecification();
    std::uint64_t specificationInstanceIndex=SPECIFIER::index(specificationInstanceCounter);
public:

    private:
    ToolLocalRegistry ARCHITECTURE{{"structure",{{"ownership","invoker"},{"connectivity","reciprocal"}}}};
    ToolLocalRegistry DESIGN{{"behavior",{{"activation","invocation-bound"},{"runtime","optional"}}}};
    public:
    class COMMANDER {
    public:
        static void define(ToolLocalRegistry& commands,const std::string& id,const std::string& key,const std::string& value){commands[id][key]=value;}
        static const ToolLocalRegistry::mapped_type* inspect(const ToolLocalRegistry& commands,const std::string& id){auto it=commands.find(id);return it==commands.end()?nullptr:&it->second;}
    };
private:
    ToolLocalRegistry MODEL{{"CODE",{{"class","CODE"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{
        {"status", {{"kind", "metadata"}, {"owner", "CODE"}}},
        {"validate", {{"kind", "source record"}, {"owner", "CODE"}}},
        {"self-test", {{"kind", "verification"}, {"owner", "CODE"}}},
    };
public:
    ToolLocalRegistry testifyIntegration() const {
        ToolLocalRegistry testimony;
        SPECIFIER::generate(testimony,"ARCHITECTURE",ARCHITECTURE);
        SPECIFIER::generate(testimony,"DESIGN",DESIGN);
        SPECIFIER::generate(testimony,"MODEL",MODEL);
        SPECIFIER::generate(testimony,"VERSION",VERSION);
        testimony["participant"]=SPECIFICATION.at("class");
        testimony["integration"]["status"]="declared; reciprocal verification required";
        return testimony;
    }
    ToolLocalRegistry specificationSnapshot() const {
        auto result=SPECIFICATION;
        SPECIFIER::generate(result,"ARCHITECTURE",ARCHITECTURE);
        SPECIFIER::generate(result,"DESIGN",DESIGN);
        SPECIFIER::generate(result,"VERSION",VERSION);
        SPECIFIER::generate(result,"INFORMATION",INFORMATION);
        SPECIFIER::generate(result,"DESCRIPTION",DESCRIPTION);
        SPECIFIER::generate(result,"MODEL",MODEL);
        SPECIFIER::generate(result,"COMMANDS",COMMANDS);
        result["instance"]["index"]=std::to_string(specificationInstanceIndex);
        result["instance"]["spid"]=result["class"]["identity"]+":"+std::to_string(specificationInstanceIndex);
        for(const auto* name: {"instantiate","initialize","load","run","execute","finalize","terminate","clean","flush","reconcile","automate","generate","copy","dump","delete","create","construct","reconstruct","deliver","serve","obtain","retain","resume","try","catch","throw","retry","request","respond","negotiate","pidkill","destroy","cleanup","commence","await","preprocess","postprocess","read","write","declare","scale","schedule","allocate","locate","resolve","returnResult","release"}){auto& entry=result[std::string("foundation:")+name];entry["version"]="0.1.0";entry["status"]="planned";}
        return result;
    }
private:


    REGISTRATION registry;
    static constexpr std::size_t maxSourceBytes = 1'048'576;
public:
    CODE() {
        registry.add("validate", "scope", "source record");
        registry.add("self-test", "scope", "source record verification");
    }
    bool validate(const std::string& language,const std::string& source) const {
        return (language=="cpp" || language=="wasm") && !source.empty() && source.size()<=maxSourceBytes;
    }

    std::string selfTest(const std::vector<std::string>& args) const {
        std::string language = "both";
        std::optional<std::size_t> bytes;
        bool languageSet = false;
        for (std::size_t i = 1; i < args.size(); i += 2) {
            if (i + 1 == args.size()) throw std::invalid_argument("Missing value for " + args[i]);
            const auto& value = args[i + 1];
            if (args[i] == "--language" && !languageSet) {
                if (value != "cpp" && value != "wasm" && value != "both") {
                    throw std::invalid_argument("Expected --language cpp, wasm or both");
                }
                language = value;
                languageSet = true;
            } else if (args[i] == "--bytes" && !bytes) {
                const auto size = languages::number(value);
                if (size > maxSourceBytes + 1) {
                    throw std::invalid_argument("--bytes must be between 0 and 1048577");
                }
                bytes = size;
            } else {
                throw std::invalid_argument("Unknown or repeated CODE self-test option: " + args[i]);
            }
        }

        unsigned count = 0;
        std::string failures;
        const auto check = [&](std::string_view name, bool passed) {
            ++count;
            if (!passed) failures += "\nFAIL " + std::string(name);
        };
        const bool cpp = language != "wasm";
        const bool wasm = language != "cpp";
        const auto first = cpp ? "cpp" : "wasm";
        const auto last = wasm ? "wasm" : "cpp";
        const std::string boundary(maxSourceBytes, 'x');
        if (cpp) check("accept C++ source record", validate("cpp", "int main() {}"));
        if (wasm) check("accept WASM source record", validate("wasm", "(module)"));
        check("reject unsupported language", !validate("python", "pass"));
        check("reject empty record", !validate(first, ""));
        check("accept 1 MiB record", validate(first, boundary));
        check("reject record over 1 MiB", !validate(last, boundary + 'x'));
        const bool expected = bytes && *bytes > 0 && *bytes <= maxSourceBytes;
        if (bytes) {
            const std::string sample(*bytes, 'x');
            if (cpp) check("C++ parameterized byte count", validate("cpp", sample) == expected);
            if (wasm) check("WASM parameterized byte count", validate("wasm", sample) == expected);
        }
        if (!failures.empty()) throw std::runtime_error("CODE self-test failed" + failures);
        std::string result = "CODE self-test passed (" + std::to_string(count) + " checks)";
        if (languageSet) result += "\nLanguages: " + language;
        if (bytes) result += "\nSample bytes: " + std::to_string(*bytes) +
            (expected ? " (accepted as expected)" : " (rejected as expected)");
        return result;
    }
};
