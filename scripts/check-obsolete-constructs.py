#! /usr/bin/python3
# Copyright (C) 2019-2020 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

"""Verifies that installed headers do not use any obsolete constructs:

 * legacy BSD typedefs superseded by <stdint.h>:
      ushort uint ulong
      u_char u_short u_int u_long
      u_intNN_t quad_t u_quad_t
      caddr_t daddr_t loff_t register_t
   (sys/types.h is allowed to _define_ these types, but not to use them
    to define anything else).

 * nested includes of other top-level headers, except as required by
   the relevant standards
"""

import argparse
import collections
import os.path
import re
import sys

# Simplified lexical analyzer for C preprocessing tokens.
# Does not implement trigraphs.
# Does not implement backslash-newline in the middle of any lexical
#   item other than a string literal.
# Does not implement universal-character-names in identifiers.
# Treats prefixed strings (e.g. L"...") as two tokens (L and "...")
# Accepts non-ASCII characters only within comments and strings.

# Caution: The order of the outermost alternation matters.
# STRING must be before BAD_STRING, CHARCONST before BAD_CHARCONST,
# BLOCK_COMMENT before BAD_BLOCK_COM before PUNCTUATOR, and OTHER must
# be last.
# Caution: There should be no capturing groups other than the named
# captures in the outermost alternation.

# For reference, these are all of the C punctuators as of C11:
#   [ ] ( ) { } , ; ? ~
#   ! != * *= / /= ^ ^= = ==
#   # ##
#   % %= %> %: %:%:
#   & &= &&
#   | |= ||
#   + += ++
#   - -= -- ->
#   . ...
#   : :>
#   < <% <: << <<= <=
#   > >= >> >>=

# The BAD_* tokens are not part of the official definition of pp-tokens;
# they match unclosed strings, character constants, and block comments,
# so that the regex engine doesn't have to backtrack all the way to the
# beginning of a broken construct and then emit dozens of junk tokens.

PP_TOKEN_RE_ = re.compile(r"""
    (?P<STRING>        \"(?:[^\"\\\r\n]|\\(?:[\r\n -~]|\r\n))*\")
   |(?P<BAD_STRING>    \"(?:[^\"\\\r\n]|\\[ -~])*)
   |(?P<CHARCONST>     \'(?:[^\'\\\r\n]|\\(?:[\r\n -~]|\r\n))*\')
   |(?P<BAD_CHARCONST> \'(?:[^\'\\\r\n]|\\[ -~])*)
   |(?P<BLOCK_COMMENT> /\*(?:\*(?!/)|[^*])*\*/)
   |(?P<BAD_BLOCK_COM> /\*(?:\*(?!/)|[^*])*\*?)
   |(?P<LINE_COMMENT>  //[^\r\n]*)
   |(?P<IDENT>         [_a-zA-Z][_a-zA-Z0-9]*)
   |(?P<PP_NUMBER>     \.?[0-9](?:[0-9a-df-oq-zA-DF-OQ-Z_.]|[eEpP][+-]?)*)
   |(?P<PUNCTUATOR>
       [,;?~(){}\[\]]
     | [!*/^=]=?
     | \#\#?
     | %(?:[=>]|:(?:%:)?)?
     | &[=&]?
     |\|[=|]?
     |\+[=+]?
     | -[=->]?
     |\.(?:\.\.)?
     | :>?
     | <(?:[%:]|<(?:=|<=?)?)?
     | >(?:=|>=?)?)
   |(?P<ESCNL>         \\(?:\r|\n|\r\n))
   |(?P<WHITESPACE>    [ \t\n\r\v\f]+)
   |(?P<OTHER>         .)
""", re.DOTALL | re.VERBOSE)

HEADER_NAME_RE_ = re.compile(r"""
    < [^>\r\n]+ >
  | " [^"\r\n]+ "
""", re.DOTALL | re.VERBOSE)

ENDLINE_RE_ = re.compile(r"""\r|\n|\r\n""")

# based on the sample code in the Python re documentation
Token_ = collections.namedtuple("Token", (
    "kind", "text", "line", "column", "context"))
Token_.__doc__ = """
   One C preprocessing token, comment, or chunk of whitespace.
   'kind' identifies the token type, which will be one of:
       STRING, CHARCONST, BLOCK_COMMENT, LINE_COMMENT, IDENT,
       PP_NUMBER, PUNCTUATOR, ESCNL, WHITESPACE, HEADER_NAME,
       or OTHER.  The BAD_* alternatives in PP_TOKEN_RE_ are
       handled within tokenize_c, below.

   'text' is the sequence of source characters making up the token;
       no decoding whatsoever is performed.

   'line' and 'column' give the position of the first character of the
      token within the source file.  They are both 1-based.

   'context' indicates whether or not this token occurred within a
      preprocessing directive; it will be None for running text,
      '<null>' for the leading '#' of a directive line (because '#'
      all by itself on a line is a "null directive"), or the name of
      the directive for tokens within a directive line, starting with
      the IDENT for the name itself.
"""

def tokenize_c(file_contents, reporter):
    """Yield a series of Token objects, one for each preprocessing
       token, comment, or chunk of whitespace within FILE_CONTENTS.
       The REPORTER object is expected to have one method,
       reporter.error(token, message), which will be called to
       indicate a lexical error at the position of TOKEN.
       If MESSAGE contains the four-character sequence '{!r}', that
       is expected to be replaced by repr(token.text).
    """

    Token = Token_
    PP_TOKEN_RE = PP_TOKEN_RE_
    ENDLINE_RE = ENDLINE_RE_
    HEADER_NAME_RE = HEADER_NAME_RE_

    line_num = 1
    line_start = 0
    pos = 0
    limit = len(file_contents)
    directive = None
    at_bol = True
    while pos < limit:
        if directive == "include":
            mo = HEADER_NAME_RE.match(file_contents, pos)
            if mo:
                kind = "HEADER_NAME"
                directive = "after_include"
            else:
                mo = PP_TOKEN_RE.match(file_contents, pos)
                kind = mo.lastgroup
                if kind != "WHITESPACE":
                    directive = "after_include"
        else:
            mo = PP_TOKEN_RE.match(file_contents, pos)
            kind = mo.lastgroup

        text = mo.group()
        line = line_num
        column = mo.start() - line_start
        adj_line_start = 0
        # only these kinds can contain a newline
        if kind in ("WHITESPACE", "BLOCK_COMMENT", "LINE_COMMENT",
                    "STRING", "CHARCONST", "BAD_BLOCK_COM", "ESCNL"):
            for tmo in ENDLINE_RE.finditer(text):
                line_num += 1
                adj_line_start = tmo.end()
            if adj_line_start:
                line_start = mo.start() + adj_line_start

        # Track whether or not we are scanning a preprocessing directive.
        if kind == "LINE_COMMENT" or (kind == "WHITESPACE" and adj_line_start):
            at_bol = True
            directive = None
        else:
            if kind == "PUNCTUATOR" and text == "#" and at_bol:
                directive = "<null>"
            elif kind == "IDENT" and directive == "<null>":
                directive = text
            at_bol = False

        # Report ill-formed tokens and rewrite them as their well-formed
        # equivalents, so downstream processing doesn't have to know about them.
        # (Rewriting instead of discarding provides better error recovery.)
        if kind == "BAD_BLOCK_COM":
            reporter.error(Token("BAD_BLOCK_COM", "", line, column+1, ""),
                           "unclosed block comment")
            text += "*/"
            kind = "BLOCK_COMMENT"
        elif kind == "BAD_STRING":
            reporter.error(Token("BAD_STRING", "", line, column+1, ""),
                           "unclosed string")
            text += "\""
            kind = "STRING"
        elif kind == "BAD_CHARCONST":
            reporter.error(Token("BAD_CHARCONST", "", line, column+1, ""),
                           "unclosed char constant")
            text += "'"
            kind = "CHARCONST"

        tok = Token(kind, text, line, column+1,
                    "include" if directive == "after_include" else directive)
        # Do not complain about OTHER tokens inside macro definitions.
        # $ and @ appear in macros defined by headers intended to be
        # included from assembly language, e.g. sysdeps/mips/sys/asm.h.
        if kind == "OTHER" and directive != "define":
            self.error(tok, "stray {!r} in program")

        yield tok
        pos = mo.end()

#
# Base and generic classes for individual checks.
#

class ConstructChecker:
    """Scan a stream of C preprocessing tokens and possibly report
       problems with them.  The REPORTER object passed to __init__ has
       one method, reporter.error(token, message), which should be
       called to indicate a problem detected at the position of TOKEN.
       If MESSAGE contains the four-character sequence '{!r}' then that
       will be replaced with a textual representation of TOKEN.
    """
    def __init__(self, reporter):
        self.reporter = reporter

    def examine(self, tok):
        """Called once for each token in a header file.
           Call self.reporter.error if a problem is detected.
        """
        raise NotImplementedError

    def eof(self):
        """Called once at the end of the stream.  Subclasses need only
           override this if it might have something to do."""
        pass

class NoCheck(ConstructChecker):
    """Generic checker class which doesn't do anything.  Substitute this
       class for a real checker when a particular check should be skipped
       for some file."""

    def examine(self, tok):
        pass

#
# Check for obsolete type names.
#

# The obsolete type names we're looking for:
OBSOLETE_TYPE_RE_ = re.compile(r"""\A
  (__)?
  (   quad_t
    | [cd]addr_t
    | loff_t
    | register_t
    | u(?: short | int | long
         | _(?: char | short | int(?:[0-9]+_t)? | long | quad_t )))
\Z""", re.VERBOSE)

# The headers that declare them:
OBSOLETE_TYPE_HDR_RE_ = re.compile(r"""\A
   [<"] bits/types/
        (?: [cd]addr_t
          | loff_t
          | register_t
          | uint
          | u_int
          | u_intN_t ) \.h [">]
\Z""", re.VERBOSE)

class ObsoleteNotAllowed(ConstructChecker):
    """Don't allow any use of the obsolete typedefs,
       or the headers that declare them."""
    def examine(self, tok):
        if ((tok.kind == "IDENT"
             and OBSOLETE_TYPE_RE_.match(tok.text))
            or (tok.kind == "HEADER_NAME"
                and OBSOLETE_TYPE_HDR_RE_.match(tok.text))):
            self.reporter.error(tok, "use of {!r}")

class ObsoleteIndirectDefinitionsAllowed(ConstructChecker):
    """Don't allow any use of the obsolete typedefs,
       but do allow inclusion of the headers that declare them."""
    def examine(self, tok):
        if (tok.kind == "IDENT"
            and OBSOLETE_TYPE_RE_.match(tok.text)):
            self.reporter.error(tok, "use of {!r}")

class ObsoletePrivateDefinitionsAllowed(ConstructChecker):
    """Allow definitions of the private versions of the
       obsolete typedefs; that is, 'typedef [anything] __obsolete;'
       Don't allow inclusion of headers that declare the public
       versions.
    """
    def __init__(self, reporter):
        super().__init__(reporter)
        self.in_typedef = False
        self.prev_token = None

    def examine(self, tok):
        if (tok.kind == "IDENT"
            and tok.text == "typedef"
            and tok.context is None):
            self.in_typedef = True
        elif tok.kind == "PUNCTUATOR" and tok.text == ";" and self.in_typedef:
            self.in_typedef = False
            if self.prev_token.kind == "IDENT":
                m = OBSOLETE_TYPE_RE_.match(self.prev_token.text)
                if m and m.group(1) != "__":
                    self.reporter.error(self.prev_token, "use of {!r}")
            self.prev_token = None
        else:
            self._check_prev()

        self.prev_token = tok

    def eof(self):
        self._check_prev()

    def _check_prev(self):
        tok = self.prev_token
        if tok is None:
            return
        if ((tok.kind == "IDENT"
             and OBSOLETE_TYPE_RE_.match(tok.text))
            or (tok.kind == "HEADER_NAME"
                and OBSOLETE_TYPE_HDR_RE_.match(tok.text))):
            self.reporter.error(tok, "use of {!r}")

class ObsoletePublicDefinitionsAllowed(ConstructChecker):
    """Allow definitions of the public versions of the obsolete
       typedefs.  Only specific forms of definition are allowed:

           typedef __obsolete obsolete;  // identifiers must agree
           typedef __uintN_t u_intN_t;   // N must agree
           typedef unsigned long int u_?long;
           typedef unsigned short int u_?short;
           typedef unsigned int u_?int;
           typedef unsigned char u_char;
           typedef __int64_t quad_t;
           typedef __uint64_t u_quad_t;

       Don't allow inclusion of headers that declare public
       versions of other obsolete typedefs.
    """
    def __init__(self, reporter):
        super().__init__(reporter)
        self.typedef_tokens = []

    def examine(self, tok):
        if tok.kind in ("WHITESPACE", "BLOCK_COMMENT",
                        "LINE_COMMENT", "NL", "ESCNL"):
            pass

        elif (tok.kind == "IDENT" and tok.text == "typedef"
              and tok.context is None):
            if self.typedef_tokens:
                self.reporter.error(tok, "typedef inside typedef")
                self._reset()
            self.typedef_tokens.append(tok)

        elif tok.kind == "PUNCTUATOR" and tok.text == ";":
            self._finish()

        elif tok.kind == "HEADER_NAME":
            if OBSOLETE_TYPE_HDR_RE_.match(tok.text):
                self.reporter.error(tok, "use of {!r}")

        elif self.typedef_tokens:
            self.typedef_tokens.append(tok)

    def eof(self):
        self._reset()

    def _reset(self):
        while self.typedef_tokens:
            tok = self.typedef_tokens.pop(0)
            if tok.kind == "IDENT" and OBSOLETE_TYPE_RE_.match(tok.text):
                self.reporter.error(tok, "use of {!r}")

    def _finish(self):
        if not self.typedef_tokens: return
        if self.typedef_tokens[-1].kind == "IDENT":
            m = OBSOLETE_TYPE_RE_.match(self.typedef_tokens[-1].text)
            if m:
                if self._permissible_public_definition(m):
                    self.typedef_tokens.clear()
        self._reset()

    def _permissible_public_definition(self, m):
        if m.group(1) == "__":
            return False
        name = m.group(2)

        toks = self.typedef_tokens
        if len(toks) > 5:
            return False
        if any(tk.kind != "IDENT" for tk in toks):
            return False
        defn = " ".join(tk.text for tk in toks[1:-1])

        if name == "u_char":
            return defn == "unsigned char"

        if name in ("ushort", "u_short"):
            return defn == "unsigned short int"

        if name in ("uint", "u_int"):
            return defn == "unsigned int"

        if name in ("ulong", "u_long"):
            return defn == "unsigned long int"

        if name == "quad_t":
            return defn == "__int64_t"

        if name == "u_quad_t":
            return defn == "__uint64_t"

        if name[:5] == "u_int" and name[-2:] == "_t":
            return defn == "__uint" + name[5:-2] + "_t"

        return defn == "__" + name


def ObsoleteTypedefChecker(reporter, fname):
    """Factory: produce an instance of the appropriate
       obsolete-typedef checker for FNAME."""

    # The obsolete rpc/ and rpcsvc/ headers are allowed to use the
    # obsolete types, because it would be more trouble than it's
    # worth to remove them from headers that we intend to stop
    # installing eventually anyway.
    if (fname.startswith("rpc/")
        or fname.startswith("rpcsvc/")
        or "/rpc/" in fname
        or "/rpcsvc/" in fname):
        sys.stderr.write("# No typedef checks for {}\n".format(fname))
        return NoCheck(reporter)

    # bits/types.h is allowed to define the __-versions of the
    # obsolete types.
    if (fname == "bits/types.h"
        or fname.endswith("/bits/types.h")):
        sys.stderr.write("# Obsolete private defs allowed for {}\n"
                         .format(fname))
        return ObsoletePrivateDefinitionsAllowed(reporter)

    # Certain bits/types/ headers are allowed to define the
    # unprefixed versions of the obsolete types.
    if ((fname.startswith("bits/types/")
         or "/bits/types/" in fname)
        and os.path.basename(fname) in ("caddr_t.h",
                                        "daddr_t.h",
                                        "loff_t.h",
                                        "register_t.h",
                                        "uint.h",
                                        "u_int.h",
                                        "u_intN_t.h")):
        sys.stderr.write("# Obsolete public defs allowed for {}\n"
                         .format(fname))
        return ObsoletePublicDefinitionsAllowed(reporter)

    # sys/types.h is allowed to include the above bits/types/ headers.
    if (fname == "sys/types.h"
        or fname.endswith("/sys/types.h")):
        sys.stderr.write("# Obsolete indirect defs allowed for {}\n"
                         .format(fname))
        return ObsoleteIndirectDefinitionsAllowed(reporter)

    return ObsoleteNotAllowed(reporter)

#
# Nested includes
#

# All header files are allowed to include these headers.
# TODO: Every header file _should_ include features.h as its first inclusion,
# but we are not ready to enforce that yet.
UNIVERSAL_ALLOWED_INCLUDES = {
    "features.h",
}

# Specific headers are allowed to include specific other headers.
# Each key in this dictionary should be the installed name of a
# header, and its value is a list of installed names that are allowed
# to be included.  (We do not currently enforce any rules about <> vs
# "" inclusion.)
HEADER_ALLOWED_INCLUDES = {
    # ISO C standard headers
    # mandated: inttypes.h -> stdint.h
    #           tgmath.h   -> complex.h, math.h
    #           threads.h  -> time.h
    "inttypes.h":                  [ "stdint.h" ],
    "signal.h":                    [ "sys/ucontext.h" ],
    "stdlib.h":                    [ "alloca.h", "sys/types.h" ],
    "string.h":                    [ "strings.h" ],
    "tgmath.h":                    [ "complex.h", "math.h" ],
    "threads.h":                   [ "time.h" ],

    # POSIX top-level headers
    # mandated: pthread.h -> sched.h, time.h
    # allowed:  ftw.h -> sys/stat.h
    #           mqueue.h -> fcntl.h
    #           sched.h -> time.h
    #           spawn.h -> sched.h
    "aio.h":                       [ "sys/types.h" ],
    "ftw.h":                       [ "sys/stat.h", "sys/types.h" ],
    "langinfo.h":                  [ "nl_types.h" ],
    "mqueue.h":                    [ "fcntl.h", "sys/types.h" ],
    "pthread.h":                   [ "sched.h", "time.h" ],
    "regex.h":                     [ "limits.h", "sys/types.h" ],
    "sched.h":                     [ "time.h" ],
    "semaphore.h":                 [ "sys/types.h" ],
    "spawn.h":                     [ "sched.h", "sys/types.h" ],
    "termios.h":                   [ "sys/ttydefaults.h" ],

    # POSIX sys/ headers
    # mandated: sys/msg.h -> sys/ipc.h
    #           sys/sem.h -> sys/ipc.h
    #           sys/shm.h -> sys/ipc.h
    # allowed:  sys/time.h -> sys/select.h
    #           sys/wait.h -> signal.h
    "sys/msg.h":                   [ "sys/ipc.h" ],
    "sys/sem.h":                   [ "sys/ipc.h" ],
    "sys/shm.h":                   [ "sys/ipc.h" ],
    "sys/time.h":                  [ "sys/select.h" ],
    "sys/types.h":                 [ "endian.h", "sys/select.h" ],
    "sys/uio.h":                   [ "sys/types.h" ],
    "sys/un.h":                    [ "string.h" ],
    "sys/wait.h":                  [ "signal.h" ],

    # POSIX networking headers
    # allowed: netdb.h -> netinet/in.h
    #          arpa/inet.h -> netinet/in.h
    "netdb.h":                     [ "netinet/in.h", "rpc/netdb.h" ],
    "arpa/inet.h":                 [ "netinet/in.h" ],
    "net/if.h":                    [ "sys/socket.h", "sys/types.h" ],
    "netinet/in.h":                [ "sys/socket.h" ],
    "netinet/tcp.h":               [ "stdint.h", "sys/socket.h",
                                     "sys/types.h" ],

    # Nonstandardized top-level headers
    "aliases.h":                   [ "sys/types.h" ],
    "argp.h":                      [ "ctype.h", "errno.h", "getopt.h",
                                     "limits.h", "stdio.h" ],
    "argz.h":                      [ "errno.h", "string.h" ],
    "elf.h":                       [ "stdint.h" ],
    "envz.h":                      [ "argz.h", "errno.h" ],
    "fts.h":                       [ "sys/types.h" ],
    "gshadow.h":                   [ "paths.h" ],
    "ieee754.h":                   [ "float.h" ],
    "lastlog.h":                   [ "utmp.h" ],
    "libintl.h":                   [ "locale.h" ],
    "link.h":                      [ "dlfcn.h", "elf.h", "sys/types.h" ],
    "mntent.h":                    [ "paths.h" ],
    "nss.h":                       [ "stdint.h" ],
    "obstack.h":                   [ "stddef.h", "string.h" ],
    "proc_service.h":              [ "sys/procfs.h" ],
    "pty.h":                       [ "sys/ioctl.h", "termios.h" ],
    "sgtty.h":                     [ "sys/ioctl.h" ],
    "shadow.h":                    [ "paths.h" ],
    "stdio_ext.h":                 [ "stdio.h" ],
    "thread_db.h":                 [ "pthread.h", "stdint.h", "sys/procfs.h",
                                     "sys/types.h" ],
    "ucontext.h":                  [ "sys/ucontext.h" ],
    "utmp.h":                      [ "sys/types.h" ],
    "utmpx.h":                     [ "sys/time.h" ],
    "values.h":                    [ "float.h", "limits.h" ],

    # Nonstandardized sys/ headers
    "sys/acct.h":                  [ "endian.h", "stdint.h", "sys/types.h" ],
    "sys/auxv.h":                  [ "elf.h" ],
    "sys/elf.h":                   [ "sys/procfs.h" ],
    "sys/epoll.h":                 [ "stdint.h", "sys/types.h" ],
    "sys/eventfd.h":               [ "stdint.h" ],
    "sys/fanotify.h":              [ "stdint.h" ],
    "sys/file.h":                  [ "fcntl.h" ],
    "sys/fsuid.h":                 [ "sys/types.h" ],
    "sys/gmon.h":                  [ "sys/types.h" ],
    "sys/inotify.h":               [ "stdint.h" ],
    "sys/ioctl.h":                 [ "sys/ttydefaults.h" ],
    "sys/mount.h":                 [ "sys/ioctl.h" ],
    "sys/mtio.h":                  [ "sys/ioctl.h", "sys/types.h" ],
    "sys/param.h":                 [ "endian.h", "limits.h", "signal.h",
                                     "sys/types.h" ],
    "sys/platform/ppc.h":          [ "stdint.h" ],
    "sys/procfs.h":                [ "sys/time.h", "sys/types.h",
                                     "sys/user.h" ],
    "sys/profil.h":                [ "sys/time.h", "sys/types.h" ],
    "sys/ptrace.h":                [ "sys/ucontext.h" ],
    "sys/quota.h":                 [ "sys/types.h" ],
    "sys/random.h":                [ "sys/types.h" ],
    "sys/raw.h":                   [ "stdint.h", "sys/ioctl.h" ],
    "sys/sendfile.h":              [ "sys/types.h" ],
    "sys/signalfd.h":              [ "stdint.h" ],
    "sys/socketvar.h":             [ "sys/socket.h" ],
    "sys/timerfd.h":               [ "time.h" ],
    "sys/timex.h":                 [ "sys/time.h" ],
    "sys/ttychars.h":              [ "sys/ttydefaults.h" ],
    "sys/ucontext.h":              [ "sys/procfs.h" ],
    "sys/vfs.h":                   [ "sys/statfs.h" ],
    "sys/xattr.h":                 [ "sys/types.h" ],

    # Nonstandardized headers that do nothing but include some other
    # header(s).  These exist for compatibility with old systems where
    # the included header did not exist or didn't provide all the
    # necessary definitions.
    "memory.h":                    [ "string.h" ],
    "re_comp.h":                   [ "regex.h" ],
    "sys/bitypes.h":               [ "sys/types.h" ],
    "sys/dir.h":                   [ "dirent.h" ],
    "sys/errno.h":                 [ "errno.h" ],
    "sys/fcntl.h":                 [ "fcntl.h" ],
    "sys/poll.h":                  [ "poll.h" ],
    "sys/signal.h":                [ "signal.h" ],
    "sys/syslog.h":                [ "syslog.h" ],
    "sys/termios.h":               [ "termios.h" ],
    "sys/unistd.h":                [ "unistd.h" ],
    "syscall.h":                   [ "sys/syscall.h" ],
    "termio.h":                    [ "sys/ioctl.h", "termios.h" ],
    "wait.h":                      [ "sys/wait.h" ],

    # Nonstandardized networking headers
    "ifaddrs.h":                   [ "sys/socket.h" ],
    "resolv.h":                    [ "arpa/nameser.h", "netinet/in.h",
                                     "stdio.h", "sys/param.h",
                                     "sys/types.h" ],

    "arpa/nameser.h":              [ "arpa/nameser_compat.h", "stdint.h",
                                     "sys/param.h", "sys/types.h" ],
    "net/ethernet.h":              [ "stdint.h", "sys/types.h",
                                     "net/if_ether.h" ],
    "net/if_arp.h":                [ "stdint.h", "sys/socket.h",
                                     "sys/types.h" ],
    "net/if_ppp.h":                [ "net/if.h", "net/ppp_defs.h", "stdint.h",
                                     "sys/ioctl.h", "sys/types.h" ],
    "net/if_shaper.h":             [ "net/if.h", "stdint.h", "sys/ioctl.h",
                                     "sys/types.h" ],
    "net/route.h":                 [ "netinet/in.h", "sys/socket.h",
                                     "sys/types.h" ],
    "netatalk/at.h":               [ "sys/socket.h" ],
    "netinet/ether.h":             [ "netinet/if_ether.h" ],
    "netinet/icmp6.h":             [ "inttypes.h", "netinet/in.h", "string.h",
                                     "sys/types.h" ],
    "netinet/if_ether.h":          [ "net/ethernet.h", "net/if_arp.h",
                                     "sys/types.h", "stdint.h" ],
    "netinet/if_fddi.h":           [ "stdint.h", "sys/types.h" ],
    "netinet/if_tr.h":             [ "stdint.h", "sys/types.h" ],
    "netinet/igmp.h":              [ "netinet/in.h", "sys/types.h" ],
    "netinet/in_systm.h":          [ "stdint.h", "sys/types.h" ],
    "netinet/ip.h":                [ "netinet/in.h", "sys/types.h" ],
    "netinet/ip6.h":               [ "inttypes.h", "netinet/in.h" ],
    "netinet/ip_icmp.h":           [ "netinet/in.h", "netinet/ip.h",
                                     "stdint.h", "sys/types.h" ],
    "netinet/udp.h":               [ "stdint.h", "sys/types.h" ],
    "netipx/ipx.h":                [ "stdint.h", "sys/types.h" ],
    "netrom/netrom.h":             [ "netax25/ax25.h" ],
    "netrose/rose.h":              [ "netax25/ax25.h", "sys/socket.h" ],
    "protocols/routed.h":          [ "sys/socket.h" ],
    "protocols/rwhod.h":           [ "paths.h", "sys/types.h" ],
    "protocols/talkd.h":           [ "stdint.h", "sys/socket.h",
                                     "sys/types.h" ],
    "protocols/timed.h":           [ "sys/time.h", "sys/types.h" ],

    # Internal headers
    "features.h":                  [ "gnu/stubs.h", "stdc-predef.h",
                                     "sys/cdefs.h" ],

    "bits/fcntl.h":                [ "sys/types.h" ],
    "bits/ipc.h":                  [ "sys/types.h" ],
    "bits/procfs.h":               [ "signal.h", "sys/ucontext.h" ],
    "bits/sem.h":                  [ "sys/types.h" ],
    "bits/socket.h":               [ "sys/types.h" ],
    "bits/types/res_state.h":      [ "netinet/in.h", "sys/types.h" ],
    "bits/utmp.h":                 [ "paths.h", "sys/time.h", "sys/types.h" ],
    "bits/utmpx.h":                [ "paths.h", "sys/time.h" ],

    "bits/types/__va_list.h":      [ "stdarg.h" ],
    "bits/types/ptrdiff_t.h":      [ "stddef.h" ],
    "bits/types/size_t.h":         [ "stddef.h" ],
    "bits/types/wchar_t.h":        [ "stddef.h" ],
    "bits/NULL.h":                 [ "stddef.h" ],
}

# As above, but each group of whitelist entries is only used for
# headers whose pathname includes a sysdeps directory with that name.
# This allows us, for instance, to restrict the use of Linux kernel
# headers to the Linux-specific variants of glibc headers.
SYSDEP_ALLOWED_INCLUDES = {
    'linux': {
        # Nonstandardized sys/ headers
        "sys/cachectl.h":          [ "asm/cachectl.h" ],
        "sys/fanotify.h":          [ "linux/fanotify.h" ],
        "sys/kd.h":                [ "linux/kd.h" ],
        "sys/pci.h":               [ "linux/pci.h" ],
        "sys/prctl.h":             [ "linux/prctl.h" ],
        "sys/quota.h":             [ "linux/quota.h" ],
        "sys/syscall.h":           [ "asm/unistd.h" ],
        "sys/sysctl.h":            [ "linux/sysctl.h" ],
        "sys/sysinfo.h":           [ "linux/kernel.h" ],
        "sys/user.h":              [ "asm/ptrace.h", "asm/reg.h" ],
        "sys/vm86.h":              [ "asm/vm86.h" ],

        # Nonstandardized networking headers
        "net/ethernet.h":          [ "linux/if_ether.h" ],
        "net/if_slip.h":           [ "linux/if_slip.h" ],
        "net/ppp_defs.h":          [ "asm/types.h", "linux/ppp_defs.h" ],
        "netatalk/at.h":           [ "asm/types.h", "linux/atalk.h" ],
        "netinet/if_ether.h":      [ "linux/if_ether.h" ],
        "netinet/if_fddi.h":       [ "linux/if_fddi.h" ],

        # Alternative names for kernel headers
        "net/ppp-comp.h":          [ "linux/ppp-comp.h" ],
        "nfs/nfs.h":               [ "linux/nfs.h" ],
        "sys/soundcard.h":         [ "linux/soundcard.h" ],
        "sys/vt.h":                [ "linux/vt.h" ],

        # Internal headers
        "bits/errno.h":            [ "linux/errno.h" ],
        "bits/fcntl-linux.h":      [ "linux/falloc.h" ],
        "bits/ioctl-types.h":      [ "asm/ioctls.h" ],
        "bits/ioctls.h":           [ "asm/ioctls.h", "linux/sockios.h" ],
        "bits/local_lim.h":        [ "linux/limits.h" ],
        "bits/param.h":            [ "linux/limits.h", "linux/param.h" ],
        "bits/procfs.h":           [ "asm/ptrace.h" ],
        "bits/sigcontext.h":       [ "asm/sigcontext.h" ],
        "bits/socket.h":           [ "asm/socket.h" ],
    },

    'mach': {
        "bits/spin-lock-inline.h": [ "errno.h", "lock-intern.h" ],
        "bits/sigcontext.h":       [ "mach/machine/fp_reg.h" ],
    },

    'mips': {
        "fpregdef.h":              [ "sys/fpregdef.h" ],
        "regdef.h":                [ "sys/fpregdef.h", "sys/regdef.h" ],

        "sys/asm.h":               [ "sgidefs.h" ],
        "sys/fpregdef.h":          [ "sgidefs.h" ],
        "sys/regdef.h":            [ "sgidefs.h" ],
        "sys/tas.h":               [ "sgidefs.h" ],
        "sys/ucontext.h":          [ "sgidefs.h" ],
        "sys/user.h":              [ "sgidefs.h" ],

        "bits/fcntl.h":            [ "sgidefs.h" ],
        "bits/link.h":             [ "sgidefs.h" ],
        "bits/long-double.h":      [ "sgidefs.h" ],
        "bits/procfs.h":           [ "sgidefs.h" ],
        "bits/setjmp.h":           [ "sgidefs.h" ],
        "bits/sigcontext.h":       [ "sgidefs.h" ],
        "bits/stat.h":             [ "sgidefs.h" ],
        "bits/wordsize.h":         [ "sgidefs.h" ],
    },
}

# Headers that are exempt from the nested includes check.  This is a
# giant regex because fnmatch.fnmatch doesn't treat / specially and
# glob.glob can't be applied to anything but the file system.
#
# Hurd-specific headers are exempt for now, because for them, nested
# include minimization is not a pure cleanup project, the way it is
# for the standard headers.  The Hurd maintainers first need to make
# some design decisions about which headers _should_ include which
# other headers.
#
# Sun RPC headers are exempt because most of them are obsolete within
# glibc; cleanups should be done within the TIRPC project instead.
# (Same rationale as for exempting them from the obsolete-types checks.)
NESTED_INCLUDES_EXEMPT_RE = re.compile(r"""
    (?: \A | / ) (?:

    # Hurd-specific headers
        faultexc_server\.h
      | hurd\.h
      | lock-intern\.h
      | mach\.h
      | mach_error\.h
      | mach_init\.h
      | mach-shortcuts\.h
      | spin-lock\.h
      | device/[^/]+?\.h
      | mach/[^/]+?\.h
      | mach/i386/[^/]+?\.h
      | hurd/[^/]+?\.h

    # Sun RPC headers
      | rpc/[^/]+?\.[hx]
      | rpcsvc/[^/]+?\.[hx]

    ) \Z
""", re.VERBOSE)


def get_allowed_nested(fname):
    """Retrieve the set of allowed nested includes for a header whose
       filename is FNAME."""
    HEADER_ALLOWED = HEADER_ALLOWED_INCLUDES
    SYSDEP_ALLOWED = SYSDEP_ALLOWED_INCLUDES

    allowed = UNIVERSAL_ALLOWED_INCLUDES.copy()
    sysdirs = {}

    # HEADER_ALLOWED is keyed by the name to be used in an #include,
    # which is some suffix of the filename; we don't know how many
    # directory components to chop off, so we go one at a time until
    # we find a match.  os.path does not include a utility function to
    # chop off the _first_ component of a pathname.
    fname = os.path.normpath(fname)
    inc = fname
    while True:
        try:
            allowed.update(HEADER_ALLOWED[inc])
            break

        except KeyError:
            pos = inc.find('/')
            if pos == -1:
                break
            dirname = inc[:pos]
            inc = inc[(pos+1):]
            if not inc:
                break
            if dirname in SYSDEP_ALLOWED and dirname not in sysdirs:
                sysdirs[dirname] = SYSDEP_ALLOWED[dirname]

    for hgroup in sysdirs.values():
        inc = fname
        while True:
            try:
                allowed.update(hgroup[inc])
                break
            except KeyError:
                pos = inc.find('/')
                if pos == -1:
                    break
                dirname = inc[:pos]
                inc = inc[(pos+1):]
                if not inc:
                    break

    return frozenset(allowed)

class NestedIncludeWhitelistOnly(ConstructChecker):
    def __init__(self, reporter, fname):
        super().__init__(reporter)
        self.allowed_nested = get_allowed_nested(fname)

    def examine(self, tok):
        if tok.kind == "HEADER_NAME":
            included = tok.text[1:-1] # chop off "" or <>
            if included in self.allowed_nested:
                pass
            # We allow any public header to include any bits header.
            # More specific rules about which bits headers should be
            # used by which public headers are enforced by the bits
            # headers themselves.
            elif included.startswith("bits/"):
                pass
            else:
                self.reporter.error(tok, "inappropriate inclusion of {!r}")


def NestedIncludeChecker(reporter, fname):
    if NESTED_INCLUDES_EXEMPT_RE.search(fname):
        sys.stderr.write("# Arbitrary nested includes allowed for {}\n"
                         .format(fname))
        return NoCheck(reporter)
    else:
        return NestedIncludeWhitelistOnly(reporter, fname)

#
# Master control
#

class HeaderChecker:
    """Perform all of the checks on each header.  This is also the
       "reporter" object expected by tokenize_c and ConstructChecker.
    """
    def __init__(self):
        self.fname = None
        self.status = 0

    def error(self, tok, message):
        self.status = 1
        if '{!r}' in message:
            message = message.format(tok.text)
        sys.stderr.write("{}:{}:{}: error: {}\n".format(
            self.fname, tok.line, tok.column, message))

    def check(self, fname):
        self.fname = fname
        try:
            with open(fname, "rt", encoding="utf-8") as fp:
                contents = fp.read()
        except OSError as e:
            sys.stderr.write("{}: {}\n".format(fname, e.strerror))
            self.status = 1
            return

        typedef_checker = ObsoleteTypedefChecker(self, self.fname)
        nested_checker = NestedIncludeChecker(self, self.fname)

        for tok in tokenize_c(contents, self):
            typedef_checker.examine(tok)
            nested_checker.examine(tok)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("headers", metavar="header", nargs="+",
                    help="one or more headers to scan for obsolete constructs")
    args = ap.parse_args()

    checker = HeaderChecker()
    for fname in args.headers:
        # Headers whose installed name begins with "finclude/" contain
        # Fortran, not C, and this program should completely ignore them.
        if not (fname.startswith("finclude/") or "/finclude/" in fname):
            checker.check(fname)
    sys.exit(checker.status)

main()
