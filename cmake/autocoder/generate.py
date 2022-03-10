#!/usr/bin/env python3

from pathlib import Path
import jinja2
from jinja2 import Template

file_dir = Path(__file__).parent.absolute()
AUTOCODERS = [
    "ai-xml",
    "fpp"
]


def get_autocoder_template() -> Path:
    return file_dir / "autocoder-template.cmake"


def generate_autocoder(name: str):
    check = file_dir / (name + "-proto.cmake")
    assert check.exists(), f"Prototype file for {name} does not exist: {check}"
    assert check.is_file(), f"Prototype file for {name} is not a file: {check}"

    variables = {
        "AUTOCODER_NAME": name.replace("-", "_"),
        "AUTOCODER_FILENAME": f"autocoder/{name}-proto",
    }

    with get_autocoder_template().open() as f:
        try:
            template = Template(f.read())
        except jinja2.exceptions.TemplateSyntaxError as e:
            raise SyntaxError("Failed to parse Jinja2 template %s: %s" % (get_autocoder_template(), e)) from e

    out_path = file_dir / f"{name}.cmake"
    print("Generating %s" % out_path)
    with out_path.open("w+") as f:
        f.write(template.render(variables))


def main():
    for ac in AUTOCODERS:
        generate_autocoder(ac)

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
