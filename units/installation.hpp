class INSTALLATION {
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
        return {{"class",{{"identity","INSTALLATION"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"INSTALLATION",{{"class","INSTALLATION"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{
        {"status", {{"kind", "metadata"}, {"owner", "INSTALLATION"}}},
        {"self-install", {{"kind", "local executable installation"}, {"owner", "INSTALLATION"}}},
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
#if defined(__linux__) && !defined(__wasi__)
    static bool sameExecutable(const std::filesystem::path& candidate,
                               const std::filesystem::path& target) {
        if (!std::filesystem::is_regular_file(std::filesystem::symlink_status(target))) return false;
        struct stat info{};
        if (::stat(target.c_str(), &info) != 0 || info.st_uid != ::geteuid() ||
            (info.st_mode & 07777) != 0700) return false;
        if (std::filesystem::file_size(candidate) != std::filesystem::file_size(target)) return false;
        std::ifstream expected(candidate, std::ios::binary);
        std::ifstream installed(target, std::ios::binary);
        expected.exceptions(std::ios::badbit);
        installed.exceptions(std::ios::badbit);
        using Byte = std::istreambuf_iterator<char>;
        return expected && installed && std::equal(Byte(expected), Byte{}, Byte(installed), Byte{});
    }

    static std::string install(const std::filesystem::path& destination) {
        namespace fs = std::filesystem;
        const auto directory = fs::canonical(destination);
        struct stat info{};
        if (::stat(directory.c_str(), &info) != 0 || !S_ISDIR(info.st_mode) ||
            info.st_uid != ::geteuid() || (info.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
            throw std::invalid_argument("Installation requires an existing directory owned by the current user and writable only by that user");
        }

        const auto target = directory / "tool";
        const native::TemporaryDirectory staging(directory);
        const auto candidate = staging.path() / "tool";
        fs::copy_file("/proc/self/exe", candidate);
        fs::permissions(candidate, fs::perms::owner_all);
        std::error_code error;
        fs::create_hard_link(candidate, target, error);
        if (!error) return "Installed Tool at " + target.string();
        if (error == std::errc::file_exists) {
            if (sameExecutable(candidate, target)) return "Tool is already installed at " + target.string();
            throw std::runtime_error("Installation target already exists: " + target.string());
        }
        throw fs::filesystem_error("Cannot publish Tool executable", target, error);
    }
#endif
public:
    INSTALLATION() {
        for (const auto& operation : COMMANDS) {
            registry.add(operation.first, "INSTALLATION", operation.second.at("kind"));
        }
    }

    std::string invoke(const TOOL::Arguments& args, bool localInstall) {
        if (args.empty() || (args.size() == 1 && args[0] == "status")) {
#if defined(__linux__) && !defined(__wasi__)
            return "Installation ready (native Linux)\nUsage: installation self-install <directory>";
#else
            return "Self-installation is available on native Linux";
#endif
        }
        if (args[0] != "self-install") return "Unsupported operation";
        if (args.size() != 2 || args[1].empty() || args[1].find('\0') != std::string::npos) {
            throw std::invalid_argument("Usage: installation self-install <directory>");
        }
        if (!localInstall) throw std::invalid_argument("Self-installation requires an explicit local CLI invocation");
#if defined(__linux__) && !defined(__wasi__)
        return install(args[1]);
#else
        throw std::runtime_error("Self-installation is available on native Linux");
#endif
    }
};
