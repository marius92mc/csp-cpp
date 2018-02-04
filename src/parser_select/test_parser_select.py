from py._path import local
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

def test_tmpdir(tmpdir: local.LocalPath) -> None:
    file_name: str = "hello.cpp"
    file_content: str = "#include <iostream>"
    p = tmpdir.join(file_name)
    p.write(file_content)

    assert tmpdir.join(file_name).isfile()
    assert p.read() == file_content


def _get_file_name() -> str:
    return "hello.cpp"


def _get_file_path(tmpdir: local.LocalPath) -> str:
    return str(tmpdir.join(_get_file_name()))


def _add_content(tmpdir: local.LocalPath, content: str) -> local.LocalPath:
    p = tmpdir.join(_get_file_name())
    p.write_text(content, encoding="utf-16-le")
    return p


def test_empty_content(tmpdir: local.LocalPath) -> None:
    _add_content(tmpdir, "#include <iostream>\n")
    select_data = SelectParser(_get_file_path(tmpdir), "").parse()
    assert select_data == []


@pytest.fixture
def cpp_includes() -> str:
    return '#include <iostream>\n' \
           '\n' \
           'using namespace std;\n' \
           '\n' \


def test_empty_select(tmpdir: local.LocalPath, cpp_includes: str) -> None:
    _add_content(tmpdir, cpp_includes +
                         "int main() {\n"
                         "  select {\n"
                         "  }\n"
                         "}\n")
    select_data = SelectParser(_get_file_path(tmpdir), "").parse()
    assert select_data == [[]]


def test_select_with_one_case(tmpdir: local.LocalPath, cpp_includes: str) -> None:
    _add_content(tmpdir, cpp_includes +
                         'int main() {\n'
                         '    select {\n' +
                         '      case message1 <- channel1:\n'
                         '      {\n'
                         '          std::cout << "1";\n'
                         '          std::cout << "2";\n'
                         '      }\n'
                         '  }\n'
                         '}\n')
    select_data = SelectParser(_get_file_path(tmpdir), "").parse()
    assert select_data == [
        [
            (
                'message1',
                'channel1',
                '      {\n'
                '          std::cout << "1";\n'
                '          std::cout << "2";\n'
                '      }\n'
            )
        ]
    ]


def test_select_with_just_one_default(tmpdir: local.LocalPath, cpp_includes: str) -> None:
    _add_content(tmpdir, cpp_includes +
                         'int main() {\n'
                         '    select {\n' +
                         '      default:\n'
                         '      {\n'
                         '          std::cout << "1";\n'
                         '          std::cout << "2";\n'
                         '      }\n'
                         '  }\n'
                         '}\n')
    select_data = SelectParser(_get_file_path(tmpdir), "").parse()
    assert select_data == [
        [
            (
                 None,
                'default',
                '      {\n'
                '          std::cout << "1";\n'
                '          std::cout << "2";\n'
                '      }\n'
            )
        ]
    ]
