#!/usr/bin/python3
# The C Parser.
# Copyright (C) 2019-2020 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

from enum import Enum
import re
from vcs_to_changelog.misc_util import *

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
    block['actions'] = actions[type]

    return block


class ExprParser:
    ''' Parent class of all of the C expression parsers.

    It is necessary that the children override the parse_line() method.
    '''
    ATTRIBUTE = r'(((__attribute__\s*\(\([^;]+\)\))|(asm\s*\([?)]+\)))\s*)*'

    def __init__(self, project_quirks, debug):
        self.project_quirks = project_quirks
        self.debug = debug

    def fast_forward_scope(self, cur, op, loc):
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

    def parse_line(self, cur, op, loc, code, macros):
        ''' The parse method should always be overridden by the child.
        '''
        raise


class FuncParser(ExprParser):
    REGEX = re.compile(ExprParser.ATTRIBUTE + r'\s*(\w+)\s*\([^(][^{]+\)\s*{')

    def parse_line(self, cur, op, loc, code, macros):
        ''' Parse a function.

        Match a function definition.

        - CUR is the string to consume this expression from
        - OP is the string array for the file
        - LOC is the first unread location in CUR
        - CODE is the block to which we add this

        - Returns: The next location to be read in the array.
        '''
        found = re.search(self.REGEX, cur)
        if not found:
            return cur, loc

        name = found.group(5)
        self.debug.print('FOUND FUNC: %s' % name)

        # Consume everything up to the ending brace of the function.
        (cur, loc) = self.fast_forward_scope(cur, op, loc)

        new_block(name, block_type.func, [cur], code)

        return '', loc


class CompositeParser(ExprParser):
    # Composite types such as structs and unions.
    REGEX = re.compile(r'(struct|union|enum)\s*(\w*)\s*{')

    def parse_line(self, cur, op, loc, code, macros):
        ''' Parse a composite type.

        Match declaration of a composite type such as a sruct or a union..

        - CUR is the string to consume this expression from
        - OP is the string array for the file
        - LOC is the first unread location in CUR
        - CODE is the block to which we add this

        - Returns: The next location to be read in the array.
        '''
        found = re.search(self.REGEX, cur)
        if not found:
            return cur, loc

        # Lap up all of the struct definition.
        (cur, loc) = self.fast_forward_scope(cur, op, loc)

        name = found.group(2)

        if not name:
            if 'typedef' in cur:
                name = re.sub(r'.*}\s*(\w+);$', r'\1', cur)
            else:
                name= '<anoymous>'

        ctype = found.group(1)

        if ctype == 'struct':
            blocktype = block_type.struct
        if ctype == 'enum':
            blocktype = block_type.enum
        if ctype == 'union':
            blocktype = block_type.union

        new_block(name, block_type.composite, [cur], code)

        return '', loc


class AssignParser(ExprParser):
    # Static assignments.
    REGEX = re.compile(r'(\w+)\s*(\[[^\]]*\])*\s*([^\s]*attribute[\s\w()]+)?\s*=')

    def parse_line(self, cur, op, loc, code, macros):
        ''' Parse an assignment statement.

        This includes array assignments.

        - CUR is the string to consume this expression from
        - OP is the string array for the file
        - LOC is the first unread location in CUR
        - CODE is the block to which we add this

        - Returns: The next location to be read in the array.
        '''
        found = re.search(self.REGEX, cur)
        if not found:
            return cur, loc

        name = found.group(1)
        self.debug.print('FOUND ASSIGN: %s' % name)
        # Lap up everything up to semicolon.
        while ';' not in cur and loc < len(op):
            cur = op[loc]
            loc = loc + 1

        new_block(name, block_type.assign, [cur], code)

        return '', loc


class DeclParser(ExprParser):
    # Function pointer typedefs.
    TYPEDEF_FN_RE = re.compile(r'\(\*(\w+)\)\s*\([^)]+\);')

    # Simple decls.
    DECL_RE = re.compile(r'(\w+)(\[\w*\])*\s*' + ExprParser.ATTRIBUTE + ';')

    # __typeof decls.
    TYPEOF_RE = re.compile(r'__typeof\s*\([\w\s]+\)\s*(\w+)\s*' + \
                           ExprParser.ATTRIBUTE + ';')

    # Function Declarations.
    FNDECL_RE = re.compile(r'\s*(\w+)\s*\([^\(][^;]*\)\s*' +
                           ExprParser.ATTRIBUTE + ';')

    def __init__(self, regex, blocktype, project_quirks, debug):
        # The regex for the current instance.
        self.REGEX = regex
        self.blocktype = blocktype
        super().__init__(project_quirks, debug)

    def parse_line(self, cur, op, loc, code, macros):
        ''' Parse a top level declaration.

        All types of declarations except function declarations.

        - CUR is the string to consume this expression from
        - OP is the string array for the file
        - LOC is the first unread location in CUR
        - CODE is the block to which we add this function

        - Returns: The next location to be read in the array.
        '''
        found = re.search(self.REGEX, cur)
        if not found:
            return cur, loc

        # The name is the first group for all of the above regexes.  This is a
        # coincidence, so care must be taken if regexes are added or changed to
        # ensure that this is true.
        name = found.group(1)

        self.debug.print('FOUND DECL: %s' % name)
        new_block(name, self.blocktype, [cur], code)

        return '', loc


class MacroParser(ExprParser):
    # The macrocall_re peeks into the next line to ensure that it doesn't
    # eat up a FUNC by accident.  The func_re regex is also quite crude and
    # only intends to ensure that the function name gets picked up
    # correctly.
    MACROCALL_RE = re.compile(r'(\w+)\s*(\(.*\))*$')

    def parse_line(self, cur, op, loc, code, macros):
        ''' Parse a macro call.

        Match a symbol hack macro calls that get added without semicolons.

        - CUR is the string to consume this expression from
        - OP is the string array for the file
        - LOC is the first unread location in CUR
        - CODE is the block to which we add this
        - MACROS is the regex match object.

        - Returns: The next location to be read in the array.
        '''

        # First we have the macros for symbol hacks and all macros we identified so
        # far.
        if cur.count('(') != cur.count(')'):
            return cur, loc
        if loc < len(op) and '{' in op[loc]:
            return cur, loc

        found = re.search(self.MACROCALL_RE, cur)
        if found:
            sym = found.group(1)
            name = found.group(2)
            if sym in macros or self.project_quirks and \
                    sym in self.project_quirks.C_MACROS:
                self.debug.print('FOUND MACROCALL: %s (%s)' % (sym, name))
                new_block(sym, block_type.macrocall, [cur], code)
                return '', loc

        # Next, there could be macros that get called right inside their #ifdef, but
        # without the semi-colon.
        if cur.strip() == code['name'].strip():
            self.debug.print('FOUND MACROCALL (without brackets): %s' % (cur))
            new_block(cur, block_type.macrocall, [cur], code)
            return '',loc

        return cur, loc


class Frontend:
    ''' The C Frontend implementation.
    '''
    KNOWN_MACROS = []

    def __init__(self, project_quirks, debug):
        self.op = []
        self.debug = debug
        self.project_quirks = project_quirks

        self.c_expr_parsers = [
                CompositeParser(project_quirks, debug),
                AssignParser(project_quirks, debug),
                DeclParser(DeclParser.TYPEOF_RE, block_type.decl,
                           project_quirks, debug),
                DeclParser(DeclParser.TYPEDEF_FN_RE, block_type.decl,
                           project_quirks, debug),
                DeclParser(DeclParser.FNDECL_RE, block_type.fndecl,
                           project_quirks, debug),
                FuncParser(project_quirks, debug),
                DeclParser(DeclParser.DECL_RE, block_type.decl, project_quirks,
                           debug),
                MacroParser(project_quirks, debug)]


    def remove_extern_c(self):
        ''' Process extern "C"/"C++" block nesting.

        The extern "C" nesting does not add much value so it's safe to almost always
        drop it.  Also drop extern "C++"
        '''
        new_op = []
        nesting = 0
        extern_nesting = 0
        for l in self.op:
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
        self.op = new_op
        new_op = []
        nesting = 0
        extern_nesting = 0
        in_cpp = False
        for l in self.op:
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

        self.op = new_op


    def remove_comments(self, op):
        ''' Remove comments.

        Return OP by removing all comments from it.
        '''
        self.debug.print('REMOVE COMMENTS')

        sep='\n'
        opstr = sep.join(op)
        opstr = re.sub(r'/\*.*?\*/', r'', opstr, flags=re.MULTILINE | re.DOTALL)
        opstr = re.sub(r'\\\n', r' ', opstr, flags=re.MULTILINE | re.DOTALL)
        new_op = list(filter(None, opstr.split(sep)))

        return new_op


    def normalize_condition(self, name):
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


    def parse_preprocessor(self, loc, code, start = ''):
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
        cur = self.op[loc]
        loc = loc + 1
        endblock = False

        self.debug.print('PARSE_MACRO: %s' % cur)

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
                self.KNOWN_MACROS.append(m.group(1))

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
            rem = self.normalize_condition(rem)
            ifdef = new_block(rem, block_type.macro_cond, [], code)
            ifdef['headcond'] = ifdef
            ifdef['start'] = start
            loc = self.parse_line(loc, ifdef, start)

        # End the previous #if/#elif and begin a new block.
        elif cur.find('elif') == 0 and code['parent']:
            rem = self.normalize_condition(re.sub(r'(elif|defined)', r'', cur).strip())
            # The #else and #elif blocks should go into the current block's parent.
            ifdef = new_block(rem, block_type.macro_cond, [], code['parent'])
            ifdef['headcond'] = code['headcond']
            loc = self.parse_line(loc, ifdef, code['headcond']['start'])
            endblock = True

        # End the previous #if/#elif and begin a new block.
        elif cur.find('else') == 0 and code['parent']:
            name = self.normalize_condition('!(' + code['name'] + ')')
            ifdef = new_block(name, block_type.macro_cond, [], code['parent'],
                              block_flags.else_block)
            ifdef['headcond'] = code['headcond']
            loc = self.parse_line(loc, ifdef, code['headcond']['start'])
            endblock = True

        elif cur.find('endif') == 0 and code['parent']:
            # Insert an empty else block if there isn't one.
            if code['flags'] != block_flags.else_block:
                name = self.normalize_condition('!(' + code['name'] + ')')
                ifdef = new_block(name, block_type.macro_cond, [], code['parent'],
                                  block_flags.else_block)
                ifdef['headcond'] = code['headcond']
                loc = self.parse_line(loc - 1, ifdef, code['headcond']['start'])
            endblock = True

        return (loc, endblock)


    def parse_c_expr(self, cur, loc, code):
        ''' Parse a C expression.

        CUR is the string to be parsed, which continues to grow until a match is
        found.  OP is the string array and LOC is the first unread location in the
        string array.  CODE is the block in which any identified expressions should
        be added.
        '''
        self.debug.print('PARSING: %s' % cur)

        for p in self.c_expr_parsers:
            cur, loc = p.parse_line(cur, self.op, loc, code, self.KNOWN_MACROS)
            if not cur:
                break

        return cur, loc


    def expand_problematic_macros(self, cur):
        ''' Replace problem macros with their substitutes in CUR.
        '''
        for p in self.project_quirks.MACRO_QUIRKS:
            cur = re.sub(p['orig'], p['sub'], cur)

        return cur


    def parse_line(self, loc, code, start = ''):
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

        while loc < len(self.op):
            nextline = self.op[loc]

            # Macros.
            if nextline[0] == '#':
                (loc, endblock) = self.parse_preprocessor(loc, code, cur)
                if endblock:
                    endblock_loc = loc
            # Rest of C Code.
            else:
                cur = cur + ' ' + nextline
                cur = self.expand_problematic_macros(cur).strip()
                cur, loc = self.parse_c_expr(cur, loc + 1, code)

            if endblock and not cur:
                # If we are returning from the first #if block, we want to proceed
                # beyond the current block, not repeat it for any preceding blocks.
                if code['headcond'] == code:
                    return loc
                else:
                    return endblock_loc

        return loc

    def drop_empty_blocks(self, tree):
        ''' Drop empty macro conditional blocks.
        '''
        newcontents = []

        for x in tree['contents']:
            if x['type'] != block_type.macro_cond or len(x['contents']) > 0:
                newcontents.append(x)

        for t in newcontents:
            if t['type'] == block_type.macro_cond:
                self.drop_empty_blocks(t)

        tree['contents'] = newcontents


    def consolidate_tree_blocks(self, tree):
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
            self.consolidate_tree_blocks(b)
            newcontents.append(b)

        newcontents.extend([x for x in tree['contents'] \
                            if x['type'] != block_type.macro_cond])

        tree['contents'] = newcontents


    def compact_tree(self, tree):
        ''' Try to reduce the tree to its minimal form.

        A source code tree in its simplest form may have a lot of duplicated
        information that may be difficult to compare and come up with a minimal
        difference.
        '''

        # First, drop all empty blocks.
        self.drop_empty_blocks(tree)

        # Macro conditions that nest the entire file aren't very interesting.  This
        # should take care of the header guards.
        if tree['type'] == block_type.file \
                and len(tree['contents']) == 1 \
                and tree['contents'][0]['type'] == block_type.macro_cond:
            tree['contents'] = tree['contents'][0]['contents']

        # Finally consolidate all macro conditional blocks.
        self.consolidate_tree_blocks(tree)


    def parse(self, op):
        ''' File parser.

        Parse the input array of lines OP and generate a tree structure to
        represent the file.  This tree structure is then used for comparison between
        the old and new file.
        '''
        self.KNOWN_MACROS = []
        tree = new_block('', block_type.file, [], None)
        self.op = self.remove_comments(op)
        self.remove_extern_c()
        self.op = [re.sub(r'#\s+', '#', x) for x in self.op]
        self.parse_line(0, tree)
        self.compact_tree(tree)
        self.dump_tree(tree, 0)

        return tree


    def print_change(self, tree, action, prologue = ''):
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
                self.print_change(t, action, prologue)
            else:
                print('\t%s(%s): %s.' % (prologue, t['name'], action))


    def compare_trees(self, left, right, prologue = ''):
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
                        self.compare_trees(cl, cr, prologue)
                        found = True
                        break
                if not found:
                    self.print_change(cl, cl['actions']['del'], prologue)

            # ... and vice versa.  This time we only need to look at unmatched
            # contents.
            for cr in right['contents']:
                if not cr['matched']:
                    self.print_change(cr, cr['actions']['new'], prologue)
        else:
            if left['contents'] != right['contents']:
                self.print_change(left, left['actions']['mod'], prologue)


    def dump_tree(self, tree, indent):
        ''' Print the entire tree.
        '''
        if not self.debug.debug:
            return

        if tree['type'] == block_type.macro_cond or tree['type'] == block_type.file:
            print('%sScope: %s' % (' ' * indent, tree['name']))
            for c in tree['contents']:
                self.dump_tree(c, indent + 4)
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


    def compare(self, oldfile, newfile):
        ''' Entry point for the C backend.

        Parse the two files into trees and compare them.  Print the result of the
        comparison in the ChangeLog-like format.
        '''
        self.debug.print('LEFT TREE')
        self.debug.print('-' * 80)
        left = self.parse(oldfile)

        self.debug.print('RIGHT TREE')
        self.debug.print('-' * 80)
        right = self.parse(newfile)

        self.compare_trees(left, right)
