// Copyright (C) 2026 Daniel Alexander Aspelin
// SPDX-License-Identifier: GPL-3.0-only

#include "units/passage.hpp"
#if defined(TOOL_BROWSER_WASM)
#include "units/browser.hpp"
#else
#include <deque>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <atomic>
#include <future>
#include <utility>
#include <exception>
#include <stdexcept>
#include <memory>
#include <set>
#include <queue>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cerrno>
#include <cstdlib>
#if defined(TOOL_CRYPTO) && defined(__linux__) && !defined(__wasi__)
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif
#include <csignal>
#include <cstdio>
#include <thread>
#include <variant>
#include <cmath>
#include <charconv>
#include <optional>
#include <locale>
#include <cstdint>
#include <condition_variable>
#include <limits>
#if defined(__unix__) && !defined(__wasi__)
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/socket.h>
#if defined(__linux__)
#include <sys/random.h>
#endif
#include <poll.h>
#endif

#if defined(__linux__) && !defined(__wasi__)
namespace native {
#include "units/temporarydirectory.hpp"

#include "units/run.hpp"

#include "units/regularfile.hpp"
}
#endif

namespace toolcatalogue {
#include "units/entry.hpp"
constexpr Entry entries[]{
    {"mainframes", "-", "Mainframes Digital launch foundation", "local preview; gateway pending"},
    {"platforms", "mainframes", "Hardware, OS and delivery options", "reference"},
    {"ibm-z", "platforms", "IBM Z: Linux on Z, z/OS and logical partitions", "https://www.ibm.com/products/z/linux"},
    {"linuxone", "platforms", "LinuxONE models and supported Linux distributions", "https://www.ibm.com/support/pages/linux-ibm-zibm-linuxone-tested-platforms"},
    {"s390x-vpc", "platforms", "IBM Cloud LinuxONE VPC retirement", "https://cloud.ibm.com/docs/vpc?topic=vpc-vs-profiles"},
    {"development", "platforms", "IBM Wazi development and test for z/OS", "https://www.ibm.com/products/wazi-as-a-service"},
    {"hosting", "platforms", "Provider, tenancy and commercial capacity options", "qualified leads; current scope and terms require confirmation"},
    {"kyndryl", "hosting", "Kyndryl zCloud managed IBM mainframe capacity", "https://www.kyndryl.com/content/dam/kyndrylprogram/doc/en/2026/idc-worldwide-mainframe-modernization-infrastructure.pdf"},
    {"fujitsu", "hosting", "Fujitsu IBM mainframe LPAR service description", "https://assets.applytosupply.digitalmarketplace.service.gov.uk/g-cloud-14/documents/92271/240552160710426-service-definition-document-2024-04-30-1342.pdf"},
    {"power", "platforms", "IBM Power Virtual Server, a separate hardware family", "https://cloud.ibm.com/power/overview"},
    {"operations", "mainframes", "Session, gateway, audit and hosting", "local preview; hosted enforcement pending"},
    {"gateway", "operations", "Stable HTTPS ingress, opaque locators, and authenticated tenancy", "local route preview; hosted integration pending"},
    {"tls-identity", "gateway", "TLS service identity and wildcard matching", "https://www.rfc-editor.org/rfc/rfc9525.html"},
    {"authenticator", "gateway", "Standard authenticators and verifier responsibilities", "https://pages.nist.gov/800-63-4/sp800-63b/authenticators/"},
    {"pages", "gateway", "Public static assets and workflow-hosted credentials", "https://docs.github.com/en/pages/getting-started-with-github-pages/what-is-github-pages"},
    {"integratives", "operations", "Browser routing, staged loading and volatile lifecycle reference", "https://github.com/Integratives/integratives.github.io/tree/a9a50c708abbffea66f8f31a9b0b97b4147bf3d3"},
    {"ixv2", "mainframes", "Internet Exchange Version 2", "Vercel inventory inspected; Tool mapping only"},
    {"ixv2-public", "ixv2", "Read-only public projection", "GET or HEAD /api/public"},
    {"ixv2-session", "ixv2", "Session and authorized exchange entry", "/api/session; /api/integration requires entered session"},
    {"ixv2-runtime", "ixv2", "Manifest-bound exchange runtime", "/exchange/* rewrites to /api/runtime"},
    {"ixv2-design", "ixv2", "Commission, Mission, Authority, launch and recovery", "https://chatgpt.com/share/6b094b3d-7460-83ed-9cf1-e1158ac4b9fa"},
    {"linguist", "mainframes", "Optional language assurance and reciprocal capability contract", "https://github.com/DanielAspelin/Linguist/tree/835d0d4"}
};

#include "units/read.hpp"
}

#include "units/timestamp.hpp"
#include "units/ipify.hpp"
#include "units/dump.hpp"
#include "units/signal.hpp"
#include "units/trigger.hpp"
#include "units/cpu.hpp"
#include "units/memory.hpp"
#include "units/thermal.hpp"
#include "units/capacity.hpp"
#include "units/pressure.hpp"
#include "units/conflict.hpp"
#include "units/throttle.hpp"
#include "units/guard.hpp"
#include "units/queue.hpp"
#include "units/watchdog.hpp"
#include "units/cleanup.hpp"
#include "units/scheduler.hpp"
#include "units/tool.hpp"
#include "units/registration.hpp"

#include "units/value.hpp"
#include "units/parameter.hpp"
#include "units/reference.hpp"
#include "units/argument.hpp"
#include "units/variable.hpp"
#include "units/return.hpp"
#include "units/function.hpp"
#include "units/call.hpp"
#include "units/class.hpp"
#include "units/object.hpp"
#include "units/instance.hpp"
#include "units/array.hpp"
#include "units/boolean.hpp"
#include "units/integer.hpp"
#include "units/float.hpp"
#include "units/double.hpp"
#include "units/character.hpp"
#include "units/string.hpp"
#include "units/linguist.hpp"

namespace languages {
#include "units/source.hpp"

// BEGIN GENERATED LANGUAGE CATALOGUE
constexpr Source sources[] = {
    {"glottolog", "human", "Glottolog 5.4 CLDF", "https://doi.org/10.5281/zenodo.3260727", 27275},
    {"iana", "human", "2026-11-18", "https://www.iana.org/assignments/language-subtag-registry", 9298},
    {"iso639-3", "human", "2026-07-15 via Unicode CLDR", "https://iso639-3.sil.org/code_tables/download_tables", 7927},
    {"iso639-3-retired", "human", "2026-07-15 via Unicode CLDR", "https://iso639-3.sil.org/code_tables/download_tables", 388},
    {"iso639-2", "human", "2026-11-27 snapshot", "https://www.loc.gov/standards/iso639-2/", 487},
    {"cldr", "human", "951424004be6", "https://cldr.unicode.org/", 1233},
    {"pldb", "programming", "2026-11-27 snapshot", "https://pldb.io/", 5172},
    {"github-linguist", "programming", "f6e9adfc7240", "https://github.com/github-linguist/linguist/tree/f6e9adfc7240b5b1f098bc9d4722cb6b3b210049", 855},
    {"pygments", "programming", "2026-11-27 snapshot", "https://pygments.org/docs/lexers/", 602},
    {"elf", "machine", "gABI registry snapshot", "https://gabi.xinuos.com/v42/elf/a-emachine.html", 212},
    {"pe", "machine", "Microsoft PE registry snapshot", "https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#machine-types", 35},
    {"mach-o", "machine", "Apple XNU CPU type snapshot", "https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/mach/machine.h", 16},
    {"llvm", "machine", "LLVM ArchType snapshot", "https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/TargetParser/Triple.h", 63},
    {"machine-spec", "machine", "Tool reference index", "https://www.w3.org/TR/wasm-core-2/", 24},
    {"spirv-opcode", "machine", "2026-11-27 snapshot", "https://registry.khronos.org/SPIR-V/", 893},
    {"riscv-opcode", "machine", "2026-11-27 snapshot", "https://github.com/riscv/riscv-opcodes", 1946},
    {"wasm-opcode", "machine", "2026-11-27 snapshot", "https://github.com/WebAssembly/wabt/blob/main/include/wabt/opcode.def", 553},
    {"jvm-opcode", "machine", "2026-11-27 snapshot", "https://docs.oracle.com/javase/specs/jvms/se25/html/jvms-6.html", 205},
};
constexpr std::string_view snapshots[] = {
#include "units/snapshots-01.hpp"
#include "units/snapshots-02.hpp"
#include "units/snapshots-03.hpp"
#include "units/snapshots-04.hpp"
#include "units/snapshots-05.hpp"
#include "units/snapshots-06.hpp"
#include "units/snapshots-07.hpp"
#include "units/snapshots-08.hpp"
#include "units/snapshots-09.hpp"
#include "units/snapshots-10.hpp"
#include "units/snapshots-11.hpp"
#include "units/snapshots-12.hpp"
#include "units/snapshots-13.hpp"
};
// END GENERATED LANGUAGE CATALOGUE

#include "units/record.hpp"

#include "units/take.hpp"

#include "units/records.hpp"

#include "units/sourcefor.hpp"

#include "units/validdomain.hpp"

#include "units/folded.hpp"

#include "units/relevance.hpp"

#include "units/number.hpp"

#include "units/append.hpp"

#include "units/invoke.hpp"
}

namespace invocation {
using Words = TOOL::Arguments;

#include "units/token.hpp"

#include "units/tokeniser.hpp"

#include "units/detokeniser.hpp"

#include "units/ir.hpp"

#include "units/preview.hpp"

#include "units/resolve.hpp"

#include "units/run-2.hpp"

#include "units/file.hpp"

#include "units/guardtrail.hpp"
#include "units/auditrail.hpp"
#include "units/changelog.hpp"
#include "units/backlog.hpp"
#include "units/changerequest.hpp"
#include "units/landmark.hpp"
#include "units/benchmark.hpp"
#include "units/summary.hpp"
#include "units/documentation.hpp"
#include "units/specification.hpp"
#include "units/reference-2.hpp"
#include "units/glossary.hpp"

#include "units/describe.hpp"
}

namespace artifact {
#if defined(TOOL_CRYPTO) && defined(__linux__) && !defined(__wasi__)
using Bytes = std::vector<unsigned char>;
constexpr std::size_t payloadLimit = 8 * 1024 * 1024;
constexpr std::string_view magic = "TOOL-CAPSULE-1";

#include "units/read-2.hpp"

#include "units/hex.hpp"

#include "units/unhex.hpp"

#include "units/digest.hpp"

#include "units/key.hpp"

#include "units/crypt.hpp"

#include "units/encode.hpp"

#include "units/decode.hpp"

#include "units/publish.hpp"

#include "units/validformat.hpp"

#include "units/pack.hpp"

#include "units/unpack.hpp"
#endif

#include "units/invoke-2.hpp"
}

namespace workspace {
using Arguments = TOOL::Arguments;

#include "units/datatype.hpp"
#include "units/datamode.hpp"
#include "units/datastate.hpp"
#include "units/status.hpp"

#include "units/item.hpp"

#include "units/inventory.hpp"

#include "units/integrity.hpp"
#include "units/consistency.hpp"
#include "units/finalisation.hpp"

#include "units/content.hpp"

#include "units/arrangement.hpp"

#include "units/transaction.hpp"
#include "units/log.hpp"
#include "units/report.hpp"
#include "units/narration.hpp"
using NARRATIVE = NARRATION;

#include "units/journal.hpp"

#include "units/prelaunch.hpp"
#include "units/liftoff.hpp"
#include "units/postlaunch.hpp"
#include "units/sequence.hpp"
#include "units/initiation.hpp"
constexpr unsigned entrancePort = 9966;
constexpr unsigned lockedPort = 6699;

#include "units/location.hpp"
#include "units/position.hpp"
#include "units/numeric.hpp"

#include "units/position-2.hpp"

#include "units/recognition.hpp"
#include "units/synthesis.hpp"

#include "units/measurement.hpp"
using LENGTH = MEASUREMENT<0, 2147483647>;
using WIDTH = LENGTH;
using HEIGHT = LENGTH;
using ALTITUDE = MEASUREMENT<-2147483647, 2147483647>;
using LATITUDE = MEASUREMENT<-90, 90>;
using LONGITUDE = MEASUREMENT<-180, 180>;
using DECIBEL = ALTITUDE;
using HERTZ = LENGTH;
using RPM = LENGTH;
using HUMIDITY = MEASUREMENT<0, 100>;
#include "units/temperature.hpp"
#include "units/compass.hpp"
using WINDDIRECTION = COMPASS;
using WINDPRESSURE = LENGTH;

#include "units/measure.hpp"

#include "units/package.hpp"
constexpr Package packages[] {
    {"gps", "GPS · GNSS · NMEA · RMC · GGA · UTC · WGS84 · DOP · HDOP", "https://www.gps.gov/interface-control-documents-icds-interface-specifications-iss", "Local coordinate validation; receiver and NMEA adapter pending."},
    {"nfc", "NFC · NDEF · RTD · TNEP · APDU · NCI · LLCP · HCE", "https://nfc-forum.org/build/specifications/", "NDEF read on supported browsers; native NFC reader discovery."},
    {"gsm", "GSM · SIM · SMS · USSD · GPRS · EDGE · PLMN · AT", "https://portal.3gpp.org/desktopmodules/Specifications/SpecificationDetails.aspx?specificationId=1515", "Reference profile; modem and carrier adapters pending."},
    {"tcpip", "TCP · UDP · IPv4 · IPv6 · ICMP · DNS · DHCP · TLS · HTTP", "https://www.rfc-editor.org/rfc/rfc9293", "Loopback HTTP/TCP in the workspace; other protocols require adapters."},
    {"rfid", "RFID · EPC · UHF · HF · LF · TID · LLRP · RAIN", "https://ref.gs1.org/architecture/system-architecture/", "Reference profile; reader and tag inventory adapters pending."},
};
#include "units/abbreviation.hpp"

#include "units/packageinfo.hpp"

constexpr std::string_view workspaceHtml = R"toolhtml(
<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Tool Workspace</title><link rel="stylesheet" href="/workspace.css"><script defer src="/workspace.js"></script></head>
<body><header><a class="brand" href="/" aria-label="Tool Workspace"><span aria-hidden="true">⬡</span> TOOL <small>WORKSPACE</small></a>
<div class="environment"><span id="runtime"></span><span id="device"></span></div></header>
<main><nav aria-label="Workspace"><button data-open="language" type="button">Languages</button><button data-open="launch" type="button">Launch</button><button data-open="packages" type="button">Packages</button><button data-open="journal" type="button">Journal</button></nav>
<section id="windows" aria-label="Workspace windows"></section></main>
<dialog id="dialog" aria-labelledby="dialog-title"><div class="titlebar"><h2 id="dialog-title"></h2><button class="close" type="button" aria-label="Close dialog">×</button></div><div class="body" id="dialog-content"></div></dialog>
<template id="language"><form class="query"><label>Domain<select name="domain"><option value="programming">Programming</option><option value="human">Human</option><option value="machine">Machine</option></select></label><label>Subject<input name="subject" required maxlength="80" value="cpp"></label><label>Topic<input name="topic" maxlength="80" value="syntax"></label><button class="primary">Open content</button></form><pre class="result" aria-live="polite"></pre></template>
<template id="launch"><pre class="result" aria-live="polite"></pre><button class="speak" type="button">Read sequence aloud</button></template>
<template id="packages"><pre class="result" aria-live="polite"></pre></template>
<template id="journal"><div class="journal-tabs"><button type="button" data-kind="transaction">Transactions</button><button type="button" data-kind="log">Logs</button><button type="button" data-kind="report">Reports</button></div><pre class="result" aria-live="polite"></pre></template>
</body></html>
)toolhtml";

constexpr std::string_view workspaceCss = R"toolcss(
:root{color-scheme:dark;font:16px/1.5 system-ui,sans-serif;background:#10151d;color:#eaf1f8;--edge:#354654;--surface:#161e29;--accent:#70e1d0}
*{box-sizing:border-box}body{margin:0;min-height:100dvh;background:radial-gradient(ellipse at 68% 0%,#203a40 0%,transparent 62%),#10151d}
header{height:64px;display:flex;align-items:center;justify-content:space-between;padding:0 28px;border-bottom:1px solid var(--edge);background:#101820e8}
.brand{color:#f1f8ff;text-decoration:none;letter-spacing:.13em;font-weight:800;display:flex;align-items:center;gap:12px}
.brand span{color:var(--accent);font-size:29px;line-height:1}.brand small{font-size:.64rem;color:#a8bcc8;letter-spacing:.15em}
.environment{display:flex;gap:18px;font-size:.75rem;color:#b9c9d5;text-transform:capitalize}
main{max-width:1480px;margin:auto;padding:28px;display:grid;grid-template-columns:154px minmax(0,1fr);gap:22px}
nav{display:grid;align-content:start;gap:8px}button,input,select{font:inherit}button{background:#202d3b;color:inherit;border:1px solid var(--edge);border-radius:8px;cursor:pointer;padding:9px 13px}nav button{text-align:left}button:hover,button[aria-pressed=true]{border-color:var(--accent);background:#273c44}button:focus-visible,input:focus-visible,select:focus-visible{outline:2px solid var(--accent);outline-offset:3px}
#windows{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:18px;align-items:start}
.window,dialog{min-width:0;background:var(--surface);color:inherit;border:1px solid var(--edge);border-radius:13px;overflow:hidden;box-shadow:0 18px 48px #0005}
.titlebar{display:flex;align-items:center;justify-content:space-between;padding:11px 16px;background:#1e2d38;border-bottom:1px solid var(--edge)}h2{font-size:1rem;margin:0}.close{padding:0 9px;font-size:1.2rem}
.body{padding:20px}.query{display:grid;grid-template-columns:repeat(3,minmax(0,1fr)) auto;gap:9px;align-items:end}label{display:grid;gap:5px;font-size:.8rem;color:#bfd0de}input,select{width:100%;min-width:0;background:#121b24;color:#edf6ff;border:1px solid #526777;border-radius:6px;padding:8px}.primary{background:var(--accent);color:#082423;font-weight:750;white-space:nowrap}.primary:hover{color:#eefaf7}
.result{font:13px/1.6 ui-monospace,monospace;white-space:pre-wrap;overflow-wrap:anywhere;min-height:44px;margin:12px 0 0;padding:12px;background:#101923;border-radius:8px;color:#dcebf7}.speak{margin-top:12px}.journal-tabs{display:flex;flex-wrap:wrap;gap:7px}
dialog{padding:0;width:min(760px,calc(100% - 32px));max-height:calc(100dvh - 40px);overflow:auto}dialog::backdrop{background:#050a12b3}
html[data-device=tablet] main,html[data-device=mobile] main{display:block}
html[data-device=tablet] nav,html[data-device=mobile] nav{grid-template-columns:repeat(4,minmax(0,1fr))}
html[data-device=tablet] nav button,html[data-device=mobile] nav button{text-align:center}
html[data-device=mobile] header{padding:0 13px}html[data-device=mobile] .brand small{display:none}html[data-device=mobile] .environment{gap:9px}
html[data-device=mobile] main{padding:14px}html[data-device=mobile] nav{grid-template-columns:repeat(2,minmax(0,1fr))}
html[data-device=mobile] dialog{width:calc(100% - 20px);margin:auto auto 10px;max-height:calc(100dvh - 24px)}
@media(max-width:720px){.query{grid-template-columns:1fr 1fr}.primary{white-space:normal}}
)toolcss";

constexpr std::string_view workspaceJs = R"tooljs(
"use strict";
const titles = {language:"Languages", launch:"Launch sequence", packages:"Protocol packages", journal:"Journal"};
const windows = document.querySelector("#windows");
const dialog = document.querySelector("#dialog");
const dialogContent = document.querySelector("#dialog-content");
const cache = new Map(), pending = new WeakMap();
let active = "language", device = "", preference = "auto";
const narrow = matchMedia("(max-width: 660px)"), medium = matchMedia("(max-width: 1020px)");

async function get(path, output) {
  const request = new AbortController();
  pending.get(output)?.abort();
  pending.set(output, request);
  output.textContent = "Loading…";
  try {
    const response = await fetch(path, {signal:request.signal});
    const message = await response.text();
    if (!request.signal.aborted) output.textContent = message;
  } catch {
    if (!request.signal.aborted) output.textContent = "The local runtime is unavailable.";
  }
}

function content(id) {
  if (cache.has(id)) return cache.get(id);
  const body = document.createElement("div");
  body.append(document.querySelector("template#" + id).content.cloneNode(true));
  const result = body.querySelector(".result");
  if (id === "language") {
    const form = body.querySelector("form");
    const submit = event => {
      event?.preventDefault();
      get("/api/language?" + new URLSearchParams(new FormData(form)), result);
    };
    form.addEventListener("submit", submit);
    form.elements.domain.addEventListener("change", () => {
      form.elements.subject.value = {human:"grammar", programming:"cpp", machine:"wasm"}[form.elements.domain.value];
      form.elements.topic.value = "syntax";
      submit();
    });
    submit();
  } else if (id === "journal") {
    const select = kind => {
      body.querySelectorAll("[data-kind]").forEach(b => b.setAttribute("aria-pressed", String(b.dataset.kind === kind)));
      get("/api/journal?kind=" + kind, result);
    };
    body.querySelectorAll("[data-kind]").forEach(b => b.addEventListener("click", () => select(b.dataset.kind)));
    select("transaction");
  } else {
    get(id === "launch" ? "/api/launch" : "/api/packages", result);
    const speak = body.querySelector(".speak");
    if (speak && !("speechSynthesis" in window)) speak.hidden = true;
    if (speak && !speak.hidden) speak.addEventListener("click", () => {
      speechSynthesis.cancel();
      speechSynthesis.speak(new SpeechSynthesisUtterance(result.textContent));
    });
  }
  cache.set(id, body);
  return body;
}

function openView(id) {
  if (!device) return;
  active = id;
  document.querySelectorAll("[data-open]").forEach(b => b.setAttribute("aria-pressed", String(b.dataset.open === id)));
  if (device !== "desktop") {
    document.querySelector("#dialog-title").textContent = titles[id];
    dialogContent.replaceChildren(content(id));
    if (!dialog.open) dialog.showModal();
    return;
  }
  let panel = windows.querySelector('[data-window="' + id + '"]');
  if (!panel) {
    panel = document.createElement("section");
    panel.className = "window";
    panel.dataset.window = id;
    const heading = document.createElement("div");
    heading.className = "titlebar";
    const title = document.createElement("h2");
    title.textContent = titles[id];
    const close = document.createElement("button");
    close.className = "close";
    close.textContent = "×";
    close.setAttribute("aria-label", "Close " + titles[id]);
    close.addEventListener("click", () => {
      panel.remove();
      const tab = document.querySelector('[data-open="' + id + '"]');
      tab.setAttribute("aria-pressed", "false");
      tab.focus();
    });
    heading.append(title, close);
    const body = document.createElement("div");
    body.className = "body";
    body.append(content(id));
    panel.append(heading, body);
    windows.append(panel);
  }
  panel.scrollIntoView({block:"nearest"});
}

function arrange() {
  const next = preference === "auto" ? narrow.matches ? "mobile" : medium.matches ? "tablet" : "desktop" : preference;
  if (next === device) return;
  const wasOpen = dialog.open || windows.children.length > 0;
  if (dialog.open) dialog.close();
  windows.replaceChildren();
  device = next;
  document.documentElement.dataset.device = next;
  document.querySelector("#device").textContent = next;
  if (wasOpen || device === "desktop") openView(active);
}

async function start() {
  try {
    const response = await fetch("/api/runtime");
    const [mode, state, arrangeMode] = (await response.text()).trim().split("\t");
    if (!response.ok || !["auto","desktop","tablet","mobile"].includes(arrangeMode)) throw new Error();
    preference = arrangeMode;
    document.querySelector("#runtime").textContent = mode + " · " + state;
    arrange();
  } catch {
    document.querySelector("#runtime").textContent = "Runtime unavailable";
  }
}
document.querySelectorAll("[data-open]").forEach(b => b.addEventListener("click", () => openView(b.dataset.open)));
document.querySelector("#dialog .close").addEventListener("click", () => dialog.close());
narrow.addEventListener("change", arrange);
medium.addEventListener("change", arrange);
start();
)tooljs";

#if defined(__linux__) && !defined(__wasi__)
#include "units/descriptor.hpp"

std::atomic<bool> stopRequested{false};
static_assert(std::atomic<bool>::is_always_lock_free, "Expected lock-free shutdown flag");
#include "units/stop.hpp"

#include "units/signals.hpp"

#include "units/urldecode.hpp"

#include "units/reply.hpp"

#include "units/transport.hpp"

#include "units/handoff.hpp"

#include "units/http.hpp"
#include "units/bindsocket.hpp"

#include "units/discovery.hpp"
#endif

#include "units/launch.hpp"
}


#include "units/token-2.hpp"
#include "units/time.hpp"
#include "units/date.hpp"
#include "units/timedate.hpp"
#include "units/gpl.hpp"
#include "units/gnu.hpp"
#include "units/rfc.hpp"
#include "units/iso.hpp"
#include "units/iec.hpp"
#include "units/assembler.hpp"
#include "units/disassembler.hpp"
#include "units/socket.hpp"
#include "units/service.hpp"
#include "units/runtime.hpp"
#include "units/capsule.hpp"
#include "units/quarantine.hpp"
#include "units/environment.hpp"
#include "units/facility.hpp"
#include "units/audit.hpp"
#include "units/forensic.hpp"
#include "units/statistical.hpp"
#include "units/analytical.hpp"
#include "units/status-2.hpp"
#include "units/qr.hpp"
#include "units/ean.hpp"
#include "units/hotcode.hpp"
#include "units/uptime.hpp"
#include "units/downtime.hpp"
#include "units/triggered.hpp"
#include "units/resolved.hpp"
#include "units/locator.hpp"
#include "units/positioner.hpp"
#include "units/prepositioner.hpp"
#include "units/passthrough.hpp"
#include "units/round.hpp"
#include "units/through.hpp"
#include "units/velocity.hpp"
#include "units/frequency.hpp"
#include "units/entry-2.hpp"
#include "units/exit.hpp"
#include "units/input.hpp"
#include "units/output.hpp"
#include "units/shell.hpp"
#include "units/terminal.hpp"
#include "units/console.hpp"
#include "units/persistence.hpp"
#include "units/volatility.hpp"
#include "units/idle.hpp"
#include "units/sleep.hpp"
#include "units/detection.hpp"
#include "units/callable.hpp"
#include "units/invokable.hpp"
#include "units/returnable.hpp"
#include "units/compiler.hpp"
#include "units/decompiler.hpp"
#include "units/hex-2.hpp"
#include "units/hexadecimal.hpp"
#include "units/hextable.hpp"
#include "units/automation.hpp"
#include "units/generation.hpp"
#include "units/encryption.hpp"
#include "units/decryption.hpp"
#include "units/encoding.hpp"
#include "units/decoding.hpp"
#include "units/lexer.hpp"
#include "units/delexer.hpp"
#include "units/lexicaliser.hpp"
#include "units/delexicaliser.hpp"
#include "units/tokeniser-2.hpp"
#include "units/detokeniser-2.hpp"
#include "units/parser.hpp"
#include "units/transparser.hpp"
#include "units/crossparser.hpp"
#include "units/process.hpp"
#include "units/processor.hpp"
#include "units/allocator.hpp"
#include "units/serialiser.hpp"
#include "units/deserialiser.hpp"
#include "units/hasher.hpp"
#include "units/salter.hpp"
#include "units/lockmaker.hpp"
#include "units/keymaker.hpp"
#include "units/builder.hpp"
#include "units/linker.hpp"
#include "units/debugger.hpp"
#include "units/scanner.hpp"
#include "units/printer.hpp"
#include "units/port.hpp"
#include "units/finder.hpp"
#include "units/search.hpp"
#include "units/cloaker.hpp"
#include "units/concealer.hpp"
#include "units/exposer.hpp"
#include "units/importer.hpp"
#include "units/exporter.hpp"
#include "units/clipboard.hpp"
#include "units/cacher.hpp"
#include "units/interface.hpp"
#include "units/driver.hpp"
#include "units/controller.hpp"
#include "units/device.hpp"
#include "units/system.hpp"
#include "units/type.hpp"
#include "units/mode.hpp"
#include "units/condition.hpp"
#include "units/state.hpp"
#include "units/network.hpp"
#include "units/tcpip.hpp"
#include "units/gps.hpp"
#include "units/gsm.hpp"
#include "units/imei.hpp"
#include "units/rfid.hpp"
#include "units/ram.hpp"
#include "units/gpu.hpp"
#include "units/sd.hpp"
#include "units/dev.hpp"
#include "units/lan.hpp"
#include "units/wan.hpp"
#include "units/host.hpp"
#include "units/hostname.hpp"
#include "units/tcp.hpp"
#include "units/ip.hpp"
#include "units/udp.hpp"
#include "units/icmp.hpp"
#include "units/nat.hpp"
#include "units/mac.hpp"
#include "units/fat.hpp"
#include "units/fat32.hpp"
#include "units/ntfs.hpp"
#include "units/apfs.hpp"
#include "units/ext4.hpp"
#include "units/exfat.hpp"
#include "units/aarch.hpp"
#include "units/arm.hpp"
#include "units/amr64.hpp"
#include "units/x86.hpp"
#include "units/x86-64.hpp"
#include "units/amd64.hpp"
#include "units/riscv.hpp"
#include "units/psu.hpp"
#include "units/power.hpp"
#include "units/oauth.hpp"
namespace protocols {
#include "units/ssl.hpp"
} // namespace protocols
#include "units/tls.hpp"
#include "units/http-2.hpp"
#include "units/https.hpp"
#include "units/ftp.hpp"
#include "units/dns.hpp"
#include "units/dhcp.hpp"
#include "units/vpn.hpp"
#include "units/proxy.hpp"
#include "units/firewall.hpp"
#include "units/uuid.hpp"
#include "units/enter.hpp"
#include "units/esc.hpp"
#include "units/ctrl.hpp"
#include "units/alt.hpp"
#include "units/space.hpp"
#include "units/shift.hpp"
#include "units/tab.hpp"
#include "units/name.hpp"
#include "units/number-2.hpp"
#include "units/pid.hpp"
#include "units/usb.hpp"
#include "units/arpa.hpp"
#include "units/provider.hpp"
#include "units/sms.hpp"
#include "units/tld.hpp"
#include "units/icann.hpp"
#include "units/iana.hpp"
#include "units/ietf.hpp"
#include "units/w3c.hpp"
#include "units/www.hpp"
#include "units/node.hpp"
#include "units/nodejs.hpp"
#include "units/image.hpp"
#include "units/img.hpp"
#include "units/block.hpp"
#include "units/line.hpp"
#include "units/endoffile.hpp"
#include "units/script.hpp"
#include "units/sourcecode.hpp"
#include "units/binary.hpp"
#include "units/source-2.hpp"
#include "units/destination.hpp"
#include "units/target.hpp"
#include "units/certificate.hpp"
#include "units/passphrase.hpp"
#include "units/password.hpp"
#include "units/username.hpp"
#include "units/user.hpp"
#include "units/secret.hpp"
#include "units/database.hpp"
#include "units/table.hpp"
#include "units/column.hpp"
#include "units/row.hpp"
#include "units/tuple.hpp"
#include "units/query.hpp"
#include "units/policy.hpp"
#include "units/map.hpp"
#include "units/tree.hpp"
#include "units/mindmap.hpp"
#include "units/markdown.hpp"
#include "units/pdf.hpp"
#include "units/json.hpp"
#include "units/xml.hpp"
#include "units/csv.hpp"
#include "units/signature.hpp"
#include "units/author.hpp"
#include "units/sender.hpp"
#include "units/receiver.hpp"
#include "units/transmission.hpp"
#include "units/wifi.hpp"
#include "units/hotspot.hpp"
#include "units/tether.hpp"
#include "units/ledger.hpp"
#include "units/router.hpp"
#include "units/switch.hpp"
#include "units/gateway.hpp"
#include "units/subnet.hpp"
#include "units/ipv4.hpp"
#include "units/ipv6.hpp"
#include "units/paging.hpp"
#include "units/command.hpp"
#include "units/instruction.hpp"
#include "units/bash.hpp"
#include "units/ps1.hpp"
#include "units/applescript.hpp"
#include "units/cmd.hpp"
#include "units/batch.hpp"
#include "units/windows.hpp"
#include "units/macos.hpp"
#include "units/linux.hpp"
#include "units/android.hpp"
#include "units/ios.hpp"
#include "units/debian.hpp"
#include "units/fedora.hpp"
#include "units/gentoo.hpp"
#include "units/patcher.hpp"
#include "units/wrapper.hpp"
#include "units/forker.hpp"
#include "units/wiring.hpp"
#include "units/volume.hpp"
#include "units/cluster.hpp"
#include "units/replication.hpp"
#include "units/migration.hpp"
#include "units/unit.hpp"
#include "units/installation.hpp"
#include "units/packaging.hpp"
#include "units/package-2.hpp"
#include "units/packet.hpp"
#include "units/filter.hpp"
#include "units/sensor.hpp"
#include "units/proximity.hpp"
#include "units/distribution.hpp"
#include "units/grouping.hpp"
#include "units/resourcing.hpp"
#include "units/hardware.hpp"
#include "units/firmware.hpp"
#include "units/bootloading.hpp"
#include "units/bootstrapping.hpp"
#include "units/hosting.hpp"
#include "units/site.hpp"
#include "units/domain.hpp"
#include "units/forest.hpp"
#include "units/global.hpp"
#include "units/local.hpp"
#include "units/joiner.hpp"
#include "units/leaver.hpp"
#include "units/online.hpp"
#include "units/offline.hpp"
#include "units/daemon.hpp"
#include "units/kernel.hpp"
#include "units/ko.hpp"
#include "units/kbuild.hpp"
#include "units/instrument.hpp"
#include "units/cloud.hpp"
#include "units/datacenter.hpp"
#include "units/frame.hpp"
#include "units/main.hpp"
#include "units/mainsystem.hpp"
#include "units/allocation.hpp"
#include "units/processing.hpp"
#include "units/versioning.hpp"
#include "units/transversioning.hpp"
#include "units/qualification.hpp"
#include "units/verification.hpp"
#include "units/validation.hpp"
#include "units/confirmation.hpp"
#include "units/authentication.hpp"
#include "units/authorisation.hpp"
#include "units/approval.hpp"
#include "units/monitoring.hpp"
#include "units/analysis.hpp"
#include "units/statistics.hpp"
#include "units/science.hpp"
#include "units/calculator.hpp"
#include "units/arithmetics.hpp"
#include "units/qualifier.hpp"
#include "units/identification.hpp"
#include "units/identifier.hpp"
#include "units/quantifier.hpp"
#include "units/javascript.hpp"
#include "units/r.hpp"
#include "units/c.hpp"
#include "units/assembly.hpp"
#include "units/lua.hpp"
#include "units/css.hpp"
#include "units/html.hpp"
#include "units/bluetooth.hpp"
#include "units/nfc.hpp"
#include "units/microsd.hpp"
#include "units/storage.hpp"
#include "units/disk.hpp"
#include "units/rest.hpp"
#include "units/directory.hpp"
namespace tool_profiles {
#include "units/file-2.hpp"
} // namespace tool_profiles

#include "units/management.hpp"
#include "units/manager.hpp"
#include "units/operator.hpp"
#include "units/transport-2.hpp"
#include "units/clock.hpp"
#include "units/info.hpp"
#include "units/dashboard.hpp"
#include "units/average.hpp"
#include "units/sum.hpp"
#include "units/summary-2.hpp"
#include "units/summarisation.hpp"
#include "units/glossary-2.hpp"
#include "units/document.hpp"
#include "units/documentation-2.hpp"
#include "units/recording.hpp"
#include "units/blackbox.hpp"
#include "units/whitebox.hpp"
#include "units/legend.hpp"
#include "units/boundary.hpp"
#include "units/rule.hpp"
#include "units/regulator.hpp"
#include "units/regulation.hpp"
#include "units/loop.hpp"
#include "units/while.hpp"
#include "units/for.hpp"
#include "units/if.hpp"
#include "units/else.hpp"
#include "units/elseif.hpp"
#include "units/injection.hpp"
#include "units/functor.hpp"
#include "units/functional.hpp"
#include "units/vector.hpp"
#include "units/graphics.hpp"
#include "units/gdt.hpp"
#include "units/mbr.hpp"
#include "units/bootsector.hpp"
#include "units/superblock.hpp"
#include "units/chunk.hpp"
#include "units/bitrate.hpp"
#include "units/sample.hpp"
#include "units/examination.hpp"
#include "units/inspection.hpp"
#include "units/forensics.hpp"
#include "units/analytics.hpp"
#include "units/forecast.hpp"
#include "units/weather.hpp"
#include "units/alert.hpp"
#include "units/alarm.hpp"
#include "units/calendar.hpp"
#include "units/ephemeral.hpp"
#include "units/temporary.hpp"
#include "units/deployment.hpp"
#include "units/flag.hpp"
#include "units/testing.hpp"
#include "units/mocking.hpp"
#include "units/tester.hpp"
#include "units/mocker.hpp"
#include "units/antipenetration.hpp"
#include "units/anticorruption.hpp"
#include "units/antivirus.hpp"
#include "units/self.hpp"
#include "units/header.hpp"
#include "units/metadata.hpp"
#include "units/constant.hpp"
#include "units/mutable.hpp"
#include "units/immutable.hpp"
#include "units/classification.hpp"
#include "units/classifier.hpp"
#include "units/saver.hpp"
#include "units/automator.hpp"
#include "units/generator.hpp"
#include "units/png.hpp"
#include "units/svg.hpp"
#include "units/mp4.hpp"
#include "units/wav.hpp"
#include "units/mp3.hpp"
#include "units/ogg.hpp"
#include "units/opus.hpp"
#include "units/flac.hpp"
#include "units/jpeg.hpp"
#include "units/txt.hpp"
#include "units/sh.hpp"
#include "units/js.hpp"
#include "units/java.hpp"
#include "units/gradle.hpp"
#include "units/swift.hpp"
#include "units/objective-c.hpp"
#include "units/objective-cpp.hpp"
#include "units/rust.hpp"
#include "units/dart.hpp"
#include "units/python.hpp"
#include "units/hal.hpp"
#include "units/syntax.hpp"
#include "units/language.hpp"
#include "units/sql.hpp"
#include "units/nosql.hpp"
#include "units/grammar.hpp"
#include "units/vocabulary.hpp"
#include "units/dictionary.hpp"
#include "units/symbol.hpp"
#include "units/alphabet.hpp"
#include "units/letter.hpp"
#include "units/digit.hpp"
#include "units/formatstring.hpp"
#include "units/formatting.hpp"
#include "units/fragmentation.hpp"
#include "units/segmentation.hpp"
#include "units/sequencing.hpp"
#include "units/sequencer.hpp"
#include "units/segmenter.hpp"
#include "units/capturer.hpp"
#include "units/explorer.hpp"
#include "units/discoverer.hpp"
#include "units/listener.hpp"
#include "units/interpreter.hpp"
#include "units/iterator.hpp"
#include "units/representer.hpp"
#include "units/representation.hpp"
#include "units/modeling.hpp"
#include "units/launching.hpp"
#include "units/initiation-2.hpp"
#include "units/ignition.hpp"
#include "units/startup.hpp"
#include "units/shutdown.hpp"
#include "units/bootup.hpp"
#include "units/timeline.hpp"
#include "units/event.hpp"
#include "units/diagnostics.hpp"
#include "units/diagnosis.hpp"
#include "units/voice.hpp"
#include "units/speech.hpp"
#include "units/text.hpp"
#include "units/unary.hpp"
#include "units/trinary.hpp"
#include "units/quaranary.hpp"
#include "units/tool-bit.hpp"
#include "units/tool-byte.hpp"
#include "units/chart.hpp"
#include "units/graph.hpp"
#include "units/plot.hpp"
#include "units/plotgraph.hpp"
#include "units/snapshot.hpp"
#include "units/io.hpp"
#include "units/tool-in.hpp"
#include "units/tool-out.hpp"
#include "units/by.hpp"
#include "units/from.hpp"
#include "units/to.hpp"
#include "units/with.hpp"
#include "units/using.hpp"
#include "units/where.hpp"
#include "units/who.hpp"
#include "units/whois.hpp"
#include "units/uplooking.hpp"
#include "units/rdap.hpp"
#include "units/ping.hpp"
#include "units/request.hpp"

#include "units/response.hpp"

#include "units/dispatcher.hpp"

#include "units/server.hpp"

#include "units/client.hpp"

#include "units/apiserver.hpp"
#include "units/apiclient.hpp"
#include "units/api.hpp"

#include "units/cliserver.hpp"
#include "units/cliclient.hpp"
#include "units/cli.hpp"

#include "units/mcpserver.hpp"
#include "units/mcpclient.hpp"
#include "units/mcp.hpp"

#include "units/sshserver.hpp"
#include "units/sshclient.hpp"
#include "units/ssh.hpp"

#include "units/uri.hpp"

#include "units/url.hpp"

#include "units/configuration.hpp"

#include "units/session.hpp"

#include "units/mesh.hpp"
#include "units/chain.hpp"
#include "units/ring.hpp"
#include "units/matrix.hpp"
#include "units/grid.hpp"
#include "units/path.hpp"
#include "units/route.hpp"
#include "units/lane.hpp"

#include "units/bridge.hpp"

#include "units/rpc.hpp"

#include "units/rdpserver.hpp"
#include "units/rdpclient.hpp"
#include "units/rdp.hpp"

#include "units/vncserver.hpp"
#include "units/vncclient.hpp"
#include "units/vnc.hpp"

#include "units/ast.hpp"
#include "units/ir-2.hpp"
#include "units/cpp.hpp"
#include "units/wasm.hpp"
#include "units/code.hpp"
#include "units/codebase.hpp"
#include "units/sandbox.hpp"
#include "units/sdk.hpp"
#include "units/aes.hpp"
#include "units/gcm.hpp"
namespace protocols {
#include "units/rsa.hpp"
} // namespace protocols
#include "units/sha.hpp"
#include "units/oaep.hpp"
#include "units/pss.hpp"
#include "units/base64.hpp"
#include "units/authorization.hpp"
#include "units/compatibility.hpp"
#include "units/adapter.hpp"
#include "units/tracerobin.hpp"
#include "units/heartbeat.hpp"

#include "units/waverobin.hpp"
#include "units/roundrobin.hpp"
#include "units/heartbeat-2.hpp"
void WAVEROBIN::echo(const std::string& target, const std::string& event) {
    {
        std::lock_guard<std::mutex> lock(guard);
        ++count;
        lastTarget = target;
        lastEvent = event;
    }
    heartbeat.pulse("waverobin");
    heartbeat.pulse(target);
}

#include "units/flow.hpp"
#include "units/lambda.hpp"
#include "units/index.hpp"
#include "units/pool.hpp"
#include "units/farm.hpp"
#include "units/reflex.hpp"
#include "units/reaction.hpp"
#include "units/hybrid.hpp"

int main(int argc, char** argv) {
    if (argc >= 2 && std::string_view(argv[1]) == "--preload") {
        if (argc == 2) {
            std::cout << "Tool passive · no runtime loaded\n";
            return 0;
        }
        // The next argument selects the first stage. Catalogue indexes stay lazy.
        return HYBRID{}.run(argc - 1, argv + 1);
    }
    if (argc == 2 && std::string_view(argv[1]) == "--preflight") {
        std::cout << "Tool passive · runtime and language indexes load on invocation\n";
        return 0;
    }
    return HYBRID{}.run(argc, argv);
}

#endif // TOOL_BROWSER_WASM
