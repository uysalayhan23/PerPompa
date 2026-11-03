#!/usr/bin/env bash
# QA-only full-source native tests. No flashing, no GitHub Actions, no device access.
set -euo pipefail
repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$repo"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  qa/p004/read_only_candidate.tar.gz' | sha256sum -c -
echo '2326f2bff168a6a165f93bafcb393ab91d333d38fb8dff7d2affaf36ec2cd539  qa/p004/flash_fullsource_native_qa.tar.gz' | sha256sum -c -
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/sketch"
tar -xzf qa/p004/read_only_candidate.tar.gz -C "$work/sketch" --strip-components=1
tar -xzf qa/p004/flash_fullsource_native_qa.tar.gz -C "$work"
python3 qa/p004/flash_lcd_literals_candidate.py "$work/sketch/Peristaltic_Pump_Software_v1.06.ino"
python3 "$work/run_new_fullsource_native.py"
ASAN_OPTIONS=detect_leaks=0 python3 "$work/run_new_fullsource_asan.py"
echo 'FULL_SOURCE_HOST_NEW=48/48x2;NEGATIVE_MUTANTS=20/20x2;REVIEW_ONLY;RELEASE_BLOCKED'
