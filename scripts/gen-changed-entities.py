#!/usr/bin/python3
# Copyright (C) 2019 Free Software Foundation, Inc.
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
# <http://www.gnu.org/licenses/>.
''' Generate a ChangeLog style output based on the git log.

This script takes two revisions as input and generates a ChangeLog style output
for all revisions between the two revisions.  This output is intended to be an
approximation and not the exact ChangeLog.

At a high level, the script enumerates all C source files (*.c and *.h) and
builds a tree of top level objects within macro conditionals.  The top level
objects the script currently attempts to identify are:

    - Include statements
    - Macro definitions and undefs
    - Declarations and definitions of variables and functions
    - Composite types

The script attempts to identify quirks typically used in glibc sources such as
the symbol hack macro calls that don't use a semicolon and tries to adjust for
them.

Known Limitations:

    - Does not identify changes in or to comments.  Comments are simply stripped
      out.
    - Weird nesting of macro conditionals may break things.  Attempts have been
      made to try and maintain state across macro conditional scopes, but
      there's still scope to fool the script.
    - Does not identify changes within functions.
'''
import subprocess
import sys
import os
import re
from enum import Enum

# General Utility functions.
def eprint(*args, **kwargs):
    ''' Print to stderr.
    '''
    print(*args, file=sys.stderr, **kwargs)


debug = False
def debug_print(*args, **kwargs):
    ''' Convenience function to print diagnostic information in the program.
    '''
    if debug:
        eprint(*args, **kwargs)


def usage(name):
    ''' Print program usage.
    '''
    eprint("usage: %s <from-ref> <to-ref>" % name)
    sys.exit(os.EX_USAGE)


def decode(string):
    ''' Attempt to decode a string.

    Decode a string read from the source file.  The multiple attempts are needed
    due to the presence of the page break characters and some tests in locales.
    '''
    codecs = ['utf8', 'latin1', 'cp1252']

    for i in codecs:
        try:
            return string.decode(i)
        except UnicodeDecodeError:
            pass

    eprint('Failed to decode: %s' % string)


#------------------------------------------------------------------------------
# C Parser.
#------------------------------------------------------------------------------


def new_block(name, type, contents, parent, flags = 0):
    '''  Create a new code block with the parent as PARENT.

    The code block is a basic structure around which the tree representation of
    the source code is built.  It has the following attributes:

    - name: A name to refer it by in the ChangeLog
    - type: Any one of the following types in BLOCK_TYPE.
    - contents: The contents of the block.  For a block of types file or
      macro_cond, this would be a list of blocks that it nests.  For other types
      it is a list with a single string specifying its contents.
    - parent: This is the parent of the current block, useful in setting up
      #elif or #else blocks in the tree.
    - flags: A special field to indicate some properties of the block. See
      BLOCK_FLAGS for values.
    '''
    block = {}
    block['matched'] = False
    block['name'] = name
    block['type'] = type
    block['contents'] = contents
    block['parent'] = parent
    if parent:
        parent['contents'].append(block)

    block['flags'] = flags

    return block


class block_flags(Enum):
    ''' Flags for the code block.
    '''
    else_block = 1
    macro_defined = 2
    macro_redefined = 3


class block_type(Enum):
    ''' Type of code block.
    '''
    file = 1
    macro_cond = 2
    macro_def = 3
    macro_undef = 4
    macro_include = 5
    macro_info = 6
    decl = 7
    func = 8
    composite = 9
    macrocall = 10
    fndecl = 11
    assign = 12
    struct = 13
    union = 14
    enum = 15


# A dictionary describing what each action (add, modify, delete) show up as in
# the ChangeLog output.
actions = {0:{'new': 'New', 'mod': 'Modified', 'del': 'Remove'},
           block_type.file:{'new': 'New file', 'mod': 'Modified file',
                            'del': 'Remove file'},
           block_type.macro_cond:{'new': 'New', 'mod': 'Modified',
                                  'del': 'Remove'},
           block_type.macro_def:{'new': 'New', 'mod': 'Modified',
                                 'del': 'Remove'},
           block_type.macro_include:{'new': 'Include file', 'mod': 'Modified',
                                     'del': 'Remove include'},
           block_type.macro_info:{'new': 'New preprocessor message',
                                  'mod': 'Modified', 'del': 'Remove'},
           block_type.decl:{'new': 'New', 'mod': 'Modified', 'del': 'Remove'},
           block_type.func:{'new': 'New function', 'mod': 'Modified function',
                 'del': 'Remove function'},
           block_type.composite:{'new': 'New', 'mod': 'Modified',
                                 'del': 'Remove'},
           block_type.struct:{'new': 'New struct', 'mod': 'Modified struct',
                                 'del': 'Remove struct'},
           block_type.union:{'new': 'New union', 'mod': 'Modified union',
                                 'del': 'Remove union'},
           block_type.enum:{'new': 'New enum', 'mod': 'Modified enum',
                                 'del': 'Remove enum'},
           block_type.macrocall:{'new': 'New', 'mod': 'Modified',
                                 'del': 'Remove'},
           block_type.fndecl:{'new': 'New function', 'mod': 'Modified',
                              'del': 'Remove'},
           block_type.assign:{'new': 'New', 'mod': 'Modified', 'del': 'Remove'}}

# Regular expressions.

# The __attribute__ are written in a bunch of different ways in glibc.
ATTRIBUTE = \
r'(((attribute_\w+)|(__attribute__\s*\(\([^;]+\)\))|(weak_function)|(_*ATTRIBUTE_*\w+)|(asm\s*\([^\)]+\)))\s*)*'

# Different Types that we recognize.  This is specifically to filter out any
# macro hack types.
TYPE_RE = r'(\w+|(ElfW\(\w+\)))'

# Function regex
FUNC_RE = re.compile(ATTRIBUTE + r'\s*(\w+)\s*\([^(][^{]+\)\s*{')

# The macrocall_re peeks into the next line to ensure that it doesn't eat up
# a FUNC by accident.  The func_re regex is also quite crude and only
# intends to ensure that the function name gets picked up correctly.
MACROCALL_RE = re.compile(r'(\w+)\s*(\(.*\))*$')

# Composite types such as structs and unions.
COMPOSITE_RE = re.compile(r'(struct|union|enum)\s*(\w*)\s*{')

# Static assignments.
ASSIGN_RE = re.compile(r'(\w+)\s*(\[[^\]]*\])*\s*([^\s]*attribute[\s\w()]+)?\s*=')

# Function Declarations.
FNDECL_RE = re.compile(r'\s*(\w+)\s*\([^\(][^;]*\)\s*(__THROW)?\s*(asm\s*\([?)]\))?\s*' + ATTRIBUTE + ';')

# Function pointer typedefs.
TYPEDEF_FN_RE = re.compile(r'\(\*(\w+)\)\s*\([^)]+\);')

# Simple decls.
DECL_RE = re.compile(r'(\w+)(\[\w*\])*\s*' + ATTRIBUTE + ';')

# __typeof decls.
TYPEOF_DECL_RE = re.compile(r'__typeof\s*\([\w\s]+\)\s*(\w+)\s*' + ATTRIBUTE + ';')


def remove_extern_c(op):
    ''' Process extern "C"/"C++" block nesting.

    The extern "C" nesting does not add much value so it's safe to almost always
    drop it.  Also drop extern "C++"
    '''
    new_op = []
    nesting = 0
    extern_nesting = 0
    for l in op:
        if '{' in l:
            nesting = nesting + 1
        if re.match(r'extern\s*"C"\s*{', l):
            extern_nesting = nesting
            continue
        if '}' in l:
            nesting = nesting - 1
            if nesting < extern_nesting:
                extern_nesting = 0
                continue
        new_op.append(l)

    # Now drop all extern C++ blocks.
    op = new_op
    new_op = []
    nesting = 0
    extern_nesting = 0
    in_cpp = False
    for l in op:
        if re.match(r'extern\s*"C\+\+"\s*{', l):
            nesting = nesting + 1
            in_cpp = True

        if in_cpp:
            if '{' in l:
                nesting = nesting + 1
            if '}' in l:
                nesting = nesting - 1
        if nesting == 0:
            new_op.append(l)

    return new_op


def remove_comments(op):
    ''' Remove comments.

    Return OP by removing all comments from it.
    '''
    debug_print('REMOVE COMMENTS')

    sep='\n'
    opstr = sep.join(op)
    opstr = re.sub(r'/\*.*?\*/', r'', opstr, flags=re.MULTILINE | re.DOTALL)
    opstr = re.sub(r'\\\n', r' ', opstr, flags=re.MULTILINE | re.DOTALL)
    new_op = list(filter(None, opstr.split(sep)))

    return new_op


def normalize_condition(name):
    ''' Make some minor transformations on macro conditions to make them more
    readable.
    '''
    # Negation with a redundant bracket.
    name = re.sub(r'!\s*\(\s*(\w+)\s*\)', r'! \1', name)
    # Pull in negation of equality.
    name = re.sub(r'!\s*\(\s*(\w+)\s*==\s*(\w+)\)', r'\1 != \2', name)
    # Pull in negation of inequality.
    name = re.sub(r'!\s*\(\s*(\w+)\s*!=\s*(\w+)\)', r'\1 == \2', name)
    # Fix simple double negation.
    name = re.sub(r'!\s*\(\s*!\s*(\w+)\s*\)', r'\1', name)
    # Similar, but nesting a complex expression.  Because of the greedy match,
    # this matches only the outermost brackets.
    name = re.sub(r'!\s*\(\s*!\s*\((.*)\)\s*\)$', r'\1', name)
    return name


def parse_preprocessor(op, loc, code, start = ''):
    ''' Parse a preprocessor directive.

    In case a preprocessor condition (i.e. if/elif/else), create a new code
    block to nest code into and in other cases, identify and add entities suchas
    include files, defines, etc.

    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this function
    - START is the string that should continue to be expanded in case we step
      into a new macro scope.

    - Returns: The next location to be read in the array.
    '''
    cur = op[loc]
    loc = loc + 1
    endblock = False

    debug_print('PARSE_MACRO: %s' % cur)

    # Remove the # and strip spaces again.
    cur = cur[1:].strip()

    # Include file.
    if cur.find('include') == 0:
        m = re.search(r'include\s*["<]?([^">]+)[">]?', cur)
        new_block(m.group(1), block_type.macro_include, [cur], code)

    # Macro definition.
    if cur.find('define') == 0:
        m = re.search(r'define\s+([a-zA-Z0-9_]+)', cur)
        name = m.group(1)
        exists = False
        # Find out if this is a redefinition.
        for c in code['contents']:
            if c['name'] == name and c['type'] == block_type.macro_def:
                c['flags'] = block_flags.macro_redefined
                exists = True
                break
        if not exists:
            new_block(m.group(1), block_type.macro_def, [cur], code,
                      block_flags.macro_defined)
            # Add macros as we encounter them.
            SYM_MACROS.append(m.group(1))

    # Macro undef.
    if cur.find('undef') == 0:
        m = re.search(r'undef\s+([a-zA-Z0-9_]+)', cur)
        new_block(m.group(1), block_type.macro_def, [cur], code)

    # #error and #warning macros.
    if cur.find('error') == 0 or cur.find('warning') == 0:
        m = re.search(r'(error|warning)\s+"?(.*)"?', cur)
        if m:
            name = m.group(2)
        else:
            name = '<blank>'
        new_block(name, block_type.macro_info, [cur], code)

    # Start of an #if or #ifdef block.
    elif cur.find('if') == 0:
        rem = re.sub(r'ifndef', r'!', cur).strip()
        rem = re.sub(r'(ifdef|defined|if)', r'', rem).strip()
        rem = normalize_condition(rem)
        ifdef = new_block(rem, block_type.macro_cond, [], code)
        ifdef['headcond'] = ifdef
        ifdef['start'] = start
        loc = c_parse(op, loc, ifdef, start)

    # End the previous #if/#elif and begin a new block.
    elif cur.find('elif') == 0 and code['parent']:
        rem = normalize_condition(re.sub(r'(elif|defined)', r'', cur).strip())
        # The #else and #elif blocks should go into the current block's parent.
        ifdef = new_block(rem, block_type.macro_cond, [], code['parent'])
        ifdef['headcond'] = code['headcond']
        loc = c_parse(op, loc, ifdef, code['headcond']['start'])
        endblock = True

    # End the previous #if/#elif and begin a new block.
    elif cur.find('else') == 0 and code['parent']:
        name = normalize_condition('!(' + code['name'] + ')')
        ifdef = new_block(name, block_type.macro_cond, [], code['parent'],
                          block_flags.else_block)
        ifdef['headcond'] = code['headcond']
        loc = c_parse(op, loc, ifdef, code['headcond']['start'])
        endblock = True

    elif cur.find('endif') == 0 and code['parent']:
        # Insert an empty else block if there isn't one.
        if code['flags'] != block_flags.else_block:
            name = normalize_condition('!(' + code['name'] + ')')
            ifdef = new_block(name, block_type.macro_cond, [], code['parent'],
                              block_flags.else_block)
            ifdef['headcond'] = code['headcond']
            loc = c_parse(op, loc - 1, ifdef, code['headcond']['start'])
        endblock = True

    return (loc, endblock)


def fast_forward_scope(cur, op, loc):
    ''' Consume lines in a code block.

    Consume all lines of a block of code such as a composite type declaration or
    a function declaration.

    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR

    - Returns: The next location to be read in the array as well as the updated
      value of CUR, which will now have the body of the function or composite
      type.
    '''
    nesting = cur.count('{') - cur.count('}')
    while nesting > 0 and loc < len(op):
        cur = cur + ' ' + op[loc]

        nesting = nesting + op[loc].count('{')
        nesting = nesting - op[loc].count('}')
        loc = loc + 1

    return (cur, loc)


def parse_decl(name, cur, op, loc, code, blocktype, match):
    ''' Parse a top level declaration.

    All types of declarations except function declarations.

    - NAME is the name of the declarated entity
    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this function
    - BLOCKTYPE is the type of the block
    - MATCH is the regex match object.

    - Returns: The next location to be read in the array.
    '''
    debug_print('FOUND DECL: %s' % name)
    new_block(name, blocktype, [cur], code)

    return loc


def parse_assign(name, cur, op, loc, code, blocktype, match):
    ''' Parse an assignment statement.

    This includes array assignments.

    - NAME is the name of the assigned entity
    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this
    - BLOCKTYPE is the type of the block
    - MATCH is the regex match object.

    - Returns: The next location to be read in the array.
    '''
    debug_print('FOUND ASSIGN: %s' % name)
    # Lap up everything up to semicolon.
    while ';' not in cur and loc < len(op):
        cur = op[loc]
        loc = loc + 1

    new_block(name, blocktype, [cur], code)

    return loc


def parse_composite(name, cur, op, loc, code, blocktype, match):
    ''' Parse a composite type.

    Match declaration of a composite type such as a sruct or a union..

    - NAME is the name of the composite type
    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this
    - BLOCKTYPE is the type of the block
    - MATCH is the regex match object.

    - Returns: The next location to be read in the array.
    '''
    # Lap up all of the struct definition.
    (cur, loc) = fast_forward_scope(cur, op, loc)

    if not name:
        if 'typedef' in cur:
            name = re.sub(r'.*}\s*(\w+);$', r'\1', cur)
        else:
            name= '<anoymous>'

    ctype = match.group(1)

    if ctype == 'struct':
        blocktype = block_type.struct
    if ctype == 'enum':
        blocktype = block_type.enum
    if ctype == 'union':
        blocktype = block_type.union

    new_block(name, blocktype, [cur], code)

    return loc


def parse_func(name, cur, op, loc, code, blocktype, match):
    ''' Parse a function.

    Match a function definition.

    - NAME is the name of the function
    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this
    - BLOCKTYPE is the type of the block
    - MATCH is the regex match object.

    - Returns: The next location to be read in the array.
    '''
    debug_print('FOUND FUNC: %s' % name)

    # Consume everything up to the ending brace of the function.
    (cur, loc) = fast_forward_scope(cur, op, loc)

    new_block(name, blocktype, [cur], code)

    return loc


SYM_MACROS = []
def parse_macrocall(cur, op, loc, code, blocktype, match):
    ''' Parse a macro call.

    Match a symbol hack macro calls that get added without semicolons.

    - CUR is the string to consume this expression from
    - OP is the string array for the file
    - LOC is the first unread location in CUR
    - CODE is the block to which we add this
    - BLOCKTYPE is the type of the block
    - MATCH is the regex match object.

    - Returns: The next location to be read in the array.
    '''

    # First we have the macros for symbol hacks and all macros we identified so
    # far.
    if cur.count('(') != cur.count(')'):
        return cur, loc
    if loc < len(op) and '{' in op[loc]:
        return cur, loc

    found = re.search(MACROCALL_RE, cur)
    if found:
        sym = found.group(1)
        name = found.group(2)
        if sym in SYM_MACROS:
            debug_print('FOUND MACROCALL: %s (%s)' % (sym, name))
            new_block(sym, blocktype, [cur], code)
            return '', loc

    # Next, there could be macros that get called right inside their #ifdef, but
    # without the semi-colon.
    if cur.strip() == code['name'].strip():
        debug_print('FOUND MACROCALL (without brackets): %s' % (cur))
        new_block(cur, blocktype, [cur], code)
        return '',loc

    return cur, loc


# Regular expressions and token consumption functions for expressions we
# encounter in C.
c_expr_parsers = [
        {'regex' : COMPOSITE_RE, 'func' : parse_composite, 'name' : 2,
            'type' : block_type.composite},
        {'regex' : ASSIGN_RE, 'func' : parse_assign, 'name' : 1,
            'type' : block_type.assign},
        {'regex' : TYPEOF_DECL_RE, 'func' : parse_decl, 'name' : 1,
            'type' : block_type.decl},
        {'regex' : TYPEDEF_FN_RE, 'func' : parse_decl, 'name' : 1,
            'type' : block_type.decl},
        {'regex' : FNDECL_RE, 'func' : parse_decl, 'name' : 1,
            'type' : block_type.fndecl},
        {'regex' : FUNC_RE, 'func' : parse_func, 'name' : 8,
            'type' : block_type.func},
        {'regex' : DECL_RE, 'func' : parse_decl, 'name' : 1,
            'type' : block_type.decl},
        # Special case at the end: macro call.
        {'regex' : None, 'func' : parse_macrocall, 'name' : 1,
            'type' : block_type.macrocall}]


def parse_c_expr(cur, op, loc, code):
    ''' Parse a C expression.

    CUR is the string to be parsed, which continues to grow until a match is
    found.  OP is the string array and LOC is the first unread location in the
    string array.  CODE is the block in which any identified expressions should
    be added.
    '''
    debug_print('PARSING: %s' % cur)

    for p in c_expr_parsers:
        if not p['regex']:
            return p['func'](cur, op, loc, code, p['type'], found)

        found = re.search(p['regex'], cur)
        if found:
            return '', p['func'](found.group(p['name']), cur, op, loc, code,
                                    p['type'], found)

    return cur, loc


# These are macros that break parsing and don't have any intuitie way to get
# around.  Just replace them with something parseable.
PROBLEM_MACROS = \
    [{'orig': r'ElfW\((\w+)\)', 'sub': r'\1__ELF_NATIVE_CLASS_t'},
     {'orig': r'(libc_freeres_fn)\s*\((\w+)\)', 'sub': r'static void \1__\2 (void)'},
     {'orig': r'(IMPL)\s*\((\w+), .*\)$', 'sub': r'static void \1__\2 (void) {}'},
     {'orig': r'__(BEGIN|END)_DECLS', 'sub': r''}]


def expand_problematic_macros(cur):
    ''' Replace problem macros with their substitutes in CUR.
    '''
    for p in PROBLEM_MACROS:
        cur = re.sub(p['orig'], p['sub'], cur)

    return cur


def c_parse(op, loc, code, start = ''):
    '''
    Parse the file line by line.  The function assumes a mostly GNU coding
    standard compliant input so it might barf with anything that is eligible for
    the Obfuscated C code contest.

    The basic idea of the parser is to identify macro conditional scopes and
    definitions, includes, etc. and then parse the remaining C code in the
    context of those macro scopes.  The parser does not try to understand the
    semantics of the code or even validate its syntax.  It only records high
    level symbols in the source and makes a tree structure to indicate the
    declaration/definition of those symbols and their scope in the macro
    definitions.

    OP is the string array.
    LOC is the first unparsed line.
    CODE is the block scope within which the parsing is currently going on.
    START is the string with which this parsing should start.
    '''
    cur = start
    endblock = False
    saved_cur = ''
    saved_loc = 0
    endblock_loc = loc

    while loc < len(op):
        nextline = op[loc]

        # Macros.
        if nextline[0] == '#':
            (loc, endblock) = parse_preprocessor(op, loc, code, cur)
            if endblock:
                endblock_loc = loc
        # Rest of C Code.
        else:
            cur = cur + ' ' + nextline
            cur = expand_problematic_macros(cur).strip()
            cur, loc = parse_c_expr(cur, op, loc + 1, code)

        if endblock and not cur:
            # If we are returning from the first #if block, we want to proceed
            # beyond the current block, not repeat it for any preceding blocks.
            if code['headcond'] == code:
                return loc
            else:
                return endblock_loc

    return loc

def drop_empty_blocks(tree):
    ''' Drop empty macro conditional blocks.
    '''
    newcontents = []

    for x in tree['contents']:
        if x['type'] != block_type.macro_cond or len(x['contents']) > 0:
            newcontents.append(x)

    for t in newcontents:
        if t['type'] == block_type.macro_cond:
            drop_empty_blocks(t)

    tree['contents'] = newcontents


def consolidate_tree_blocks(tree):
    ''' Consolidate common macro conditional blocks.

    Get macro conditional blocks at the same level but scatterred across the
    file together into a single common block to allow for better comparison.
    '''
    # Nothing to do for non-nesting blocks.
    if tree['type'] != block_type.macro_cond \
            and tree['type'] != block_type.file:
        return

    # Now for nesting blocks, get the list of unique condition names and
    # consolidate code under them.  The result also bunches up all the
    # conditions at the top.
    newcontents = []

    macros = [x for x in tree['contents'] \
              if x['type'] == block_type.macro_cond]
    macro_names = sorted(set([x['name'] for x in macros]))
    for m in macro_names:
        nc = [x['contents'] for x in tree['contents'] if x['name'] == m \
                and x['type'] == block_type.macro_cond]
        b = new_block(m, block_type.macro_cond, sum(nc, []), tree)
        consolidate_tree_blocks(b)
        newcontents.append(b)

    newcontents.extend([x for x in tree['contents'] \
                        if x['type'] != block_type.macro_cond])

    tree['contents'] = newcontents


def compact_tree(tree):
    ''' Try to reduce the tree to its minimal form.

    A source code tree in its simplest form may have a lot of duplicated
    information that may be difficult to compare and come up with a minimal
    difference.
    '''

    # First, drop all empty blocks.
    drop_empty_blocks(tree)

    # Macro conditions that nest the entire file aren't very interesting.  This
    # should take care of the header guards.
    if tree['type'] == block_type.file \
            and len(tree['contents']) == 1 \
            and tree['contents'][0]['type'] == block_type.macro_cond:
        tree['contents'] = tree['contents'][0]['contents']

    # Finally consolidate all macro conditional blocks.
    consolidate_tree_blocks(tree)


def build_macrocalls():
    ''' Build a list of macro calls used for symbol versioning and attributes.

    glibc uses a set of macro calls that do not end with a semi-colon and hence
    breaks our parser.  Identify those calls from include/libc-symbols.h and
    filter them out.
    '''
    with open('../include/libc-symbols.h') as macrofile:
        global SYM_MACROS
        op = macrofile.readlines()
        op = remove_comments(op)
        SYM_MACROS = [re.sub(r'.*define (\w+).*', r'\1', x[:-1]) for x in op \
                      if 'define ' in x]


def c_parse_output(op):
    ''' File parser.

    Parse the input array of lines OP and generate a tree structure to
    represent the file.  This tree structure is then used for comparison between
    the old and new file.
    '''
    build_macrocalls()
    tree = new_block('', block_type.file, [], None)
    op = remove_comments(op)
    op = remove_extern_c(op)
    op = [re.sub(r'#\s+', '#', x) for x in op]
    op = c_parse(op, 0, tree)
    compact_tree(tree)
    c_dump_tree(tree, 0)

    return tree


def print_change(tree, action, prologue = ''):
    ''' Print the nature of the differences found in the tree compared to the
    other tree.  TREE is the tree that changed, action is what the change was
    (Added, Removed, Modified) and prologue specifies the macro scope the change
    is in.  The function calls itself recursively for all macro condition tree
    nodes.
    '''

    if tree['type'] != block_type.macro_cond:
        print('\t%s(%s): %s.' % (prologue, tree['name'], action))
        return

    prologue = '%s[%s]' % (prologue, tree['name'])
    for t in tree['contents']:
        if t['type'] == block_type.macro_cond:
            print_change(t, action, prologue)
        else:
            print('\t%s(%s): %s.' % (prologue, t['name'], action))


def c_compare_trees(left, right, prologue = ''):
    ''' Compare two trees and print the difference.

    This routine is the entry point to compare two trees and print out their
    differences.  LEFT and RIGHT will always have the same name and type,
    starting with block_type.file and '' at the top level.
    '''

    if left['type'] == block_type.macro_cond or left['type'] == block_type.file:

        if left['type'] == block_type.macro_cond:
            prologue = '%s[%s]' % (prologue, left['name'])

        # Make sure that everything in the left tree exists in the right tree.
        for cl in left['contents']:
            found = False
            for cr in right['contents']:
                if not cl['matched'] and not cr['matched'] and \
                        cl['name'] == cr['name'] and cl['type'] == cr['type']:
                    cl['matched'] = cr['matched'] = True
                    c_compare_trees(cl, cr, prologue)
                    found = True
                    break
            if not found:
                print_change(cl, actions[cl['type']]['del'], prologue)

        # ... and vice versa.  This time we only need to look at unmatched
        # contents.
        for cr in right['contents']:
            if not cr['matched']:
                print_change(cr, actions[cr['type']]['new'], prologue)
    else:
        if left['contents'] != right['contents']:
            print_change(left, actions[left['type']]['mod'], prologue)


def c_dump_tree(tree, indent):
    ''' Print the entire tree.
    '''
    if not debug:
        return

    if tree['type'] == block_type.macro_cond or tree['type'] == block_type.file:
        print('%sScope: %s' % (' ' * indent, tree['name']))
        for c in tree['contents']:
            c_dump_tree(c, indent + 4)
        print('%sEndScope: %s' % (' ' * indent, tree['name']))
    else:
        if tree['type'] == block_type.func:
            print('%sFUNC: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.composite:
            print('%sCOMPOSITE: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.assign:
            print('%sASSIGN: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.fndecl:
            print('%sFNDECL: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.decl:
            print('%sDECL: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.macrocall:
            print('%sMACROCALL: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.macro_def:
            print('%sDEFINE: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.macro_include:
            print('%sINCLUDE: %s' % (' ' * indent, tree['name']))
        elif tree['type'] == block_type.macro_undef:
            print('%sUNDEF: %s' % (' ' * indent, tree['name']))
        else:
            print('%sMACRO LEAF: %s' % (' ' * indent, tree['name']))


def c_compare(oldfile, newfile):
    ''' Entry point for the C backend.

    Parse the two files into trees and compare them.  Print the result of the
    comparison in the ChangeLog-like format.
    '''
    debug_print('LEFT TREE')
    debug_print('-' * 80)
    left = c_parse_output(oldfile)

    debug_print('RIGHT TREE')
    debug_print('-' * 80)
    right = c_parse_output(newfile)

    c_compare_trees(left, right)


#------------------------------------------------------------------------------


# Register backends for specific file extensions.
BACKENDS = \
        {'.c': {'parse_output': c_parse_output, 'compare': c_compare},
         '.h': {'parse_output': c_parse_output, 'compare': c_compare}}


def get_backend(filename):
    ''' Get an appropriate backend for FILENAME.
    '''
    name, ext = os.path.splitext(filename)

    if not ext in BACKENDS.keys():
        return None

    return BACKENDS[ext]


def exec_git_cmd(args):
    ''' Execute a git command and return its result as a list of strings.
    '''
    args.insert(0, 'git')
    debug_print(args)
    proc = subprocess.Popen(args, stdout=subprocess.PIPE)

    # Clean up the output by removing trailing spaces, newlines and dropping
    # blank lines.
    op = [decode(x[:-1]).strip() for x in proc.stdout]
    op = [re.sub(r'[\s\f]+', ' ', x) for x in op]
    op = [x for x in op if x]
    return op


def analyze_diff(oldfile, newfile, filename):
    ''' Parse the output of the old and new files and print the difference.

    For input files OLDFILE and NEWFILE with name FILENAME, generate reduced
    trees for them and compare them.  We limit our comparison to only C source
    files.
    '''
    backend = get_backend(filename)

    if not backend:
        return

    backend['compare'](exec_git_cmd(['show', oldfile]),
                       exec_git_cmd(['show', newfile]))


IGNORE_LIST = [
    'ChangeLog',
    'sysdeps/x86_64/dl-trampoline.h'
    ]


def list_changes(commit):
    ''' List changes in a single commit.

    For the input commit id COMMIT, identify the files that have changed and the
    nature of their changes.  Print commit information in the ChangeLog format,
    calling into helper functions as necessary.
    '''

    op = exec_git_cmd(['show', '--date=short', '--raw', commit])
    authors = []
    date = ''
    merge = False
    copyright_exempt=''

    for l in op:
        if l.lower().find('copyright-paperwork-exempt:') == 0 \
                and 'yes' in l.lower():
            copyright_exempt=' (tiny change)'
        elif l.lower().find('co-authored-by:') == 0 or \
                l.find('Author:') == 0:
            author = l.split(':')[1]
            author = re.sub(r'(.*)\s*(<.*)', r'\1  \2', author)
            authors.append(author)
        elif l.find('Date:') == 0:
            date=l[5:].strip()
        elif l.find('Merge:') == 0:
            merge = True

    # Find raw commit information for all non-ChangeLog files.
    op = [x[1:] for x in op if len(x) > 0 and re.match(r'^:[0-9]+', x)]

    # Skip all ignored files.
    for ign in IGNORE_LIST:
        op = [x for x in op if ign not in x]

    # It was only the ChangeLog, ignore.
    if len(op) == 0:
        return

    print('%s  %s' % (date, authors[0]))

    if (len(authors) > 1):
        authors = authors[1:]
        for author in authors:
            print('            %s' % author)

    print()

    if merge:
       print('\t MERGE COMMIT: %s\n' % commit)
       return

    print('\tCOMMIT%s: %s\n' % (copyright_exempt, commit))

    # Each of these lines has a space separated format like so:
    # :<OLD MODE> <NEW MODE> <OLD REF> <NEW REF> <OPERATION> <FILE1> <FILE2>
    #
    # where OPERATION can be one of the following:
    # A: File added
    # D: File removed
    # M[0-9]{3}: File modified
    # R[0-9]{3}: File renamed, with the 3 digit number following it indicating
    # what percentage of the file is intact.
    # C[0-9]{3}: File copied.  Same semantics as R.
    # T: The permission bits of the file changed
    # U: Unmerged.  We should not encounter this, so we ignore it/
    # X, or anything else: Most likely a bug.  Report it.
    #
    # FILE2 is set only when OPERATION is R or C, to indicate the new file name.
    #
    # Also note that merge commits have a different format here, with three
    # entries each for the modes and refs, but we don't bother with it for now.
    #
    # For more details: https://git-scm.com/docs/diff-format
    for f in op:
        data = f.split()
        if data[4] == 'A':
            print('\t* %s: New file.' % data[5])
        elif data[4] == 'D':
            print('\t* %s: Delete file.' % data[5])
        elif data[4] == 'T':
            print('\t* %s: Changed file permission bits from %s to %s' % \
                    (data[5], data[0], data[1]))
        elif data[4][0] == 'M':
            print('\t* %s: Modified.' % data[5])
            analyze_diff(data[2], data[3], data[5])
        elif data[4][0] == 'R' or data[4][0] == 'C':
            change = int(data[4][1:])
            print('\t* %s: Move to...' % data[5])
            print('\t* %s: ... here.' % data[6])
            if change < 100:
                analyze_diff(data[2], data[3], data[6])
        # We should never encounter this, so ignore for now.
        elif data[4] == 'U':
            pass
        else:
            eprint('%s: Unknown line format %s' % (commit, data[4]))
            sys.exit(42)

    print('')


def list_commits(revs):
    ''' List commit IDs between the two revs in the REVS list.
    '''
    ref = revs[0] + '..' + revs[1]
    return exec_git_cmd(['log', '--pretty=%H', ref])


def main(revs):
    ''' ChangeLog Generator Entry Point.
    '''
    commits = list_commits(revs)
    for commit in commits:
        list_changes(commit)


def backend_file_test(f):
    ''' Parser debugger Entry Point.
    '''
    backend = get_backend(f)

    if not backend:
        debug_print('%s: No backend for this file type, cannot debug' % f)
        return

    with open(f) as srcfile:
        op = srcfile.readlines()
        # Form feeds and line feeds removed.
        op = [re.sub(r'\f+', r'', x[:-1]) for x in op]
        tree = backend['parse_output'](op)


# Program Entry point.  If -d is specified, the second argument is assumed to be
# a file and only the backend for the file is run in verbose mode.
if __name__ == '__main__':
    if len(sys.argv) != 3:
        usage(sys.argv[0])

    if sys.argv[1] == '-d':
        debug = True
        backend_file_test(sys.argv[2])
    else:
        main(sys.argv[1:])
