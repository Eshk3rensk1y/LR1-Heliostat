
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent

RULES = {
    "App": {
        "forbidden_includes": [
            "main.h",
            "adc.h",
            "tim.h",
            "i2c.h",
            "gpio.h",
            "usart.h",
            "platform_io.h",
            "heliostat_devices.h",
        ],
        "forbidden_patterns": [
            r"\bHAL_[A-Za-z0-9_]+\b",
            r"\b__HAL_[A-Za-z0-9_]+\b",
            r"\bMX_[A-Za-z0-9_]+\b",
        ],
    },
    "Devices": {
        "forbidden_includes": [
            "main.h",
            "adc.h",
            "tim.h",
            "i2c.h",
            "gpio.h",
            "usart.h",
        ],
        "forbidden_patterns": [
            r"\bHAL_[A-Za-z0-9_]+\b",
            r"\b__HAL_[A-Za-z0-9_]+\b",
            r"\bMX_[A-Za-z0-9_]+\b",
        ],
    },
}

errors = []

for layer, rules in RULES.items():
    directory = ROOT / layer

    if not directory.is_dir():
        errors.append(f"Missing directory: {layer}")
        continue

    files = list(directory.rglob("*.c"))
    files += list(directory.rglob("*.h"))

    if not files:
        errors.append(f"No source files in {layer}")

    for path in files:
        content = path.read_text(encoding="utf-8")

        includes = re.findall(
            r'^\s*#\s*include\s*[<"]([^>"]+)[>"]',
            content,
            re.MULTILINE,
        )

        for include in includes:
            name = Path(include).name

            if (
                name in rules["forbidden_includes"]
                or name.startswith("stm32")
            ):
                errors.append(
                    f"{path.relative_to(ROOT)}: "
                    f"forbidden include {include}"
                )

        for pattern in rules["forbidden_patterns"]:
            if re.search(pattern, content):
                errors.append(
                    f"{path.relative_to(ROOT)}: "
                    f"forbidden HAL/CubeMX dependency"
                )

if errors:
    print("ARCHITECTURE CHECK FAILED")

    for error in errors:
        print(f"ERROR: {error}")

    sys.exit(1)

print("ARCHITECTURE CHECK PASSED")
print("App: no forbidden hardware dependencies")
print("Devices: no direct HAL dependencies")
