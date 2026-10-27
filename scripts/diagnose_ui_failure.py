#!/usr/bin/env python3
"""One diagnostic replay of failed synthetic Qt tests; never clears CTest failure."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess


def diagnose(build, ctest="ctest", gdb="gdb"):
    build = Path(build).resolve()
    failure_list = build / "Testing/Temporary/LastTestsFailed.log"
    if not failure_list.is_file():
        return []
    failed = {line.partition(":")[2].strip()
              for line in failure_list.read_text().splitlines()}
    selected = failed & {"mr_ui_tests", "mr_ui_scale_1.25", "mr_ui_scale_1.5"}
    if not selected:
        return []
    listing = subprocess.run([ctest, "--test-dir", str(build), "--show-only=json-v1"],
                             check=True, capture_output=True, text=True)
    results = []
    for test in json.loads(listing.stdout)["tests"]:
        if test["name"] not in selected:
            continue
        properties = {p["name"]: p["value"] for p in test.get("properties", [])}
        env = os.environ.copy()
        for entry in properties.get("ENVIRONMENT", []):
            key, sep, value = entry.partition("=")
            if sep:
                env[key] = value
        # No bt full / local variables / memory or core dump: these tests use
        # synthetic fixtures, but diagnostics must still minimize data exposure.
        # QuickTest's original -o files are evidence. Replay only to stdout.
        arguments = iter(test["command"])
        replay_command = []
        for argument in arguments:
            if argument == "-o":
                next(arguments, None)
            else:
                replay_command.append(argument)
        replay_command += ["-o", "-,txt"]
        command = [gdb, "-nx", "--batch", "--return-child-result",
                   "-ex", "set pagination off", "-ex", "set debuginfod enabled off",
                   "-ex", "run", "-ex", "thread apply all bt", "--args",
                   *replay_command]
        log = build / (test["name"] + "-gdb.txt")
        with log.open("w") as output:
            try:
                replay = subprocess.run(command, cwd=properties.get("WORKING_DIRECTORY", str(build)),
                                        env=env, stdout=output, stderr=subprocess.STDOUT,
                                        timeout=180, check=False)
                code = replay.returncode
            except subprocess.TimeoutExpired:
                output.write("\nDiagnostic replay exceeded 180 seconds.\n")
                code = "timeout"
            except OSError as error:
                output.write("Diagnostic tool unavailable: " + type(error).__name__ + "\n")
                code = "unavailable"
        results.append({"test": test["name"], "diagnosticExit": code,
                        "log": log.name, "clearsOriginalFailure": False})
    (build / "ui-crash-diagnostics.json").write_text(json.dumps(results, indent=2) + "\n")
    return results


def run_suite(build, ctest="ctest", gdb="gdb"):
    build = Path(build).resolve()
    result = subprocess.run([ctest, "--test-dir", str(build), "--output-on-failure",
                             "--output-junit", "linux-qt-results.xml"], check=False)
    original = build / "Testing/Temporary/LastTest.log"
    if original.is_file():
        shutil.copyfile(original, build / "full-suite-last-test.log")
    failures = build / "Testing/Temporary/LastTestsFailed.log"
    if result.returncode and failures.is_file():
        shutil.copyfile(failures, build / "full-suite-failed-tests.log")
        try:
            diagnose(build, ctest, gdb)
        except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
            (build / "ui-crash-diagnostic-error.txt").write_text(type(error).__name__ + "\n")
    return result.returncode


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, required=True)
    args = parser.parse_args()
    raise SystemExit(run_suite(args.build))
