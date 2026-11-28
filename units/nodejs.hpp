class NODEJS {
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
        return {{"class",{{"identity","NODEJS"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"NODEJS",{{"class","NODEJS"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","NODEJS"}}}};
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
 
public:
    NODEJS() { registry.add("run", "NODEJS", "volatile Node.js 26"); }

    std::string invoke(const TOOL::Arguments& args) const {
        constexpr const char* version = "26.13.1";
        if (args.empty() || (args.size() == 1 && args[0] == "status")) {
            return std::string("Node.js v") + version + " pinned; use nodejs version or nodejs run script";
        }
        const bool versionOnly = args.size() == 1 && args[0] == "version";
        if (!versionOnly && (args.size() < 2 || args[0] != "run")) {
            throw std::invalid_argument("Usage: nodejs status | version | run local-script [arguments...]");
        }
#if defined(__linux__) && !defined(__wasi__)
#if defined(__x86_64__)
        constexpr const char* architecture = "x64";
        constexpr const char* digest = "5c063e46bab0fbc1c17cc445d7ff129cbdcd0471be6ed83b6ab864f8143bc864";
#elif defined(__aarch64__)
        constexpr const char* architecture = "arm64";
        constexpr const char* digest = "89e1853138db51e8b82d3e98ba9c0d3f2956a28e5e55ea1b7e12d25aae92d5dd";
#elif defined(__s390x__)
        constexpr const char* architecture = "s390x";
        constexpr const char* digest = "2d65ece78005320dd13cccf15eeb30636a7352c18b92727f4b7c079de7d6969f";
#elif defined(__powerpc64__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        constexpr const char* architecture = "ppc64le";
        constexpr const char* digest = "06d5638602a919cb9a559eb92520862a387b4a0b170fed8883930b1eb392ec73";
#else
        constexpr const char* architecture = nullptr;
        constexpr const char* digest = nullptr;
#endif
        if (!architecture) throw std::runtime_error("No pinned Node.js archive for this Linux architecture");
        const auto script = versionOnly ? std::filesystem::path{} : native::regularFile(args[1]);
        const native::TemporaryDirectory temporary;
        const auto folder = std::string("node-v") + version + "-linux-" + architecture;
        const auto archive = temporary.path() / "node.tar.xz";
        const auto checksum = temporary.path() / "sha256";
        const auto binary = temporary.path() / "node";
        const auto url = std::string("https://nodejs.org/dist/v") + version + "/" + folder + ".tar.xz";
        native::run({"/usr/bin/curl", "-q", "--fail", "--silent", "--show-error", "--proto", "=https",
                     "--connect-timeout", "15", "--max-time", "90", "--max-filesize", "100000000",
                     "--output", archive.string(), url}, "95s", {}, true);
        native::run({"/usr/bin/sha256sum", "--", archive.string()}, "15s", checksum);
        std::string actual;
        std::ifstream(checksum) >> actual;
        if (actual != digest) throw std::runtime_error("Node.js archive digest mismatch");
        native::run({"/usr/bin/tar", "-xJOf", archive.string(), folder + "/bin/node"}, "25s", binary);
        std::filesystem::permissions(binary, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);
        const auto release = temporary.path() / "version";
        native::run({binary.string(), "--version"}, "10s", release);
        std::string reported;
        std::getline(std::ifstream(release), reported);
        if (reported != std::string("v") + version) throw std::runtime_error("Unexpected Node.js executable version");
        if (versionOnly) return reported;
        std::vector<std::string> command{binary.string(), "--", script.string()};
        command.insert(command.end(), args.begin() + 2, args.end());
        native::run(command, "60s");
        return {};
#else
        throw std::runtime_error("Volatile Node.js acquisition is available on Linux");
#endif
    }
};
