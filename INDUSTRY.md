# Tool Industry Systems Specification

Revision 1.0 · Reference review 28 November 2026

## Shared control contract

| Control | Responsibility |
|---|---|
| Transmission Control | Send bounded-duration HTTPS requests. Let the platform negotiate and verify TLS. Reject redirect following and reject requests that leave the registered origin. |
| Connection Control | Register provider origins and select the route by provider identity. Configure provider endpoints and secrets at the server. |
| Communication Control | Dispatch typed `profile`, `operation`, and `input` requests. Expose the available operations as `tool.industry.v1`. |

The server entry point is `industry/system.mjs`. Mount `IndustryEndpoint.handle(request)` from `industry/endpoint.mjs` on an authenticated HTTPS route. It serves capabilities via GET and operations via same-origin JSON POST. The host supplies authentication and operation authorization. The authorization handler resolves authoritative order terms, device, message and subscriber references from the caller's identity. Return null to deny. Use the application's permissions and request-forgery controls. Keep provider configurations and secrets in the server environment.

Serve the built `industry/client.mjs`, `industry/industry.R`, and `industry/telecom.mjs` together. Initialize webR before calling `IndustryClient.create({ webR, endpoint, csrfToken })`. Use `capabilities()` to determine available controls and `invoke(profile, operation, input)` for a user action. The R runtime receives operation and outcome counts. The browser can validate IMEI and estimate SMS segments locally via `TelecomData`.

## Payment systems

| Profiles | Adapter and supported operations | Maintained reference |
|---|---|---|
| Visa, Mastercard | Stripe hosted Checkout Session create and status. Network selection and availability follow the merchant's processor configuration. | [Stripe Checkout](https://docs.stripe.com/payments/checkout) · [EMVCo tokenisation](https://www.emvco.com/emv-technologies/payment-tokenisation/) |
| Google Pay, Apple Pay | Same hosted card checkout. Advertise each wallet only after merchant enablement is confirmed and the wallet is added to the server's `stripe.wallets` list. The provider decides buyer eligibility by region, device and browser. | [Google Pay requests](https://developers.google.com/pay/api/web/reference/request-objects) · [Apple Pay on the Web](https://developer.apple.com/documentation/applepayontheweb) |
| Vipps MobilePay | ePayment v1 wallet redirect, status and explicit capture. | [Create payment](https://developer.vippsmobilepay.com/docs/APIs/epayment-api/api-guide/operations/create/) · [ePayment API](https://developer.vippsmobilepay.com/api/epayment/) |
| UPI | Razorpay UPI Payment Link, INR only. Read provider status using the link reference. Actual availability follows the merchant's approved UPI route and customer environment. | [NPCI UPI](https://www.npci.org.in/what-we-do/upi/product-overview) · [Razorpay UPI](https://razorpay.com/docs/payments/payment-methods/upi/) |
| PayPal | Orders v2 approval, status and explicit capture. An approved order awaits capture. | [Orders v2](https://developer.paypal.com/api/orders/v2/) |
| Bitcoin | BTCPay Greenfield v1 hosted on-chain invoice and status. A settled invoice must match the expected order total and currency. | [Greenfield v1](https://docs.btcpayserver.org/API/Greenfield/v1/) · [Invoice states](https://docs.btcpayserver.org/Users/invoices/) · [Bitcoin payment URI BIP 321](https://bips.dev/321/) |
| Shopify | Storefront `cartCreate` returns Shopify's checkout URL. Shopify owns payment UX and order settlement. | [Storefront cartCreate](https://shopify.dev/docs/api/storefront/latest/mutations/cartCreate) · [Shopify API 2026-10](https://shopify.dev/changelog/release-notes/2026-10) · [Payments extensions](https://shopify.dev/docs/apps/build/payments) |

Stripe and Shopify versions are pinned in deployment configuration. Shopify 2026-10 is the verified current stable Storefront release at the review date. Provider adapters use configured merchant credentials and exact API and checkout origins. Hosted card checkout keeps card numbers and security codes on the provider surface. PCI DSS applicability remains a merchant assessment. [PCI SSC Document Library](https://www.pcisecuritystandards.org/document_library/)

### Payment operation contract

- Authorize `create` using a server-owned `orderKey`, positive integer `amountMinor`, and currency. Shopify resolves server-approved variant IDs and quantities. Store the returned reference against the order and return the hosted checkout URL to the customer.
- Recheck order ownership before `status` or `capture`. Capture receives a separate stable `requestKey` where supported and uses trusted order terms. Reconcile successful provider evidence with amount and currency. Authorize fulfillment through the application's durable order workflow. Handle refunds, disputes, duplicate requests, and provider webhook verification there. Shopify requires its own order evidence.
- A redirect or browser return cannot establish settlement. After transmission failure or an uncertain outcome, reconcile using the order and provider records before retrying. Provider mutations run once per authorized invocation. BTCPay invoices, Shopify carts, and SMS delivery require host-side deduplication.

## Telecommunications

| Profile | Implementation | Maintained reference |
|---|---|---|
| GSM and mobile access | Optional Linux ModemManager adapter reads the configured modem's registration state and access technology. Reports the technology the modem actually returns, including LTE/5G where present. | [ModemManager](https://modemmanager.org/docs/modemmanager/) · [3GPP specifications](https://www.3gpp.org/specifications-technologies/specifications-by-series) |
| IMEI | Local 15-digit checksum validation in C++ and JavaScript. The checksum only establishes syntax and consistency. | [3GPP TS 23.003 / ETSI TS 123 003, Annex B](https://www.etsi.org/deliver/etsi_ts/123000_123099/123003/19.05.00_60/ts_123003v190500p.pdf) |
| SMS | GSM-7 default and extension table planning and Unicode UTF-16 segment estimates. Optional Twilio Messaging Service route supports authorized sends and delivery-status reads. | [3GPP TS 23.038 / ETSI TS 123 038](https://www.etsi.org/deliver/etsi_ts/123000_123099/123038/18.00.00_60/ts_123038v180000p.pdf) · [3GPP TS 23.040](https://www.3gpp.org/dynareport/23040.htm) · [Twilio Messages](https://www.twilio.com/docs/messaging/api/message-resource) |
| Operator Number Verification | Configured CAMARA route. The application supplies a subscriber-bound access token obtained through the operator's enrollment and authorization flow. | [CAMARA Number Verification](https://github.com/camaraproject/NumberVerification) · [GSMA Open Gateway](https://www.gsma.com/solutions-and-impact/gsma-open-gateway/) |

SMS planning assumes default concatenation headers. Carrier encoding and segment limits can differ. The application resolves consent, delivery policy and quotas before it authorizes sending. SIM Swap, Device Status, Quality on Demand, eSIM provisioning, IMS, and voice are reference domains for separate operator adapters.

## Internet industry profiles

| Layer | Reference and Tool boundary |
|---|---|
| IP, DNS and transport | Host stack handles [IPv6 RFC 8200](https://www.rfc-editor.org/info/rfc8200), [DNS RFC 1035](https://www.rfc-editor.org/info/rfc1035), and [TCP RFC 9293](https://www.rfc-editor.org/info/rfc9293). [QUIC RFC 9000](https://www.rfc-editor.org/info/rfc9000) is a platform transport option. |
| Security and application exchange | Transmission uses HTTPS and relies on platform TLS ([TLS 1.3 RFC 8446](https://www.rfc-editor.org/info/rfc8446), [HTTP Semantics RFC 9110](https://www.rfc-editor.org/info/rfc9110)). Configured Internet status reads return one of `ready`, `degraded`, and `unavailable` from a registered JSON status source. |
| Network operations and routing | [DHCP RFC 2131](https://www.rfc-editor.org/info/rfc2131), [BGP-4 RFC 4271](https://www.rfc-editor.org/info/rfc4271), [DNS over HTTPS RFC 8484](https://www.rfc-editor.org/info/rfc8484), and [IPv4 RFC 791](https://www.rfc-editor.org/info/rfc791) are reference profiles. Operational routing and addressing belong to the configured network infrastructure. |
| Interactive streams and delivery | [WebSocket RFC 6455](https://www.rfc-editor.org/info/rfc6455), [SIP RFC 3261](https://www.rfc-editor.org/info/rfc3261), [RTP RFC 3550](https://www.rfc-editor.org/info/rfc3550), and [SMTP RFC 5321](https://www.rfc-editor.org/info/rfc5321) are reference profiles for future dedicated adapters. |

## Verification scope

The fixture suite covers provider request construction, outbound origin enforcement, authorization before transmission, checkout URL validation, payment reconciliation, telecom operations, and the browser/endpoint contract. With `TOOL_TEST_WEBR` set to the path of a WebR module, it exercises the R audit using the actual runtime. Merchant sandbox transactions, carrier delivery, and physical modem operation require scoped credentials and test accounts or equipment.
