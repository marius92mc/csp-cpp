import typing


class IndicesCase(typing.NamedTuple):
    receiver: int
    sender: int
    content: int


class SeparatorsCodeBlock(typing.NamedTuple):
    open: str
    close: str


class SelectParser(object):
    """Parses a `*.cpp` file and leaves the content as it is apart
    from the `select` block of code.
    When it encounters the `select`, it outputs it as a C/C++ `define`.

    IMPORTANT
    Better leave the `select` code without comments, because the possibility of encountering a `select` string
    in a comment area is not handled yet.
    """
    _SELECT_KEYWORD: str = "select"
    _CASE_KEYWORD: str = "case"
    _INDICES_CASE = IndicesCase(receiver=0, sender=1, content=2)
    _CASE_SEPARATOR: str = "<-"
    _DEFAULT_CASE_NAME: str = "default"
    _FILE_ENCODING: str = "utf-16-le"
    _SEPARATORS_CODE_BLOCK = SeparatorsCodeBlock(open="{", close="}")

    # case message1 <- channel1:   or    case channel1 <- message1:
    # {
    #    content1
    # }
    # (receiver, sender, content)
    # The default case is codified as follows:
    # (None, default, content)
    CaseContent = typing.Tuple[typing.Optional[str], str, str]
    SelectContent = typing.List[CaseContent]

    def __init__(self, input_file_name: str, output_file_name: str) -> None:
        self._input_file_name: str = input_file_name
        self._output_file_name: str = output_file_name
        #self._case: typing.List[self.CaseContent] = []
        self._content: typing.List[str] = self._get_input_content()

    def _get_input_content(self) -> typing.List[str]:
        with open(self._input_file_name, "r", encoding=self._FILE_ENCODING) as input_file:
            return input_file.readlines()

    def _is_valid_index(self, index: int) -> bool:
        return index >= 0 and index < len(self._content)

    def _is_comment_line(self, index: int) -> bool:
        """Return True if it is a line starting with `//`, False otherwise."""
        if not self._is_valid_index(index):
            return False
        components: typing.List[str] = self._content[index].split()
        if not components:
            return False
        return components[0] == "//" or components[0] == "/*" or components[len(components) - 1] == "*/"

    def _get_previous_important_line(self, index: int) -> typing.Optional[str]:
        """Return the previous line which doesn't contain a comment with '//' or a newline."""
        previous_index: int = index - 1
        if not self._is_valid_index(previous_index):
            return None
        while previous_index >= 0:
            if not self._is_comment_line(previous_index) and self._content[previous_index] != "\n":
                return self._content[previous_index]
            previous_index = previous_index - 1
        return None

    def _parse_select_block(self, index: int, select_data: SelectContent) -> int:
        """Return the index of the last line in the current `select` block.
        IMPORTANT
        Please restrict to not having more than one block of code (separated by "{}") inside the `case`s.
        It handles only this case for the moment.
        """
        # ----------------------------------------
        def _is_default_case(line: str) -> bool:
            return line.split()[0] == self._DEFAULT_CASE_NAME + ":"

        def _get_receiver(line: str) -> typing.Optional[str]:
            if _is_default_case(line):
                return None
            receiver_component: typing.List[str] = line.split(self._CASE_KEYWORD)[1].split(
                self._CASE_SEPARATOR
            )[0].split()
            return receiver_component[0] if receiver_component else None

        def _get_sender(line: str) -> str:
            if _is_default_case(line):
                return self._DEFAULT_CASE_NAME
            sender: str = line.split(self._CASE_KEYWORD)[1].split(
                self._CASE_SEPARATOR
            )[self._INDICES_CASE.sender].split()[0]
            # It has ":".
            return sender[:len(sender) - 1]

        def _is_closing_select(index) -> bool:
            if not self._is_valid_index(index - 1):
                return False
            if not self._is_valid_index(index):
                return False
            previous_line: str = self._get_previous_important_line(index)
            current_line: str = self._content[index]
            if previous_line.split() == [] or current_line.split() == []:
                return False
            if previous_line.split()[0] == self._SEPARATORS_CODE_BLOCK.close:
                if current_line.split()[0] == self._SEPARATORS_CODE_BLOCK.close:
                    return True
            return False

        def _get_content(index: int) -> typing.Tuple[int, str]:
            """Return the index of the last line from the current `select`'s content and the actual content.
            For example, from
            ```
            case {
                //case content here
            }
            ```
            it will return
            ```
            {
                //case content here
            }
            ```
            """
            index += 1  # to go on the starting content area
            content: str = ""
            while True:
                if not self._is_valid_index(index):
                    break
                content += self._content[index]
                if self._content[index].split() == []:
                    index += 1
                    continue
                if self._content[index].split()[0] == self._SEPARATORS_CODE_BLOCK.close:
                    break
                index += 1
            return index, content
        # ----------------------------------------
        # `select` line
        index += 1

        while True:
            if not self._is_valid_index(index):
                break
            if _is_closing_select(index):
                break
            if self._content[index].split() == [] or self._is_comment_line(index):
                index += 1
                continue
            # `case` area
            receiver: str = _get_receiver(self._content[index])
            sender: str = _get_sender(self._content[index])
            index, content = _get_content(index)
            case_content: self.CaseContent = (receiver, sender, content)
            select_data.append(case_content)
            index += 1

        return index

    def parse(self) -> typing.List[SelectContent]:
        """Return an array of `select` data."""
        index: int = 0
        all_selects: typing.List[self.SelectContent] = []

        while True:
            if not self._is_valid_index(index):
                break
            line: str = self._content[index]
            if self._SELECT_KEYWORD not in line.split():
                index += 1
                continue

            # `select` area
            select_data: self.SelectContent = []
            index = self._parse_select_block(index, select_data)
            index += 1
            all_selects.append(select_data)


        return all_selects


def main():
    print(SelectParser("SelectTest.cpp", "").parse())


if __name__ == "__main__":
    main()
