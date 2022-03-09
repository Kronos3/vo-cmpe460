#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from pathlib import Path
from typing import List
from jinja2 import Template

this_directory = Path(__file__).parent
type_sizes = {
    "u8": 1, "s8": 1,
    "u16": 2, "s16": 2,
    "u32": 4, "s32": 4,
    "u64": 8, "s64": 8,
    "f32": 4, "f64": 8,
}
special_types = {
    "string"
}


class Evr:
    evr_id: int
    name: str
    comment: str

    class Arg:
        name: str
        type_: str
        comment: str

        def __init__(self, xml: ET.Element):
            self.name = xml.text.strip()
            self.type_ = xml.attrib.get("type")
            self.comment = xml.attrib.get("comment", "")

            assert self.type_ in type_sizes or self.type_ in special_types, \
                "%s is not a valid type" % self.type_

        def qual_type(self) -> str:
            if self.type_ in type_sizes:
                return self.type_
            elif self.type_ == "string":
                return "const CString&"

    args: List[Arg]

    def __init__(self, xml: ET.Element, evr_id):
        self.evr_id = evr_id
        self.name = xml.attrib["name"]
        self.comment = xml.attrib.get("comment", "")
        self.args = []

        for arg in xml:
            assert arg.tag == "arg", "All children of EVR must be <arg>"
            self.args.append(Evr.Arg(arg))

    def get_size(self) -> str:
        # Add all of the parameters that we can
        n_sum = 0
        all_sum = []
        for arg in self.args:
            if arg.type_ in type_sizes:
                n_sum += type_sizes[arg.type_]
            elif arg.type_ == "string":
                n_sum += 4  # encode string length
                all_sum.append(f"{arg.name}.GetLength()")
            else:
                assert False

        all_sum.append(str(n_sum))
        return " + ".join(all_sum)


def main(args):
    if len(args) < 4:
        print("usage %s: input.xml output.cc output.h" % args[0])
        return 1

    evr_xml = Path(args[1])
    assert evr_xml.exists() and evr_xml.is_file(), "%s does not exist" % evr_xml

    output_cc = Path(args[2])
    output_h = Path(args[3])

    # Parse the input XML
    tree = ET.parse(evr_xml)
    root = tree.getroot()
    evrs = []
    evr_id = 1
    for evr in root:
        evrs.append(Evr(evr, evr_id))
        evr_id += 1

    header_templ = Template((this_directory / "evr_h.templ.txt").open("r").read())
    with output_h.open("w+") as fp:
        fp.write(header_templ.render(evrs=evrs))

    source_templ = Template((this_directory / "evr_cc.templ.txt").open("r").read())
    with output_cc.open("w+") as fp:
        fp.write(source_templ.render(evrs=evrs))

    return 0


if __name__ == "__main__":
    import sys

    exit(main(sys.argv))
