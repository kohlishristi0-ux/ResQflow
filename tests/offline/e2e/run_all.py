#!/usr/bin/env python3
"""Runs every offline end-to-end suite and prints one overall result (exit code 1 on any failure)."""
import glob, os, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
total_pass = total_fail = 0
for suite in sorted(glob.glob(os.path.join(here, 'test_*.py'))):
    result = subprocess.run([sys.executable, suite], capture_output=True, text=True)
    sys.stdout.write(result.stdout)
    for line in result.stdout.splitlines():
        if line.startswith('TOTAL passed'):
            parts = line.split()
            total_pass += int(parts[2]); total_fail += int(parts[4])
    if result.returncode != 0 or 'TOTAL passed' not in result.stdout:
        total_fail += 1
        sys.stdout.write(result.stderr)
print("\n==== OVERALL: %d checks passed, %d failed ====" % (total_pass, total_fail))
sys.exit(1 if total_fail else 0)
