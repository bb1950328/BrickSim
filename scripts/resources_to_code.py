#!/usr/bin/env python3
import re
from pathlib import Path

indent_level = 0

BLACKLIST = [
    ".*drawio",
    "logo_fit\\.png",
    "logo_fit_314x163",
    "logo_fit_highres",
    "logo_icon_uninstall",
    "logo_square",
]


def indent():
    return 4 * indent_level * " "


def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def dump_file(path):
    with open(str(path), "rb") as file:
        data = file.read()
        name = path.name
        var_name = to_valid_identifier(name)
        declaration = f"extern const std::array<uint8_t, {len(data)}> {var_name}"
        h_file.write(indent() + f"{declaration};\n")
        cpp_file.write(indent() + declaration + " = {\n")
        global indent_level
        indent_level += 1
        for ch in chunks(data, 2 ** 16):
            cpp_file.write(indent() + ",".join(map(str, ch)) + ",\n")
        indent_level -= 1
        cpp_file.write(indent() + "};\n")


def to_valid_identifier(name):
    return re.sub(r'\W+|^(?=\d)', '_', name)


def walk_directory(path: Path):
    for child in sorted(path.iterdir()):
        if any([re.search(item, str(child)) for item in BLACKLIST]):
            continue
        if child.is_dir():
            namespace_decl = f"{indent()}namespace {to_valid_identifier(child.name)} {{\n"
            h_file.write(namespace_decl)
            cpp_file.write(namespace_decl)
            global indent_level
            indent_level += 1
            walk_directory(child)
            indent_level -= 1
            h_file.write(indent() + "}\n")
            cpp_file.write(indent() + "}\n")
        else:
            dump_file(child)


if __name__ == '__main__':
    res_dir = Path("./resources")
    if not res_dir.exists():
        raise FileNotFoundError("please change the working directory to the parent of the resources directory")

    with open("./src/constant_data/resources.h", "w") as h_file, \
            open("./src/constant_data/resources.cpp", "w") as cpp_file:
        comment = "//this file is automatically generated by scripts/resources_to_code.py, editing it is a bad idea\n"
        h_file.write(comment)
        cpp_file.write(comment)
        cpp_file.write("#include \"resources.h\"\n")
        h_file.write("#include <array>\n#include <cstdint>\n")
        h_file.write("namespace bricksim::resources {\n")
        cpp_file.write("namespace bricksim::resources {\n")
        indent_level += 1

        walk_directory(res_dir)

        indent_level -= 1

        cpp_file.write("}\n")
        h_file.write("}\n")
