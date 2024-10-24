# SPDX-FileCopyrightText: © 2024 ZeldaRET
# SPDX-License-Identifier: CC0-1.0
#
#
#

from xml.etree import ElementTree
from xml.etree.ElementTree import Element

from .util import error

class ExtractionDescription:

    def __init__(self, file_path : str, file_name : str, version_name : str) -> None:
        self.type_name = type(self).__name__.replace("ExtractionDescription", "")
        self.file_name = file_name.replace(".xml", "")
        self.file_path = file_path

        xml_root = ElementTree.parse(file_path).getroot()
        if xml_root.tag != self.type_name or "Name" not in xml_root.attrib or "Index" not in xml_root.attrib:
            error(f"Malformed {self.type_name} extraction xml: \"{file_path}\"")

        self.name = xml_root.attrib["Name"]
        self.index = int(xml_root.attrib["Index"])

        self.post_init(xml_root, version_name)

    def post_init(self, xml_root : Element, version_name : str):
        raise NotImplementedError() # Implement in subclass

class SampleBankExtractionDescription(ExtractionDescription):

    def post_init(self, xml_root : Element, version_name : str):
        self.included_version = None
        self.all_version_contents = {}
        self.sample_info = {}
        self.blob_info = {}

        was_included = False

        for item in xml_root:
            assert item.tag == "Version"

            # Version can either specify what versions are allowed or what versions are disallowed
            version_include = item.attrib.get("Include", "")
            version_exclude = item.attrib.get("Exclude", "")
            version_ident = (version_include, version_exclude)

            # Save all version trees incase we're writing them back out
            version_contents = [(item2.tag, item2.attrib) for item2 in item]
            assert (version_include, version_exclude) not in self.all_version_contents
            self.all_version_contents[version_ident] = version_contents

            if version_include == "":
                version_include = "All"
            if version_exclude == "":
                version_exclude = "None"

            # Determine if this layout is the one we need
            if version_include != "All":
                version_include = version_include.split(",")
            if version_exclude != "None":
                version_exclude = version_exclude.split(",")

            included = version_include == "All" or version_name in version_include
            excluded = version_exclude != "None" and version_name in version_exclude

            if included and not excluded:
                # This item is the one we need to reference for extraction and rebuild the contents of if we're
                # writing it back out
                assert not was_included
                was_included = True

                self.included_version = version_ident

                for item2 in item:
                    if item2.tag == "Sample":
                        self.sample_info[int(item2.attrib["Offset"], 16)] = item2.attrib
                    elif item2.tag == "Blob":
                        self.blob_info[int(item2.attrib["Offset"], 16)] = item2.attrib
                    else:
                        assert False

        assert was_included

class SoundFontExtractionDescription(ExtractionDescription):

    def post_init(self, xml_root : Element, version_name : str):
        self.envelopes_info = []
        self.instruments_info = {}
        self.drums_info = []
        self.effects_info = []

        for item in xml_root:
            if item.tag == "Envelopes":
                for env in item:
                    assert env.tag == "Envelope"
                    self.envelopes_info.append(env.attrib["Name"])
            elif item.tag == "Instruments":
                for instr in item:
                    assert instr.tag == "Instrument"
                    self.instruments_info[int(instr.attrib["ProgramNumber"])] = instr.attrib["Name"]
            elif item.tag == "Drums":
                for drum in item:
                    self.drums_info.append(drum.attrib["Name"])
            elif item.tag == "Effects":
                for effect in item:
                    self.effects_info.append(effect.attrib["Name"])
            else:
                assert False, item.tag

class SequenceExtractionDescription(ExtractionDescription):

    def post_init(self, xml_root : Element, version_name : str):
        pass
