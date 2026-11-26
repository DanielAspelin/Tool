# Tool

**An invocable, cross-functional C++ and WebAssembly runtime architecture.**

Tool is an independent architectural standardization project by Daniel Alexander Aspelin. Its purpose is to establish a decomposed but integrative runtime in which interfaces, classes, functions, instructions, sessions and operational passages can be instantiated, registered, invoked and reconciled across a common operational environment.

License: GNU GPL version 3 only (GPL-3.0-only). See [LICENSE](LICENSE).

## Reconciled mandatory architecture — superseding earlier conventions

All primary classes shall be independently instantiable multi-argument, multiparameterized functors containing parameterized functional objects. Each class shall retain ARCHITECTURE, DESIGN, MODEL, VERSION and SPECIFICATION nested associative arrays, its functional registry, MODELER, COMMANDER and SPECIFIER helpers, and a logical subprocess identifier distinct from an OS PID. Each primary class shall contain SERVICE, PROTOCOL, SOCKET and INTERFACE, each with its own TYPE, MODE, CONDITION and STATE subordinate types. This is a finite two-level hierarchy, not recursive replication.

### Comprehensive operator and argument coverage

Every functor shall provide comprehensive overload coverage for all applicable overloadable C++ operators, including operator(), and all supported datatype and argument-entry combinations. Accept zero or more arguments where the operator permits, heterogeneous and homogeneous variadic packs, native and user-defined types, composites, references, pointers and compatible callable objects. Use templates, constrained overloads or C++17 SFINAE, forwarding and runtime type erasure rather than enumerating unbounded permutations. Operators must respect C++ arity, syntax and semantic constraints; non-overloadable operators and unsupported combinations must receive explicit diagnostic resolution through the functional dispatch boundary. No claim is made that arbitrary C++ types or every mathematical combination can be instantiated or converted.

### Live datatype replacement and conversion

At every instantiation and subsequent regulated invocation, datatype bindings shall be live replaceable or convertible through registered typed conversion and replacement contracts. Preserve instance identity, ownership, lifetime, version and provenance. Runtime replacement changes a dynamic value or binding, not the static C++ type of an existing object; use variant, type erasure or reconstruction where appropriate. Validate conversions, range, precision, ownership and compatibility before atomic publication; reject unsupported or unsafe conversions, retain the previous binding on failure, and synchronize concurrent readers. Existing invocations retain a consistent snapshot unless an explicitly regulated safe transition is supported.

### Kernelisation and universal portable neutrality

KERNELISATION is an independent primary functor containing its own nested KERNELISER functor, in addition to the universal class-local architecture. KERNELISATION establishes foundational operational cores for systems, programs, software, applications, apps and engines. KERNELISER performs the corresponding multiparameterized functional operations. Kernelisation is independent of operating-system kernels and does not involve kernel modification or require the KERNEL functor. TOOL shall contain its own independently instantiable TOOLCORE functor, representing Tool's foundational operational core. TOOLCORE follows the universal primary functor architecture, including nested STACK and HEAP, comprehensive applicable operator overloads, live datatype bindings, regulation, neutral portability and fallback resorts. TOOLCORE is distinct from the reusable KERNELISATION and KERNELISER capabilities; no mandatory dependency between them is presumed.

Every primary and subordinate functor shall remain neutral wherever technically possible and portable-ready across operating systems, processor architectures, native and WebAssembly execution environments, language boundaries, datatypes and communication mechanisms. Each functor shall independently identify capabilities, dependencies and compatible alternative operational resorts. Fallback coverage shall include preferred supported implementation, compatible alternative, platform-neutral implementation, authorized external delegation, local self-sustaining reduced-capability operation, and controlled deferral or explicit failure. Resort selection is individually regulated and capability-driven, not a mandatory fixed sequence. Preserve invocation identity, functional intent, authorization, datatype integrity and observable outcome across transitions. Bound retries, prevent fallback cycles, disclose degraded capability and individually testify integration. External binaries remain subject to execute resolution. All applicable operator overload, functional registry, live datatype and runtime requirements remain in force. These are target specifications pending source implementation and portability testing.

### Universal class-local STACK and HEAP functors

Every primary class shall contain independently instantiable nested STACK and HEAP functors. Both shall expose comprehensive applicable C++ operator overloads and multiparameterized functional objects, including heterogeneous variadic argument dispatch, live datatype replacement and conversion, individual logical subprocess identification, class-local associative registries, regulation and integration testimony. STACK governs scoped operational storage, argument frames, temporary values and invocation context; HEAP governs dynamic allocation, retained objects, ownership and managed lifetimes. Both shall support regulated concurrent and parallel operation, checked allocation, lifetime validation, deterministic cleanup where applicable, and explicit failure resolution. These abstractions do not imply direct control of the native machine stack or allocator internals. No recursive replication of STACK and HEAP inside themselves is required. This is a mandatory target specification pending implementation and verification.

### Build architecture expansion

MAKE, CMAKE, MK and MAKEFILE are mandatory independent primary class profiles. MAKE orchestrates build targets and dependencies; CMAKE manages build configuration, generation and toolchain preparation; MK represents reusable makefile fragments and modular build definitions; MAKEFILE manages makefile parsing, construction, generation and reconciliation. Each follows the universal functor, operator, live datatype, subordinate class, registry, regulation and runtime contracts. Integration with BUILDER, COMPILER, ASSEMBLER, LINKER and other concerned classes is individually testified. External build binaries are invoked only through regulated execute resolution. These four profiles are specified additions, not verified source declarations or working build backends.

### Runtime and integration

Each class retains reconcile, automate, generate, copy, dump, delete (registered operation; reserved C++ keyword), create, construct, reconstruct, deliver, serve, obtain, retain, resume and service functional operations. The runtime circulates Await -> Preprocess -> Postprocess -> Await, concurrently and in parallel where dependencies permit. Execute is a resolution option for external binaries only. While-loop bodies delegate to parameterized calls. Individual and grouped regulation supports preconfiguration and live configuration through <uuid>.configuration.tool, including reactive/nonreactive and active/passive modes. Each concerned class individually testifies integration; FUSION and fuse are obsolete target-architecture elements and must be migrated and removed. Earlier references below are historical baseline documentation, not current requirements.

All these requirements are specifications pending source reconciliation, compilation and testing.

## Development status

Architectural foundation / experimental prototype. Source inventory as of 2026-11-26: **116 classes and 84 original commands, plus 334 generated registrations (474 expected total)** in [tool.cpp](tool.cpp). These counts describe source declarations, not independently verified working features. Native compilation, WebAssembly compilation and end-to-end testing have not yet been verified.

## Architectural conventions

- C++ and WebAssembly are the intended foundational implementation and portable execution pair.
- The monolith is internally decomposed into instantiable, uppercase-named classes with cross-functional relationships.
- Operational functionality belongs inside classes; the entry point delegates to the hybrid runtime.
- While loops delegate execution to function calls, enabling controlled roundtrips.
- Each class should maintain its own nested associative function registry.
- Every class and relevant function should ultimately be independently invocable, continuously instantiable and capable of controlled concurrent and parallel execution.
- Each class should ultimately have a pool; invocations should circulate through logically shared, physically distributable pooling environments.
- Background services and processes, volatile and persistent operation, and transitions between invocation modes are architectural objectives.
- Reciprocal relationships must be bounded by identity, acknowledgment, completion, cancellation and duplicate detection rather than uncontrolled recursion.

These are design conventions; universal enforcement is not yet implemented.

## Foundational invocation procedure

1. Receive an invocation and capture its original system-clock and monotonic-clock snapshots.
2. Optionally associate an observed public network egress address through IPIFY, without blocking offline execution.
3. Initiate signaling and triggering.
4. Establish the primary reciprocal relationship between HEARTBEAT and REFLEX: detection and immediate response.
5. Extend immediate response through REACTION.
6. Determine eligible circulation through ROUNDROBIN and conduct progression through FLOW.
7. Use POOL as the operational medium for reaching the intended destination, with FARM configuring pooling environments.
8. Resolve instructions through compatible passages, adapters and cross-fusion.
9. Acknowledge completion, measure elapsed time and return the result.
10. Preserve unresolved, unknown or exceptional activity locally through DUMP.

This is the intended operational sequence, not a claim that every stage is already automatically connected.

### Primary reciprocal relationships

| Relationship | Responsibility |
| --- | --- |
| SIGNAL / TRIGGER | External and internal stimulation and initiation |
| HEARTBEAT / REFLEX | Primary reactive detection and immediate acknowledgment |
| REFLEX / REACTION | Immediate response and extended operational response |
| ROUNDROBIN / FLOW | Destination selection and circulation feedback |
| POOL / FARM | Operational passage and pooling configuration |

The signal-blast concept distributes operational awareness to eligible participants; it does not require every recipient to execute the same instruction. Round-robin selection and concurrent notification are distinct mechanisms. Feedback, bounded propagation and acknowledgment remain development objectives.

## Class architecture

| Area | Classes | Current scope |
| --- | --- | --- |
| Runtime | TOOL, HYBRID, REGISTRATION, REQUEST, RESPONSE, DISPATCHER, SERVER, CLIENT | Local invocation registry, dispatch and composition |
| Interfaces | API, APISERVER, APICLIENT, CLI, CLISERVER, CLICLIENT, MCP, MCPSERVER, MCPCLIENT, SSH, SSHSERVER, SSHCLIENT, RPC, RDP, RDPSERVER, RDPCLIENT, VNC, VNCSERVER, VNCCLIENT | Local interface abstractions; not real network protocol implementations |
| Identification and configuration | URI, URL, CONFIGURATION, SESSION, INDEX | Basic syntax checks, in-memory configuration, bounded session tree and index |
| Passages | MESH, CHAIN, RING, MATRIX, GRID, PATH, ROUTE, LANE, BRIDGE | Local topology, named passages and route selection |
| Cross-fusion | AUTHORIZATION, COMPATIBILITY, ADAPTER, FUSION | Limited local authorization, compatibility and adapter-based routing |
| Reactive circulation | SIGNAL, TRIGGER, HEARTBEAT, REFLEX, REACTION, ROUNDROBIN, FLOW | Event counters, pulse observations, immediate response and cyclic selection; automatic reciprocal propagation pending |
| Pooling | POOL, FARM | Current resource leasing and pool composition prototype; broader invocation circulation pending |
| Dynamic invocation | LAMBDA | Registered callable invocation |
| Code architecture | AST, IR, CPP, WASM, CODE, CODEBASE, SANDBOX, SDK | Request representation and preparation scaffolding; not a compiler or secure execution sandbox |
| Security | AES, GCM, RSA, SHA, OAEP, PSS, BASE64 | Cryptographic profiles only, except actual Base64 encoding/decoding; Base64 is not encryption |
| Observation | TIMESTAMP, IPIFY, DUMP | Local time capture, optional native public-IP observation, local anomaly records |

## Resource regulation and simultaneous invocation

| Class | Current responsibility |
| --- | --- |
| CPU | Hardware thread count |
| MEMORY | Per-invocation input size and argument limits |
| THERMAL | Linux thermal-zone observations |
| CAPACITY | Conservative concurrency ceiling |
| PRESSURE | Linux memory pressure observation |
| CONFLICT | Exclusive admission for selected named operations |
| THROTTLE | Adaptive admission limits |
| GUARD | Input and nested invocation depth checks |
| QUEUE | Bounded FIFO scaffolding |
| WATCHDOG | Post-completion deadline observation |
| CLEANUP | Scope-based release of admission and conflict leases |
| SCHEDULER | Bounded concurrent admission |

The invocation boundary now checks input size (64 KiB of argument content, 256 arguments and 256-byte method name), nested depth (16), selected operation conflicts and outer invocation capacity. Capacity is capped at eight and normally no more than half the reported hardware concurrency. Missing thermal or pressure telemetry reduces admission to one; observed thermal-zone values at or above 90 C stop new outer admissions. These are provisional conservative policies, not hardware-certified thresholds. Existing running work is not interrupted. The regulation command reports available telemetry and admission limit, but is itself subject to admission.

QUEUE is not yet connected to a worker executor; simultaneous callers may be deferred rather than queued. WATCHDOG observes deadlines after completion, not during a stall. MEMORY limits input, not total process memory. CONFLICT protects selected same-name operations, not all shared resources. Thermal sensing is Linux-specific and may be absent or unrelated to CPU temperature. No guarantees against overheating, deadlock, overflow or hardware failure are claimed. OS resource controls, host capability adapters, reserved control capacity, actual worker scheduling, bounded registries and stress tests remain pending.

## Parameter and value foundations

VALUE holds a typed null, boolean, integer, floating-point or string value. PARAMETER defines a named expected type and requiredness. ARGUMENT binds an immutable shared value to a parameter through validation. REFERENCE resolves a non-owning weak reference to a value, safely reporting expiration through an empty shared pointer. The four classes maintain individual registries.

The registered value, parameter, argument and reference commands demonstrate string-valued operations. Full typed command parsing, persistent reference identity, and integration into REQUEST and every invocation remain pending. Compilation has not been verified.

## Callable and object foundations

CALL delegates to the existing TOOL invocation boundary. RETURN carries an explicit boolean and result string; CALL currently marks normal returns successful even when TOOL returns an error string, so typed outcome reconciliation is pending. FUNCTION wraps a callable. VARIABLE synchronizes replacement of immutable shared values. CLASS represents runtime class metadata, OBJECT stores up to 64 named fields against a shared class definition, and INSTANCE associates a caller-supplied identity with an owned object. Each has an individual registry. These are runtime abstractions, not dynamic C++ class compilation or persistent instance management. Demonstration commands are registered; universal typed integration and compilation verification remain pending.

## Array and boolean foundations

ARRAY provides synchronized indexed storage of up to 256 immutable shared VALUE objects, with checked retrieval. BOOLEAN provides immutable truth values, negation, conjunction, disjunction, exclusive OR and conversion to VALUE. Both maintain independent registries. The array command demonstrates bounded string collection and returns its size; the boolean command evaluates explicit true/false operands. Nested arrays, universal invocation integration and compilation verification remain pending.

## Numeric foundations

INTEGER represents signed 64-bit integers; FLOAT and DOUBLE represent native single- and double-precision floating-point values. Each maintains an independent registry, immutable numeric state, validated text parsing and conversion to VALUE. Nonfinite floating-point values and malformed or out-of-range input are rejected by the parsing boundary; floating-point rounding and underflow remain subject to native representation and parsing behavior. FLOAT widens to double when converted to VALUE because VALUE currently has no separate float alternative. The integer, float and double commands demonstrate parsing and representation; checked arithmetic, universal typed invocation integration and compilation verification remain pending.

## Character and string foundations

CHARACTER represents one Unicode scalar value, with strict UTF-8 decoding and encoding. STRING represents immutable validated UTF-8 text bounded to 64 KiB, with byte and Unicode scalar counts. Both maintain independent registries and convert to the existing string alternative of VALUE. The character and string commands demonstrate validation and representation. Scalar counts are not grapheme counts; normalization, grapheme segmentation, distinct character typing in VALUE, universal integration and compilation verification remain pending.

## Token foundation

TOKEN represents bounded, immutable, nonsecret metadata for lexical, invocation, session, instance, reference, access, resource and execution categories. It validates category and UTF-8 metadata, retains origin, scope and creation time, supports a caller-supplied expiry duration and converts its identity to VALUE. Access-category metadata is not an authentication credential; no secrets should be passed to the token command. No credential issuance, authorization, uniqueness, revocation or full lexical analysis is implemented. Expiry currently uses the system clock. Automatic lifecycle integration and compilation verification remain pending.

## Time and date foundations

TIME projects a TIMESTAMP snapshot into UTC time (HH:MM:SS). DATE projects it into a UTC calendar date (YYYY-MM-DD). TIMEDATE composes both from the same immutable snapshot into an ISO 8601 UTC representation (YYYY-MM-DDTHH:MM:SSZ), retaining the original system and monotonic clock readings. Each has an independent registry. UTC is the default to avoid ambiguous local times; timezone conversion, trusted clock synchronization and fractional-second formatting are pending. The commands capture their own snapshots, not yet the original TOOL invocation context. Compilation remains unverified.

## Licensing and standards reference foundations

GPL, GNU, RFC, ISO and IEC provide independent invocable reference classes with per-class registries, descriptive identities and official source URLs. GPL identifies this project's existing GPL-3.0-only licensing designation; GNU identifies the GNU Project; RFC identifies the Request for Comments publication series (not every RFC is a standard); ISO and IEC identify their respective standards organizations. These are reference foundations, not standards implementations, certification, compliance verification, or license enforcement. Document retrieval, version-specific catalogs, cross-functional integration and compilation verification remain pending.

## Universal invocation coverage — first stage

Every class has generated CLASS.describe and CLASS.status commands for discovery and registration inspection (not runtime health). 77 class names map to existing command handlers through CLASS.invoke; TOOL and HYBRID map to help only. 32 operation aliases forward to existing handlers, e.g. gpl.source and heartbeat.pulse. The coverage command lists classes, filters registered commands by prefix, and inspects candidate class passages without claiming compatibility. Expected registrations: 474, including the 80 original commands. Aliases reuse functionality, not new independent implementations. No arbitrary class-to-class execution is enabled. Runtime counts and compilation remain unverified.

## Specialized data invocation expansion

35 additional registrations expose integer, float and double add/subtract/multiply/divide/compare/min/max; string bytes/length/empty/compare/contains/append; character codepoint/bytes; boolean not/and/or/xor; and array count/contains. Integer arithmetic checks overflow and division by zero. Floating point rejects nonfinite results and float range overflow, but rounding and underflow remain native. String comparison is bytewise, not locale collation. Boolean operations reuse the existing handler. These operations are registered in HYBRID rather than fully decomposed into their respective classes; that refactoring and native/WASM compilation tests remain pending. Expected total after assembler expansion: 457 registrations, not runtime verified.

## Assembler and disassembler foundations

ASSEMBLER and DISASSEMBLER share a bounded x86_64 instruction encoding table (12 exact instruction forms). The assemble command accepts one quoted instruction per argument and returns hexadecimal bytes; disassemble accepts contiguous hexadecimal bytes and returns one instruction per line. Unsupported instructions or encodings are rejected. No machine code is executed. Both classes have independent registries and generated coverage commands. This is a subset codec, not a complete assembler, disassembler, linker or object-file tool. Other architectures, labels, relocations, operands beyond the table, and native/WASM testing remain pending. Expected total: 116 classes and 474 command registrations.

## Socket and service foundations

SOCKET maintains up to 256 named TCP/UDP port definitions in memory, with define, inspect, remove and list operations. It does not open or bind operating-system sockets. SERVICE maintains up to 256 named local command definitions, with define, run, inspect, remove and list operations. Run dispatches synchronously through TOOL. Definitions survive within an interactive process only. Neither class establishes background daemons, network listeners or remote access. Expected total: 117 classes and 488 registrations, pending compilation and runtime verification.

## Runtime and instance integration

RUNTIME maintains up to 64 named volatile logical environments, each containing up to 64 retained INSTANCE objects. Operations create, inspect, attach, detach, instances, suspend, resume, remove, list and run are independently registered. Run checks active state and instance membership before dispatching an existing command synchronously through TOOL; it does not execute in an isolated instance context. Suspension prevents new direct dispatch but does not interrupt ongoing work. INSTANCE itself is unchanged; subordinate runtimes, reciprocal ownership, background workers, persistence and isolation remain pending. Expected total: 117 classes and 488 registrations, unverified by compilation.

## Capsule and quarantine foundations

CAPSULE stores up to 256 named volatile text records, each up to 4096 bytes, with create, inspect, read, seal, remove and list operations. Seal is a logical marker, not encryption or integrity certification. QUARANTINE shares the capsule registry through synchronized hold, inspect, release and list operations. Held capsules reject ordinary read, seal and removal until manually released. Release does not verify safety or enforce an independent authorization policy. This is logical data containment only, not process isolation, malware analysis, or a security sandbox. Expected current total: 119 classes and 506 registrations, pending compilation and runtime verification.

## Environment and facility foundations

ENVIRONMENT maintains up to 64 volatile logical contexts with up to 64 settings each, and create, inspect, set, get, suspend, resume, list and remove operations. FACILITY maintains up to 64 named organizations of environment identities with create, attach, detach, inspect, list and remove operations. Attach checks existence; inspection marks missing references after environment removal. Neither class changes OS environment variables, provisions infrastructure, or controls actual runtime execution. Expected current total: 121 classes and 528 registrations, pending compilation and runtime verification.

## Observation and evaluation foundations

AUDIT stores up to 1024 manually submitted volatile category/text records. FORENSIC traces matching records in insertion order; ANALYTICAL counts categories and compares text exactly. STATISTICAL calculates count, mean, minimum and maximum of up to 256 finite numeric inputs. STATUS stores up to 256 manually declared named states. These classes do not automatically collect invocation telemetry, establish forensic chain of custody, certify integrity, or measure live health. Expected total: 126 classes and 559 registrations, pending compilation and runtime verification.

## QR and EAN foundations

QR provides nominal byte-mode capacity lookup for versions 1–40 and correction levels L/M/Q/H, plus payload byte counting. EAN calculates and validates EAN-8 and EAN-13 check digits, preserving leading zeros. Checksum validity does not establish GS1 assignment or authenticity. Neither class renders or scans symbols yet. Expected total: 128 classes and 572 registrations, pending compilation and runtime verification.

## Wave Robin

WAVEROBIN receives SIGNAL emissions through a callback outside the signal mutex and pulses HEARTBEAT for the wave and its target. A thread-local guard prevents recursive callback propagation on the same thread. Manual echo and status commands are available; ordinary command instrumentation also emits signals, including status queries. This is synchronous local stimulation, not distributed wave propagation or automatic heartbeat-to-round-robin dispatch. Robin Round and Robin Through remain proposed. Expected total: 129 classes and 578 registrations, pending compilation and runtime verification.

## Trace Robin

TRACEROBIN observes SIGNAL callbacks before WAVEROBIN echo and retains the latest 1024 timestamped target/event records with local sequence numbers. Manual record, trace, status, enable and disable operations are available. Records are volatile, truncated to 128 bytes per field and not correlated by invocation ID. Status and trace commands themselves may generate observed signals. This is a local signal observer, not full route reconstruction, distributed tracing or proof of delivery. Expected total: 130 classes and 587 registrations, pending compilation and runtime verification.

## Hotcode

HOTCODE provides up to 256 named volatile bindings to existing registered commands, with bind, replace, inspect, enable, disable, run, list and remove operations. Replacement is synchronized and revisioned; run snapshots the command before dispatch, so existing calls are not redirected mid-execution. This is live invocation rebinding, not compilation, arbitrary code injection, native hot patching or WebAssembly module replacement. Expected total: 131 classes and 599 registrations, pending compilation and runtime verification.

## Availability and resolution foundations

UPTIME and DOWNTIME share up to 256 named availability records and measure cumulative declared up/down intervals using a monotonic clock. TRIGGERED and RESOLVED share up to 1024 named occurrence records, with one recorded resolution and elapsed duration per occurrence. States are manually declared and volatile; automatic heartbeat health detection, invocation correlation, verified outcomes, persistence and retention management remain pending. Expected total: 135 classes and 626 registrations, pending compilation and runtime verification.

## Operational expansion

Added LOCATOR, POSITIONER, PREPOSITIONER, PASSTHROUGH, ROUND, THROUGH, VELOCITY, FREQUENCY, ENTRY, EXIT, INPUT, OUTPUT, SHELL, TERMINAL, CONSOLE, PERSISTENCE, VOLATILITY, IDLE, SLEEP. Existing SESSION retained with list and trigger aliases. ROUND and THROUGH produce textual plans only; PASSTHROUGH and SHELL/TERMINAL/CONSOLE dispatch registered Tool commands, not OS shell commands. LOCATOR searches command registrations; POSITIONER and PREPOSITIONER store logical positions. VELOCITY and FREQUENCY calculate supplied ratios. ENTRY/EXIT/INPUT/OUTPUT inspect byte lengths; EXIT uses exit.inspect.boundary to preserve the existing exit command. PERSISTENCE and VOLATILITY delegate to the existing invocation store. IDLE reports policy only; SLEEP waits at most one second on native targets. Expected total: 154 classes and 732 registrations, pending compilation and runtime verification. No automatic circulation, physical positioning, OS terminal or scheduler idle detection is implemented by this expansion.

## Detection

DETECTION checks registered command presence and compares manually supplied observations for up to 256 named entities, tracking state changes under a mutex. Automatic sensing, signal emission on changes, hardware/environment probes and persistent history are not implemented. Expected total: 155 classes and 741 registrations, pending compilation and runtime verification.

## Operational capabilities

CALLABLE checks registered command presence and dispatches calls through TOOL. INVOKABLE delegates to CALLABLE for operational dispatch; registration is not proof of authorization or readiness. RETURNABLE retains up to 256 manually declared success/failure outcomes with bounded text values. Automatic typed return propagation, invocation correlation and universal capability integration remain pending. Expected total: 158 classes and 761 registrations, pending compilation and runtime verification.

## Programming and transformation expansion

Added COMPILER, DECOMPILER, HEX, HEXADECIMAL, HEXTABLE, AUTOMATION, GENERATION, ENCRYPTION, DECRYPTION, ENCODING, DECODING, LEXER, DELEXER, LEXICALISER, DELEXICALISER, TOKENISER, DETOKENISER, PARSER, TRANSPARSER, CROSSPARSER, PROCESS, PROCESSOR, ALLOCATOR, SERIALISER, DESERIALISER, HASHER, SALTER, LOCKMAKER, KEYMAKER, BUILDER, LINKER, DEBUGGER, SCANNER, PRINTER, PORT, FINDER, SEARCH. CALL, RETURN, ASSEMBLER and DISASSEMBLER retained. Local operations include hexadecimal conversion, whitespace splitting/joining, quoted text packing/unpacking, bounded repetition, command dispatch and prefix search, text scanning, port number validation and size policy checks. Lexical and parser operations are not language grammars; unpack output is not lossless for arbitrary embedded newlines. COMPILER, DECOMPILER, ENCRYPTION, DECRYPTION, HASHER, SALTER, LOCKMAKER, KEYMAKER, BUILDER, LINKER, DEBUGGER expose status only and report unavailable backends. No cryptographic security, compiler execution, process isolation, real memory allocation, physical printing or OS port binding is claimed. Historical totals are estimates pending full inventory reconciliation and runtime verification; this expansion adds 213 expected registrations.

## Passive runtime convention

Classes should remain registered and instantiated but perform specialized work only on demand, returning to passive existence after invocation unless explicitly configured for ongoing work. This expansion adds CLOAKER, CONCEALER, EXPOSER, IMPORTER, EXPORTER, CLIPBOARD, CACHER, INTERFACE, DRIVER, CONTROLLER, DEVICE, SYSTEM and retains ADAPTER. New constructors register metadata only; no workers or external probes are started. Redaction and masking are presentation transformations, not network concealment or security boundaries. Import/export and clipboard/cache are local text operations; DRIVER and DEVICE expose unavailable-backend status. SYSTEM reports compile target, not live OS detection. CONTROLLER dispatches registered commands subject to existing Tool admission. The convention is not yet enforced across all older classes. Added 77 expected registrations; runtime inventory and compilation remain unverified.

## Extended passive runtime profiles

Added TYPE, MODE, CONDITION, STATE, NETWORK, TCPIP, GPS, GSM, IMEI, RFID, RAM, GPU, SD, DEV, LAN, WAN, HOST, HOSTNAME, TCP, IP, UDP, ICMP, NAT, MAC, FAT, FAT32, NTFS, APFS, EXT4, EXFAT, AARCH, ARM, AMR64, X86, X86_64, AMD64, RISCV, PSU, POWER, OAUTH, SSL, TLS, HTTP, HTTPS, FTP, DNS, DHCP, VPN, PROXY, FIREWALL, UUID, ENTER, ESC, CTRL, ALT, SPACE, SHIFT, TAB, NAME, NUMBER, PID, USB, ARPA, PROVIDER, SMS, TLD, ICANN, IANA, IETF, W3C, WWW, NODE, NODEJS, IMAGE, IMG, BLOCK, LINE, EOF, SCRIPT, SOURCECODE, BINARY, SOURCE, DESTINATION, TARGET, CERTIFICATE, PASSPHRASE, PASSWORD, USERNAME, USER, SECRET, DATABASE, TABLE, COLUMN, ROW, TUPLE, QUERY, POLICY, MAP, TREE, MINDMAP, MARKDOWN, PDF, JSON, XML, CSV, SIGNATURE, AUTHOR, SENDER, RECEIVER, TRANSMISSION, WIFI, HOTSPOT, TETHER, LEDGER, ROUTER, SWITCH, GATEWAY, SUBNET, IPV4, IPV6, PAGING, COMMAND, INSTRUCTION. CPU, OBJECT and ISO retained. EOF is implemented as C++ class ENDOFFILE to avoid the standard EOF macro, with EOF public invocation identity. AMR64 spelling retained as requested; ARM64 is the conventional spelling. New constructors register metadata only. Most classes expose status and explicitly report unavailable specialized backends. Limited local operations: CONDITION.equal, NUMBER.validate, LINE.count, BINARY.validate, UUID.validate, COMMAND.check, INSTRUCTION.check, EOF.marker, ENTER.code, ESC.code, SPACE.code, TAB.code. Key codes are ASCII/control character values, not hardware key events. UUID validation checks shape only. No credentials, hardware probes, network connections, filesystem mounting, protocol stacks or database engines are activated. Added 627 expected registrations; cumulative estimates and source inventory require runtime reconciliation. Compilation and WASM verification pending.

## Platform and infrastructure profiles

Added BASH, PS1, APPLESCRIPT, CMD, BATCH, WINDOWS, MACOS, LINUX, ANDROID, IOS, DEBIAN, FEDORA, GENTOO, PATCHER, WRAPPER, FORKER, WIRING, VOLUME, CLUSTER, REPLICATION, MIGRATION, UNIT, INSTALLATION, PACKAGING, PACKAGE, PACKET, FILTER, SENSOR, PROXIMITY, DISTRIBUTION, GROUPING, RESOURCING, HARDWARE, FIRMWARE, BOOTLOADING, BOOTSTRAPPING, HOSTING, SITE, DOMAIN, FOREST, GLOBAL, LOCAL, JOINER, LEAVER, ONLINE, OFFLINE. Constructors register metadata only; specialized execution is on demand. Local operations: FILTER.contains, GROUPING.count, JOINER.join, PACKET.size, UNIT.describe. Other profiles expose status only; no shell execution, OS installation, firmware changes, process forking, deployment, network probes or live connectivity detection is implemented. PS1 denotes the requested PowerShell script profile. Added 235 expected registrations; compilation and runtime enumeration pending.

## Invocation-bound expansion

Added DAEMON, KERNEL, KO, KBUILD, INSTRUMENT, CLOUD, DATACENTER, FRAME, MAIN, MAINSYSTEM, ALLOCATION, PROCESSING, VERSIONING, TRANSVERSIONING, QUALIFICATION, VERIFICATION, VALIDATION, CONFIRMATION, AUTHENTICATION, AUTHORISATION, APPROVAL, MONITORING, ANALYSIS, STATISTICS, SCIENCE, CALCULATOR, ARITHMETICS, QUALIFIER, IDENTIFICATION, IDENTIFIER, QUANTIFIER, JAVASCRIPT, R, C, ASSEMBLY, LUA, CSS, HTML, BLUETOOTH, NFC, MICROSD, STORAGE, DISK, REST, DIRECTORY, FILE, MANAGEMENT, MANAGER, OPERATOR, TRANSPORT, CLOCK, INFO, DASHBOARD, AVERAGE, SUM, SUMMARY, SUMMARISATION, GLOSSARY, DOCUMENT, DOCUMENTATION, RECORDING, BLACKBOX, WHITEBOX, LEGEND, BOUNDARY, RULE, REGULATOR, REGULATION, LOOP, WHILE, FOR, IF, ELSE, ELSEIF, INJECTION, FUNCTOR, FUNCTIONAL, VECTOR, GRAPHICS, GDT, MBR, BOOTSECTOR, SUPERBLOCK, CHUNK, BITRATE, SAMPLE, EXAMINATION, INSPECTION, FORENSICS, ANALYTICS, FORECAST, WEATHER, ALERT, ALARM, CALENDAR, EPHEMERAL, TEMPORARY, DEPLOYMENT, FLAG, TESTING, MOCKING, TESTER, MOCKER, ANTIPENETRATION, ANTICORRUPTION, ANTIVIRUS, SELF. Existing RUNTIME, MEMORY, AUDIT, STATUS, STATISTICAL, ANALYTICAL, FLOW, FUNCTION retained. AUTHORISATION is a separate requested spelling/profile from existing AUTHORIZATION; reconciliation pending. Tool's intended runtime belongs to the invoker until resolution; persistent artifacts do not imply persistent execution. DAEMON is a passive profile, not a started daemon. New constructors register metadata only. Local operations: CALCULATOR.calculate, ARITHMETICS.calculate, CLOCK.now, AVERAGE.calculate, SUM.calculate, BOUNDARY.check, IF.evaluate, ELSE.evaluate, ELSEIF.evaluate, CHUNK.size, FLAG.validate, SELF.describe. CALCULATOR and ARITHMETICS currently sum only; conditional operations are boolean demonstrations, not control-flow engines. Other classes report unavailable backends. No kernel changes, security scanning, language execution or live weather retrieval. Added 547 expected registrations; existing runtime ownership enforcement, compilation and enumeration pending.

## Representation and language expansion

Added HEADER, METADATA, CONSTANT, MUTABLE, IMMUTABLE, CLASSIFICATION, CLASSIFIER, SAVER, AUTOMATOR, GENERATOR, PNG, SVG, MP4, WAV, MP3, OGG, OPUS, FLAC, JPEG, TXT, SH, JS, JAVA, GRADLE, SWIFT, OBJECTIVE_C, OBJECTIVE_CPP, RUST, DART, PYTHON, HAL, SYNTAX, LANGUAGE, SQL, NOSQL, GRAMMAR, VOCABULARY, DICTIONARY, SYMBOL, ALPHABET, LETTER, DIGIT, FORMATSTRING, FORMATTING, FRAGMENTATION, SEGMENTATION, SEQUENCING, SEQUENCER, SEGMENTER, CAPTURER, EXPLORER, DISCOVERER, LISTENER, INTERPRETER, ITERATOR, REPRESENTER, REPRESENTATION. Existing LUA retained. New classes are passive invocation-bound foundations. Local operations: METADATA.size, TXT.size, SYMBOL.size, ALPHABET.ascii, LETTER.validate, DIGIT.validate, FORMATSTRING.literal, SEQUENCING.count, ITERATOR.count, REPRESENTATION.size. LETTER and DIGIT are ASCII only; size operations count bytes. JS is a separate requested profile from JAVASCRIPT; alias reconciliation pending. SAVER is intended to support the native `<uuid>.<op*_func*>.<class>.tool` persistence convention, but file writing, encryption and restoration are not implemented. Media codecs, language execution, database backends and specialized transformations are pending. Added 295 expected registrations; compilation and runtime enumeration pending.

## Universal class-local modeling and command convention

All 528 top-level class definitions contain their own nested MODELER and COMMANDER types and private per-instance MODELS and COMMANDS nested associative maps. The nested types expose metadata definition and inspection helpers, with no autonomous execution. Initial entries identify the owning class and invocation-bound activation, and a status metadata entry. Existing function registration and dispatch remain separate; full model/command synchronization, callable binding, concurrency protection and native persistence are pending. The convention applies to primary classes, not recursively to the nested helper types. Compilation remains unverified.

## Universal specification and transversioning foundations

All 528 primary classes now declare a private SPECIFICATION nested associative map and nested SPECIFIER helper. Initial class and helper operation versions are 0.1.0. SPECIFIER provides explicit revision replacement with previous revision metadata, array projection and a per-class atomic construction counter. These are foundations only: ordinary functions, functors, variables and all nested arrays are not automatically enumerated or synchronized, instance copying does not allocate a new index, revision updates are not automatically triggered, and no persistent revision history is implemented. The counter retry loop requires reconciliation with the function-call-only loop convention. Specification access, concurrency safety, full callable indexing, compilation and runtime validation remain pending. The intended canonical class array is SPECIFICATION; runtime ownership remains invocation-bound.

## Invocation lifecycle

The current registry supports direct invocation, volatile definitions, persistent definitions, transitions and execution of named plans. Persistent definitions are stored in the working directory in `tool.invocations`; this does not imply persistent execution. The file is not encrypted or transactionally updated. Do not store credentials in invocation plans.

Future invocation dimensions include synchronous/asynchronous, concurrent/parallel, foreground/background, temporary/continuous/recurring, direct/indirect/conditional/scheduled, static/dynamic/injected, local/remote/distributed, roundtrip/callback/loopback/fusion and dedicated/pooled allocation.

## Temporal and network snapshots

TIMESTAMP captures system and monotonic clocks at invocation entry. Epoch nanoseconds are a representation, not a guarantee of clock accuracy. The latest timing record per command is retained in memory. Trusted time synchronization, uncertainty reporting and propagation of a common invocation identity and timestamp across nested calls are pending.

IPIFY is optional and disabled by default. On supported native Unix-like systems it uses a bounded asynchronous curl request to https://api64.ipify.org, validating the returned IPv4 or IPv6 address. Enabling it contacts an external service which observes the network egress address. Offline or unsupported environments can continue without an IP. The current snapshot is read at invocation entry but not yet retained in a durable invocation record. Automatic periodic refresh and WASM host integration are pending.

DUMP appends anomaly metadata to `tool.dump` in the execution working directory on supported native Unix-like systems, with restricted permissions and checks against unsafe file types. Unknown invocations and exceptions are captured automatically; unacknowledged activity can be recorded manually. Records contain timestamp, process-local sequence, classification and bounded invocation name; arguments and exception payloads are omitted. This is diagnostic metadata, not a memory dump. Automatic acknowledgment timeouts, rotation, cross-process coordination, crash durability and WASM storage are pending.

## Registered commands

`gpl`, `gnu`, `rfc`, `iso`, `iec`, `time`, `date`, `timedate`, `token`, `character`, `string`, `integer`, `float`, `double`, `array`, `boolean`, `call`, `return`, `variable`, `function`, `class`, `object`, `instance`, `parameter`, `argument`, `reference`, `value`, `regulation`, `ipify`, `dump`, `invoke`, `signal`, `trigger`, `timestamp`, `help`, `echo`, `exit`, `cpp`, `wasm`, `code`, `codebase`, `sandbox`, `sdk`, `aes`, `gcm`, `rsa`, `oaep`, `pss`, `sha`, `base64`, `roundrobin`, `heartbeat`, `flow`, `lambda`, `index`, `pool`, `farm`, `reaction`, `reflex`, `fuse`, `rpc`, `bridge`, `mesh`, `chain`, `ring`, `route`, `path`, `grid`, `matrix`, `lane`, `session`, `configure`, `uri`, `url`, `api`, `cli`, `mcp`, `rdp`, `vnc`, `ssh`.

## Build and usage

The supplied [build.bash](build.bash) targets C++17 and WASI SDK. It requires a native C++ compiler, Bash and an installed WASI SDK. Native IP discovery additionally requires curl.

```bash
export WASI_SDK_PATH=/path/to/wasi-sdk
bash build.bash
```

The script is intended to produce `tool` and `tool.wasm` and run a basic native smoke check. These outputs and successful execution have not yet been verified. WASI support for threads, filesystem and networking requires separate validation and host capabilities.

Illustrative commands (subject to build verification):

```bash
./tool help
./tool echo hello
./tool api echo hello
./tool timestamp now
./tool reflex session1
./tool reaction session1 api cli mcp ssh
./tool dump location
./tool
```

In the interactive runtime, enter `ipify enable`, then `ipify status` after the background request completes. Enter `ipify disable` to stop optional observation. In-memory state does not persist across separate executable launches.

## Development priorities

1. Verify native and WASI builds and correct compilation defects.
2. Establish invocation identity, immutable context and typed outcomes.
3. Complete reciprocal heartbeat/reflex and roundrobin/flow feedback with bounded signal propagation.
4. Reconcile pooling around invocation passage rather than resource leasing alone.
5. Implement acknowledgment deadlines, local diagnostic retention and optional IP context association.
6. Implement real transport adapters, security primitives and controlled execution environments only after their contracts and tests are established.
7. Audit concurrency, parallelism, lifecycle transitions, security, performance and recovery.

Tool is an architectural foundation, not a production-ready remote access system, cryptographic library or standards-compliant MCP/SSH/RDP/VNC server.

## Repository

- [tool.cpp](tool.cpp): decomposed C++ monolith.
- [build.bash](build.bash): native and WASI build preparation.
- [LICENSE](LICENSE): GNU GPL v3.
