# P0-04 — GitHub Actions AVR compile QA (2026-11-03)

**Disposition: SOFTWARE_REVIEW; PHYSICAL_NOT_RUN; STACK_PEAK_NOT_RUN; RELEASE_BLOCKED.**

This is a build-only review of a temporary v1.06 EEPROM recovery / MergedGates candidate. It does **not** promote the candidate to production firmware.

- Original production main before QA: `a415f1f66761872e2d9297f6bb33f607fceefe14`.
- Review-only candidate archive: `qa/p004/read_only_candidate.tar.gz` in historical QA commit `aca1af43ef69f098267c60c3aa6b9360f9a40ad7`; archive SHA256 `b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2`.
- Candidate `.ino` SHA256 `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21`. Workflow verified the archive and exact source SHA before compilation.
- GitHub-hosted Actions run: https://github.com/uysalayhan23/PerPompa/actions/runs/41637720429
- Workflow event: owner-authored one-shot issue; GitHub-hosted `ubuntu-24.04`; `contents: read` only; one simultaneous job, 15 minute timeout, no upload-to-device or repo write.
- Actual Arduino CLI / AVR Boards 1.8.8 compile: **2/2 PASS**.
  - `arduino:avr:pro:cpu=16MHzatmega328`: Flash **27932 / 30720 B**; global/static SRAM **1436 / 2048 B**; unallocated static SRAM **612 B**.
  - `arduino:avr:pro:cpu=8MHzatmega328`: Flash **27928 / 30720 B**; global/static SRAM **1436 / 2048 B**; unallocated static SRAM **612 B**.
- The compiler's "612 bytes remaining" excludes true peak stack/heap. **STACK_PEAK_NOT_RUN**. EEPROM real power cuts, motor/RNG physical observation, LCD/encoder hardware and release compliance remain **PHYSICAL_NOT_RUN / RELEASE_BLOCKED**.
- Earlier HOST_CPP native evidence for this identical SHA is separately recorded as `106/106 × 4`, second-save power-cut `128/128` and generation wrap `128/128`. These are historical host results, **not** AVR or physical tests and were not re-run in this Actions job.
- No production `.ino`, headers, v1.05 policy, v1.04 archive, automation or protected files changed. The ephemeral issue-triggered workflow and QA archive are removed from current main after the run to avoid leaving an externally triggerable workflow; Git commit history retains the reproducible QA input.

**Firmware promotion: HOLD.** No release, no implicit separate firmware acceptance. The prior RAM-only production v1.06 blob `957f963c1d4f59a04f18a896e9f6c92890061a49` remains unchanged.
