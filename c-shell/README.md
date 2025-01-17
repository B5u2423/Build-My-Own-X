# My Shell in C 

*The work of CodeCrafters course on making a bash shell with my own tweaks. Everything is in one and only file `main.c` because I love having everthing in one place when I was doing the tutorial.*

## What It Can Do

Bare minimum stuff on CodeCrafters like:

- Builtin commands: `echo`, `exit`, `type`, `cd` and `pwd`.
- Support executables available in the PATH.
- Handle single quotes `'`, and double quotes `"`, though not that gracefully.
- Redirection from `stdout` and `stderr` to a file.

## Limitations

A lot of edge cases and bugs that need to be ironed out. Might cause memory leaks (?), I did check with `valgrind` and no mem leak so far. No tests, all validations are manual.

Missing a lot of functionalities: parsing environment variables, arrow key support,... Even the working ones are not so robust and kind of flaky, the program might fold even the slightest deviation from the parsing.