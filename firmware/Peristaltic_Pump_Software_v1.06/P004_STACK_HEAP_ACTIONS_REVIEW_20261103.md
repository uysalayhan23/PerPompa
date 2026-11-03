# PerPompa P0-04 — AVR stack / heap evidence (2026-11-03)


> **ÖLÇÜM DÜZELTMESİ — 2026-11-03 (sonraki doğrulama):** Önceki 95/205 B ham SP örneklemesi iki baytlı AVR SPH/SPL güncellemesi arasındaki geçici değeri içerebilir. Flash-LCD adayı için 205 B'nin bu nedenle hatalı olduğu makine koduyla doğrulandı. Flash-LCD aynı SHA için tamamlanmış SP açıklığı 16 MHz 383 B, 8 MHz 377 B olarak **yalnız sınırlandırılmış simAVR izinde** ölçüldü. Eski 95 B baseline tamamlanmış-SP ile ayrıca yeniden ölçülmedi. Kaynak ve kanıt: [P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md](P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md). Önceki ham sonuçlar tarihsel kanıt olarak korunur; `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED` değişmez.

**Software assessment: CONDITIONAL/HOLD. This report is not a physical peak stack acceptance.**
`STACK_SIM_TRACE_PASS` · `TRUE_ALL_PATHS_STACK_PEAK=NOT_RUN` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Exact source and provenance

- Production firmware on main was **not changed**: `firmware/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino` Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49`.
- Separate, temporary P0-04 MergedGates/recovery source (NOT for production) has .ino SHA256 `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21`, matching historical QA archive SHA256 `b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2`.
- Exact SHA verification on GitHub-hosted `ubuntu-24.04`. SHA-locked source, Arduino AVR Boards 1.8.8 and Arduino CLI; GitHub Actions issues-triggered owner-only; checkout without persisted credentials; `contents: read`; at most one job; 15-minute timeout; no AVR uploader, release or deploy action.
- Successful source-identical AVR stack profiling: [run 41640904072](https://github.com/uysalayhan23/PerPompa/actions/runs/41640904072). Extended 70,000,000-step run: [run 41642141415](https://github.com/uysalayhan23/PerPompa/actions/runs/41642141415). Artifacts are retained briefly by GitHub; important measurements reproduced below.
- Three other runs [41640214934](https://github.com/uysalayhan23/PerPompa/actions/runs/41640214934), [41640539073](https://github.com/uysalayhan23/PerPompa/actions/runs/41640539073) and [41641675274](https://github.com/uysalayhan23/PerPompa/actions/runs/41641675274) did not satisfy the stack gate because of a tool PATH mistake, unavailable GCC LTO `.su` output, and Arduino library mirror timeout respectively. They were not counted as PASS. A separate non-LTO diagnostic build emitted 368 frame records per target; these cannot be substituted for the optimized LTO firmware's whole-program stack bound.

## Measured on identical optimized AVR firmware binaries

| Metric | Pro Mini 16 MHz | Pro Mini 8 MHz |
|---|---:|---:|
| Real AVR compiler | PASS | PASS |
| Flash bytes | 27,932 | 27,928 |
| Global/static SRAM bytes | 1,436 | 1,436 |
| Static-unallocated SRAM bytes | 612 | 612 |
| Simulated CPU instruction steps | 70,000,000 | 70,000,000 |
| Lowest observed SP | 0x0713 | 0x0713 |
| Observed stack depth from RAMEND 0x08FF | 492 B | 492 B |
| BSS end | 0x069C | 0x069C |
| Maximum observed malloc heap break | 0x06B4 | 0x06B4 |
| Observed heap extension beyond BSS end | 24 B | 24 B |
| **Smallest observed free stack–heap gap** | **95 B** | **95 B** |
| Simulated I2C LCD initialized | YES | YES |
| Input stimulus stages completed | 6/6 | 6/6 |
| HW Setup active seen in this run | YES | NO |

The simulated peripherals include a simplified PCF8574 LCD model, encoder button and Remote transitions, with a test-only direct RAM assignment of a menu selection to reach relevant code. This is **not** an exhaustive or physical reproduction of LCD, EEPROM writes, UART traffic, ISR timing, pump motion or every menu/callback path. The 95-byte number is the **smallest observed gap in these bounded traces**, not a proven minimum over all possible paths. AVR-libc heap growth and interrupts under different conditions could narrow this gap.

`-fstack-usage` with the production LTO flags emitted no usable `.su` frames. A **separate diagnostic** non-LTO compile produced 368 entries per target; largest individual static local frame: 101 B in `perpompa_p004::Store::save`; 76 B dynamic-bounded in `format_menu_line`. Summing these entries is not a valid optimized AVR/ISR worst-case proof.

## Safety and delivery decision

- **STACK_HEAP_SOFTWARE_EVIDENCE = MEASURED_BOUNDED_TRACE_PASS**; **ALL_PATHS_RUNTIME_PEAK = NOT_RUN/REVIEW**. The 95-byte observed margin is narrow and cannot be accepted as industrial firmware stack safety. No universal minimum or maximum safety margin is claimed.
- Before firmware promotion, require a demonstrably safe bound or additional memory reduction, and hardware-informed stack/heap high-water validation across relevant operating and interrupt combinations. The Actions checks do not substitute for this.
- `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`, P0-04 17/48 conservative recovery, and `31/48 SERVICE_HOLD` remain unchanged.
- **NO firmware patch / commit / push**. GitHub clean-up commit removes the temporary workflow, read-only archive and emulator harness from active main after evidence capture; historical commits preserve forensic reproducibility. v1.04 archive, v1.05 manifest and P0-04 automation are untouched.
