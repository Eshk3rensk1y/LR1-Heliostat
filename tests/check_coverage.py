
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build" / "coverage"
MIN_COVERAGE = 80.0


def run(*args):
    print("+", " ".join(map(str, args)), flush=True)
    subprocess.run(
        [str(arg) for arg in args],
        cwd=ROOT,
        check=True,
    )


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)

    for file in OUTPUT.glob("*.gcda"):
        file.unlink()

    for file in OUTPUT.glob("*.gcno"):
        file.unlink()

    executable = OUTPUT / "test_heliostat"
    full_info = OUTPUT / "full.info"
    app_info = OUTPUT / "app.info"
    html_dir = OUTPUT / "html"

    run(
        "gcc",
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-Werror",
        "--coverage",
        "-O0",
        "-IApp/Inc",
        "App/Src/heliostat_logic.c",
        "tests/test_heliostat_logic.c",
        "-o",
        executable,
    )

    run(executable)

    run(
        "lcov",
        "--capture",
        "--directory",
        OUTPUT,
        "--base-directory",
        ROOT,
        "--output-file",
        full_info,
    )

    run(
        "lcov",
        "--extract",
        full_info,
        "*/App/Src/heliostat_logic.c",
        "--output-file",
        app_info,
    )

    run(
        "genhtml",
        app_info,
        "--output-directory",
        html_dir,
    )

    found = 0
    hit = 0

    for line in app_info.read_text(
        encoding="utf-8"
    ).splitlines():
        if line.startswith("LF:"):
            found += int(line[3:])
        elif line.startswith("LH:"):
            hit += int(line[3:])

    if found == 0:
        print("ERROR: No coverage data found")
        return 1

    coverage = hit * 100.0 / found

    print()
    print("=== HELIOSTAT COVERAGE REPORT ===")
    print(f"Covered lines: {hit}/{found}")
    print(f"Line coverage: {coverage:.2f}%")
    print(f"Minimum required: {MIN_COVERAGE:.2f}%")
    print(f"HTML report: {html_dir / 'index.html'}")

    if coverage < MIN_COVERAGE:
        print("COVERAGE CHECK FAILED")
        return 1

    print("COVERAGE CHECK PASSED")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except subprocess.CalledProcessError as error:
        print(f"Command failed: {error}", file=sys.stderr)
        sys.exit(1)
