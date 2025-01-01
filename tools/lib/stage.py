# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2024-2025 The TokTok team
import time
from typing import Any
from typing import Optional

sleep = time.sleep


class UserAbort(Exception):
    pass


class InvalidState(Exception):
    pass


def _isatty() -> bool:
    """Returns True if the output is a terminal."""
    try:
        import sys

        return sys.stdout.isatty()
    except Exception:
        return False


def _window_width() -> int:
    """Returns the width of the terminal window."""
    try:
        import shutil

        return shutil.get_terminal_size().columns
    except Exception:
        return 100


def clear() -> None:
    """Clear the line."""
    if not _isatty():
        return
    columns = _window_width()
    print("\r" + " " * columns + "\r", end="", flush=True)


def print_stage_start(name: str, description: str) -> None:
    """Prints a colorful message indicating that a stage is starting.

    Does not print a newline at the end.

    Looks roughly like:
       [ .... ] Stage name (requirement description)
    """
    clear()
    text = f"\033[1;34m[ .... ]\033[0m {name} {description}"
    print(text, end="", flush=True)


def print_stage_progress(name: str, description: str, start_time: int) -> None:
    """Prints a colorful message indicating that a stage is progressing.

    Does not print a newline at the end. Starts with a carriage return to
    overwrite the previous line. Prints the time elapsed since the start of the
    stage.

    Looks roughly like:
       [  30 ] Stage name (requirement description)
    """
    clear()
    elapsed = int(time.time()) - start_time
    text = f"\r\033[1;34m[ {elapsed:4d} ]\033[0m {name} {description}"
    print(text, end="", flush=True)


def print_stage_end(name: str, description: str, success: bool) -> None:
    """Prints a colorful message indicating that a stage has finished.

    Now (unlike the above print_stage_start) it prints a newline at the end to
    get ready for the next stage.

    Looks roughly like:
       [  OK  ] Stage name (result description)
    """
    clear()
    status = " \033[1;32mOK\033[0m " if success else "\033[1;31mFAIL\033[0m"
    text = f"\r\033[1;34m[ {status} \033[1;34m]\033[0m {name} {description}"
    print(text)


class Stage:
    """A class to run stages and print colorful messages for the user."""

    def __init__(
        self,
        name: str,
        description: str,
        failures: Optional[list[str]] = None,
        parent: Optional["Stage"] = None,
    ) -> None:
        """Initializes a new stage. Doesn't enter it yet (use `with` for that).

        The `failures` parameter is a list that will be appended to if the stage
        fails. If it's None, the stage will raise an exception on error instead.
        """
        self.failures = failures
        self.name = name
        self.parent = parent
        self.description = description
        self.done = False
        self.start_time = int(time.time())

    def __enter__(self) -> "Stage":
        if self.parent:
            self.progress(self.description)
        else:
            print_stage_start(self.name, f"({self.description})")
        return self

    def ok(self, description: str = "Done") -> None:
        print_stage_end(
            self.name,
            f"({description})",
            True,
        )
        self.done = True

    def progress(self, description: str) -> None:
        print_stage_progress(self.name,
                             f"({description})",
                             start_time=self.start_time)

    def fail(self, description: str) -> InvalidState:
        print_stage_end(
            self.name,
            f"({description})",
            False,
        )
        self.done = True
        exn = InvalidState(f"Stage {self.name} failed: {description}")
        if self.failures is None:
            raise exn
        self.failures.append(self.name)
        return exn

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> None:
        if not self.done:
            self.fail("The stage did not complete")
