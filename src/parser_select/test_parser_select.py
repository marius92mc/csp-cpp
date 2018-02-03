import typing

import pytest

from .parser_select import SelectParser


def test_general_case_overall() -> None:
    parser = SelectParser("SelectTest.cpp", "")
    assert parser.parse() == [
        [
            (
                'message1',
                'channel1',
                '        {\n            printf("1");\n        }   \n'
            ),
            (
                'message2',
                'channel2',
                '        {\n            printf("2");\n        }\n'
            ),
            (
                'channel3',
                'message3',
                '        {\n            printf("Succeeded to write on channel '
                '3\\n");\n        }\n'
            ),
            (
                None,
                'channel_GuiSim',
                '        {\n            printf("Gui");\n        }\n'
            ),
            (
                None,
                'default',
                '        {\n            print ("nothing");\n        }\n'
            )
        ]
    ]
