class WASM {
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
        return {{"class",{{"identity","WASM"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"WASM",{{"class","WASM"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","WASM"}}}};
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


    CPP& cpp;
    IR& ir;
    REGISTRATION registry;
public:
    WASM(CPP& c,IR& i):cpp(c),ir(i){registry.add("prepare","target","WebAssembly");}
    REQUEST prepare(const REQUEST& r) const {return ir.normalize(cpp.prepare(r));}
    std::string invoke(const TOOL::Arguments& args) const {
        if (args.empty() || args[0] == "status") return "WASM: prepare or generate the freestanding route module";
        if (args[0] == "bundle") {
#if defined(TOOL_CRYPTO) && defined(__linux__) && !defined(__wasi__)
            if (args.size() != 4 && args.size() != 5) {
                throw std::invalid_argument("Usage: wasm bundle source.cpp destination text|hex|binary [key-file]");
            }
            const native::TemporaryDirectory temporary;
            const auto payload = (temporary.path() / "module.wasm").string();
            invoke({"generate", args[1], payload});
            TOOL::Arguments pack{"pack", payload, args[2], args[3]};
            if (args.size() == 5) pack.push_back(args[4]);
            return artifact::invoke(pack);
#else
            throw std::runtime_error("Build the Linux native target with OpenSSL to bundle WebAssembly");
#endif
        }
        if (args[0] == "generate") {
            if (args.size() != 3) throw std::invalid_argument("Usage: wasm generate source.cpp output.wasm");
#if defined(__linux__) && !defined(__wasi__)
            const auto source = native::regularFile(args[1]);
            const auto output = std::filesystem::absolute(args[2]);
            const char* configured = std::getenv("TOOL_WASM_CXX");
            auto compiler = std::filesystem::path(configured && *configured ? configured : "/usr/bin/clang++");
            if (!compiler.is_absolute()) throw std::invalid_argument("TOOL_WASM_CXX must be an absolute path");
            if (source.extension() != ".cpp" || output.extension() != ".wasm") {
                throw std::invalid_argument("Expected a .cpp input and .wasm output");
            }
            if (std::filesystem::exists(output)) throw std::invalid_argument("WASM output already exists");
            const native::TemporaryDirectory temporary(output.parent_path());
            const auto staged = temporary.path() / "module.wasm";
            native::run({compiler.string(), "--target=wasm32-unknown-unknown", "-std=c++17", "-O2",
                         "-DTOOL_BROWSER_WASM", "-nostdlib", "-fno-exceptions", "-fno-rtti",
                         "-Wl,--no-entry", "-Wl,--strip-all", "-Wl,--export=tool_route_abi",
                         "-Wl,--export=tool_route_state", "-Wl,--export=tool_route_step",
                         "-Wl,--export=tool_javascript_size", "-Wl,--export=tool_javascript_byte",
                         source.string(), "-o", staged.string()}, "90s");
            char magic[8]{};
            std::ifstream binary(staged, std::ios::binary);
            if (!binary.read(magic, sizeof(magic)) ||
                std::string_view(magic, sizeof(magic)) != std::string_view("\0asm\1\0\0\0", 8)) {
                throw std::runtime_error("Compiler did not produce a WebAssembly binary");
            }
            std::filesystem::create_hard_link(staged, output);
            return "Generated " + output.string();
#else
            throw std::runtime_error("Tool WASM generation is available on Linux");
#endif
        }
        if(args[0]!="prepare" || args.size()<2)return "Usage: wasm status | wasm prepare method [arguments...]";
        auto r=prepare({args[1],TOOL::Arguments(args.begin()+2,args.end())});
        return "Prepared WASM invocation: "+r.method+" ("+std::to_string(r.arguments.size())+" arguments)";
    }
};
