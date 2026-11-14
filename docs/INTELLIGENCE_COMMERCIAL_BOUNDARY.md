# Intelligence retail and supplier accounting

- One customer search carries a EUR 14.00 quote (1400 minor units), independent of supplier Photons.
- Category cards open the corresponding input form inside the workstation. Categories and service definitions come from the verified provider catalog; no desktop binaries or proprietary source are bundled.
- An encrypted local invoice intent is persisted before the retail search is dispatched. A positive tool response makes the claim due and unpaid; a top-level tool failure voids it. Network/cancellation uncertainty requires review and is never automatically marked paid or retried.
- Customer invoice DTOs omit supplier cost. The native owner ledger distinguishes quoted Photons from a provider-confirmed charge; unknown actual charges remain null. An account balance change alone is not a per-operation receipt.
- The local invoice register is not a payment gateway or central customer-accounting system. No Euro wallet deduction, payment confirmation, guaranteed profit, or automatic supplier refund is simulated.
- Before reseller deployment, provider credentials and authoritative charging/invoice state must move to a Nexvary-managed backend. Keeping a supplier key under the customer's own Windows DPAPI profile does not protect the key from that customer or enforce central billing.
- Setup creation remains paused. Windows compilation and local mock/UI checks may run; live paid crawlers still need a specifically authorized target and charge.
