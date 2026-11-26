# Tool ↔ Linguist: reciprocal capability contract

**Contract:** proposed `tool-linguist/1` — specification, not a live connected service.

The standalone Tool project at https://github.com/DanielAspelin/Tool and the standalone Linguist project at https://github.com/DanielAspelin/Linguist have complementary ownership. Tool owns operations, invocation and infrastructure; Linguist owns linguistic processing and linguistic assurance. Either may be caller or provider, depending on the user's actual use. Both must remain usable without the other.

Linguist shall be discovered as an optional capability provider of Tool, and Tool shall be discovered as an optional provider of Linguist. No mutual build dependency, automatic service startup, network listener, hidden callback or implicit delegation is authorized by this specification.

## Reciprocal procedure

1. Discover the counterpart only when requested or configured.
2. Identify the counterpart and negotiate a matching protocol version.
3. Identify the exact operation, input types, capabilities and consent.
4. Apply each side's authorization and independent regulators.
5. Assign bounded correlation, parent and trace identifiers, deadline and maximum hop depth.
6. Send a normalized request through an explicitly permitted adapter.
7. Preprocess and validate at the receiver; refuse unsupported or unauthorized operations.
8. Execute permitted **internal class capabilities** through the receiver's runtime. External binaries require separately authorized execute resolution.
9. Postprocess, acknowledge, verify, provide provenance and return to the caller.
10. Reconcile results, account for failures and maintain independent local fallback.

The runtime within each project remains **Await → Preprocess → Postprocess → Await**; class-level independent integration testimony replaces generic fusion. Request envelopes: `protocol,id,parent_id,origin,target,operation,capability,payload,payload_type,locale,authorization,deadline,max_hops,trace,revision`; responses: `status,result,error,provenance`. Wire encoding, authentication and transports are intentionally unimplemented until cross-repository tests establish interoperability.

## Invariant architectural obligations

Both projects retain independent class functors, nested STACK / HEAP, nested associative architecture/design/model/version/specification arrays, live compatible datatype bindings, individually regulated concurrency/parallelism, bounded fallbacks, and separation of logical subprocess identities from operating system PIDs. A class may use another project's capability only after it has individually testified compatibility, authorization, privacy, integrity and the operational state. Never claim translations, commands, networking or automated cooperation work until verified by tests.

## Planned integration verification

- Validate command/operation schema and version mismatch fallback.
- Exercise Tool → Linguist linguistic request and Linguist → Tool infrastructure request.
- Exercise absent-counterpart and permission-denied behavior without loss of standalone functionality.
- Test loop resistance, repeated deliveries, cancellation, timeouts, sensitive payload redaction and least-privilege invocation.
- Confirm native and WASM builds separately and record implementation/test status in both READMEs.

**Current status:** Documentation only. Not connected, compiled or end-to-end verified.
