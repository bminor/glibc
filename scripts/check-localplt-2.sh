#! /bin/sh

# This shell script performs a fine-grained test for unwanted calls
# through the PLT.  It expects its first command line argument to
# be a specification of expected results, in the same format that
# check-localplt.awk uses, and the rest of the arguments to be _pic.a
# archives which should be checked.  AWK should be set in the
# environment.

set -e
if [ -n "$BASH_VERSION" ]; then
    set -o pipefail
fi

LC_ALL=C
export LC_ALL

all_expectations="$(mktemp)"
unsorted_output="$(mktemp)"
trap "rm -f '$all_expectations' '$unsorted_output'" 0

# Preprocess the expected PLT calls.
while [ x"$1" != x-- ]; do
    grep -Ev '^($|#)' "$1" >> "$all_expectations"
    shift
done
shift

for lib in "$@"; do
    readelf -WSrs "$lib" | tr -s ' 	' ' ' | "${AWK-awk}" '
BEGIN {
  # Whitelist of relocation types that are allowed to appear in a text
  # section, regardless of the name of the symbol.  Since the ELF R_*
  # constants already contain an architecture label, we can use just
  # one big whitelist and each architecture will only notice the
  # values that are relevant to it.
  #
  # For most architectures, the relocation types that are OK are those
  # used for a reference, from within a shared object, to a symbol
  # that was visible as external, but hidden, when the object file was
  # compiled.  There are usually at least three, one for functions,
  # one for ordinary data, and one for thread-local data.
  #
  # Please keep this list in alphabetical order.

  ok_relocs["R_X86_64_GOTPC32"] = 1;
  ok_relocs["R_X86_64_GOTPCREL"] = 1;
  ok_relocs["R_X86_64_GOTPCRELX"] = 1;
  ok_relocs["R_X86_64_GOTTPOFF"] = 1;
  ok_relocs["R_X86_64_PC32"] = 1;
  ok_relocs["R_X86_64_REX_GOTPCRELX"] = 1;

  # The state machine is reset every time we see a "File:" line, but
  # set it up here anyway as a backstop.
  in_section_headers = 0;
  in_text_relocs = 0;
  in_symbol_table = 0;
  delete text_sections;
}
$1 == "File:" {
   fname = $0;
   sub(/^File: */, "", fname);
   sub(/\(/, " ", fname);
   sub(/\)/, "", fname);

   in_section_headers = 0;
   in_text_relocs = 0;
   in_symbol_table = 0;
   delete text_sections;
   next;
}
$0 == "" {
  in_text_relocs = 0;
  in_section_headers = 0;
  in_symbol_table = 0;
  next;
}

# We only care about relocations against code, but there may be a lot of
# code sections with weird names, so we parse the section headers to
# find them all.  This is trickier than it ought to be because readelf -S
# output is not precisely space-separated columns.
# We rely on "readelf -WSrs" to print the section headers first and the
# relocation entries second.
$0 == "Section Headers:" { in_section_headers = 1; delete text_sections; next; }
$0 == "Key to Flags:"    { in_section_headers = 0; next; }
in_section_headers {
  if (/ PROGBITS / && / AX / && !/\[Nr\]/) {
    sub(/^ *\[[ 0-9]*\] */, "");
    text_sections[$1] = 1;
  }
  next;
}

/^Relocation section '\''/ {
  section = $3
  gsub(/'\''/, "", section)
  sub(/^\.rela?/, "", section)
  in_text_relocs = (section in text_sections);
  next;
}

# Relocation section dumps _are_ space-separated columns, or close enough
# for what we need.  Print the relocation type and the symbol name for
# each relocation that addresses a symbol.
in_text_relocs && $1 ~ /^[0-9a-f]/ && $5 !~ /^\./ && !($3 in ok_relocs) {
  print fname " " $3 " " $5
}

# Also print out all of the symbols that are defined by this library.
# Cross-library references have to go through the PLT regardless.
/^Symbol table '\''/ {
  in_symbol_table = 1;
  next;
}
in_symbol_table && $7 != "UND" \
  && ($5 == "GLOBAL" || $5 == "WEAK") \
  && ($4 != "NOTYPE" && $4 != "FILE" && $4 != "SECTION") \
{
  print fname " _DEFINITION_ " $8
}
'
done | ${AWK-awk} '
FILENAME != "-" {
  # Note: unlike check-localplt.awk, this program ignores +/? and relocation
  # type annotations in the whitelist file.
  # Comments were already stripped above.
  library = $1;
  symbol = $2;
  sub(/:$/, "", library);
  sub(/\.so$/, "", library);
  ok_symbols[library,symbol] = 1;
}
FILENAME == "-" {
  library = $1;
  symbol = $4;
  sub(/^.*\//, "", library);
  sub(/\.so$/, "", library);
  sub(/\.a$/,  "", library);
  sub(/_pic$/, "", library);
  sub(/@.*$/,  "", symbol);

  if ($3 == "_DEFINITION_") {
    defined_syms[library,symbol] = 1;
  } else {
    if (!((library,symbol) in ok_symbols) && !(("*",symbol) in ok_symbols)) {
      if ((library,symbol) in maybe_bad_syms) {
        maybe_bad_syms[library,symbol] = maybe_bad_syms[library,symbol] "\n" $0;
      } else {
        maybe_bad_syms[library,symbol] = $0;
      }
    }
  }
}
END {
  for (libsym in maybe_bad_syms) {
    if (libsym in defined_syms) {
      print maybe_bad_syms[libsym];
      result = 1;
    }
  }
}
' "$all_expectations" - > "$unsorted_output"

if [ -s "$unsorted_output" ]; then
    echo "*** Undesirable relocations:"
    sed -e 's:^[^ ]*/::' < "$unsorted_output" | sort -u
    exit 1
else
    exit 0
fi
