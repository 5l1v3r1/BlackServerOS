"""A decorator for separating I/O context.

This decorator isolates I/O context of target
function or method.

I/O Context is a mix of terminal related elements,
such as current stdout and readline completer
attributes.

This decorator is useful if you run something
that reconfigures the readline completer, or
needs to use the default stdout file descriptor
instead of the phpsploit's stdout wrapper.

"""

import sys


def isolate_io_context(function):
    def wrapper(*args, **kwargs):
        try:
            import readline
            handle_readline = True
        except:
            handle_readline = False

        # backup phpsploit I/O context
        if handle_readline:
            old_readline_completer = readline.get_completer()
            readline.set_completer((lambda x: x))
        old_stdout = sys.stdout
        # function's stdout is the default one
        sys.stdout = sys.__stdout__
        # execute function with fresh context
        try:
            retval = function(*args, **kwargs)
        # restore phpsploit I/O context
        finally:
            if handle_readline:
                readline.set_completer(old_readline_completer)
            sys.stdout = old_stdout
        # return function result
        return retval
    return wrapper
