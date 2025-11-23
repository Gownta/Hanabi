# AI Guidelines

After each change, do the following things:

- Implement unit tests to support the code.
- Run unit tests, using `python3 -m unittest discover -s tests -p "*_test.py"`
   If there are failures, then fix them
- Run `python3 -m black` to format the code.
- Commit the code, using git. Include a description of the changes, as well as the command used to test the code.


# Coding conventions

- When adding doc comments, if the doc comment is more then one line then start the comment on the line after the quotes. Examples:

    """
    multi-line
    comment
    """

    """single-line comments like this"""

- When adding comments to code, remove comments whose content can be inferred from the variable name.
