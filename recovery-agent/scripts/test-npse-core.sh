#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
output=$(mktemp -d)
trap 'rm -rf "$output"' EXIT
javac -Xlint:all -Werror -d "$output" npse-core/src/main/java/com/nexvary/npse/core/*.java npse-core/src/test/java/com/nexvary/npse/core/EngineTest.java
java -cp "$output" com.nexvary.npse.core.EngineTest
