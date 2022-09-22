#! /usr/bin/python3
# Approximation to C preprocessing.
# Copyright (C) 2019-2022 Free Software Foundation, Inc.
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

"""
Simplified lexical analyzer for C preprocessing tokens.

Does not implement trigraphs.

Does not implement backslash-newline in the middle of any lexical
item other than a string literal.

Does not implement universal-character-names in identifiers.

Treats prefixed strings (e.g. L"...") as two tokens (L and "...").

Accepts non-ASCII characters only within comments and strings.
"""

import collections
import re

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
