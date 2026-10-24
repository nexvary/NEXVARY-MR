# Published workflow review for the UI rebuild

Reviewed 2026-10-23. This is a design-input review of public vendor material,
not a licensed-product benchmark or a claim of feature/performance superiority.
No competitor artwork, interface assets or proprietary implementation is copied.
Where only documentation/indexed material was available, pixel layout and current
binary behavior remain unverified.

| Product / primary source | Observable workflow evidence | NEXVARY design decision | Boundary |
| --- | --- | --- | --- |
| [Chimera: integrated USB log](https://news.chimeratool.com/2025/12/19/streamlining-device-management-exploring-chimera-tools-integrated-usb-log-view/) and [help center](https://chimeratool.com/en/docs) | Vendor describes persistent connected-device/USB visibility beside procedures and connection events. Help groups model, connection and procedure references. | Keep selected target, connection and running operation in the shell; make device discovery and identity checks reachable from the working page. | Vendor-described behavior; no timing or multi-device reliability benchmark. |
| [Hydra official documentation](https://docs.hydradongle.com/) and [Allwinner module](https://docs.hydradongle.com/supported-devices/allwinner/module/) | Documentation routes chipset → brand → module. It distinguishes available functions from work still under testing. | Group related native services; show explicit implemented, Laboratory-only, missing-tool and unsupported states. Prefer capability evidence over a generic green “supported” badge. | Documentation taxonomy informs organization; not proof that NEXVARY implements Hydra's low-level capabilities. |
| [UnlockTool developer site](https://unlocktool.net/) | The developer page returned HTTP 403 to the research surface. Search results included multiple lookalike/reseller domains; these were not accepted as authoritative UI documentation. | Keep a searchable target/service catalog and connection-mode context as independently chosen product requirements; do not claim these decisions came from a verified current UnlockTool interface. | Current official UI inspection is **blocked**, not completed. No licensed binary was installed. |
| [Octoplus Pro software manual](https://octoplusbox.com/en/info/octoplus-pro-software-user-manual/) | The manual explicitly separates interface selection/configuration, device information/log, operation percentage, speed, elapsed/remaining time and connection state. It describes initialization before selecting a recovery method. | Distinct target summary, grouped actions, results and operation progress. Show only measured percentages/throughput; use indeterminate state when no reliable progress exists. | Published manual includes hardware-specific operations outside NEXVARY's accepted scope. No latency or recovery equivalence inferred. |
| [Cellebrite Physical Analyzer](https://cellebrite.com/en/products/cellebrite-inseyets/physical-analyzer/) | Indexed vendor material describes case dashboard widgets and a visual event timeline. Direct page retrieval returned 403. | Case summary and evidence/results inspection belong together; clearly distinguish collection/audit events from authenticated artifact events. | Indexed vendor description only; direct current page/binary inspection incomplete. No protected-device extraction copied. |
| [Oxygen: data search](https://www.oxygenforensics.com/technical-resources/data-search-feature/) | Indexed vendor article identifies one/multiple-extraction search, criteria tabs, result filters and result export. Full-page retrieval was restricted. | Keep evidence search, type filters, selected-result details and consented export in one workspace; make source provenance visible. | Search-indexed vendor material; current full interface and extraction performance not verified. A reseller image explicitly named “Generated Image” was rejected as evidence. |
| [Magnet AXIOM getting started](https://www.magnetforensics.com/resources/getting-started-magnet-axiom/) and [AXIOM Cyber user guide](https://www.magnetforensics.com/docs/axiom-cyber/html/Content/Resources/PDFs/Magnet%20AXIOM%20Cyber%20User%20Guide.pdf) | Official learning sequence distinguishes process settings/evidence-source setup, Examine explorers/menus, search/filter/tag review, and case reporting. The guide describes progress and post-processing exceptions. | Separate preparing a job, running it, inspecting results and reporting. Keep errors/exceptions adjacent to results rather than hiding them behind a generic success message. | Published workflow reference only; no claim about licensed product accuracy or UI task times. |

## Decisions used in the rebuild

1. A stable shell shows target and operation context; selecting another workspace
   does not erase that context or silently select another device.
2. One coherent card represents one major job: attributed color icon, title,
   short purpose, availability reason, primary action and related secondary actions.
3. Use responsive grids for operations and reserve meaningful detail/results
   space. Empty states provide the next valid step, not decorative filler.
4. Make Review → Execute → Verify distinct. A host process starting or a command
   acknowledgment must not be displayed as verified physical success.
5. Place raw details behind clear disclosure/scrolling while preserving important
   target, consent, progress and failure information at normal reading size.
6. Implement the same hierarchy in Arabic RTL and English LTR. Actual SVG assets
   are local resources; no font emoji are used as operation icons.

## Fair future comparison protocol

Use separately licensed vendor tools and the same authorized phone/build/cable/
driver/host, with recorded tool versions and identical read-only tasks. Measure:
time to stable discovery (p50/p95), wrong-target prevention, clicks to properties/
diagnostics/report, visible failure reasons, result correctness against known
fixtures, and AR/EN layout/accessibility defects. Separate cold/warm starts and
discovery from identity collection. Report failures and unsupported cases as well
as successes. No such comparative run was performed in this increment.

NEXVARY's immediately testable targets are: every navigation destination is
reachable, every unavailable action has an explanation, no operation starts
without its required target/consent, and controls remain usable across the stated
window sizes. These are engineering acceptance targets, not competitor rankings.
