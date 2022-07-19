#!/usr/bin/python3
# ELF support functionality for Python.
# Copyright (C) 2022 Free Software Foundation, Inc.
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

"""Basic ELF parser.

Use Image.readfile(path) to read an ELF file into memory and begin
parsing it.

"""

import collections
import enum
import struct

if not hasattr(enum, 'IntFlag'):
    import sys
    sys.stdout.write(
        'warning: glibcelf.py needs Python 3.6 for enum support\n')
    sys.exit(77)

class _OpenIntEnum(enum.IntEnum):
    """Integer enumeration that supports arbitrary int values."""
    @classmethod
    def _missing_(cls, value):
        # See enum.IntFlag._create_pseudo_member_.  This allows
        # creating of enum constants with arbitrary integer values.
        pseudo_member = int.__new__(cls, value)
        pseudo_member._name_ = None
        pseudo_member._value_ = value
        return pseudo_member

    def __repr__(self):
        name = self._name_
        if name is not None:
            # The names have prefixes like SHT_, implying their type.
            return name
        return '{}({})'.format(self.__class__.__name__, self._value_)

    def __str__(self):
        name = self._name_
        if name is not None:
            return name
        return str(self._value_)

class ElfClass(_OpenIntEnum):
    """ELF word size.  Type of EI_CLASS values."""
    ELFCLASSNONE = 0
    ELFCLASS32 = 1
    ELFCLASS64 = 2

class ElfData(_OpenIntEnum):
    """ELF endianess.  Type of EI_DATA values."""
    ELFDATANONE = 0
    ELFDATA2LSB = 1
    ELFDATA2MSB = 2

class Machine(_OpenIntEnum):
    """ELF machine type.  Type of values in Ehdr.e_machine field."""
    EM_NONE = 0
    EM_M32 = 1
    EM_SPARC = 2
    EM_386 = 3
    EM_68K = 4
    EM_88K = 5
    EM_IAMCU = 6
    EM_860 = 7
    EM_MIPS = 8
    EM_S370 = 9
    EM_MIPS_RS3_LE = 10
    EM_PARISC = 15
    EM_VPP500 = 17
    EM_SPARC32PLUS = 18
    EM_960 = 19
    EM_PPC = 20
    EM_PPC64 = 21
    EM_S390 = 22
    EM_SPU = 23
    EM_V800 = 36
    EM_FR20 = 37
    EM_RH32 = 38
    EM_RCE = 39
    EM_ARM = 40
    EM_FAKE_ALPHA = 41
    EM_SH = 42
    EM_SPARCV9 = 43
    EM_TRICORE = 44
    EM_ARC = 45
    EM_H8_300 = 46
    EM_H8_300H = 47
    EM_H8S = 48
    EM_H8_500 = 49
    EM_IA_64 = 50
    EM_MIPS_X = 51
    EM_COLDFIRE = 52
    EM_68HC12 = 53
    EM_MMA = 54
    EM_PCP = 55
    EM_NCPU = 56
    EM_NDR1 = 57
    EM_STARCORE = 58
    EM_ME16 = 59
    EM_ST100 = 60
    EM_TINYJ = 61
    EM_X86_64 = 62
    EM_PDSP = 63
    EM_PDP10 = 64
    EM_PDP11 = 65
    EM_FX66 = 66
    EM_ST9PLUS = 67
    EM_ST7 = 68
    EM_68HC16 = 69
    EM_68HC11 = 70
    EM_68HC08 = 71
    EM_68HC05 = 72
    EM_SVX = 73
    EM_ST19 = 74
    EM_VAX = 75
    EM_CRIS = 76
    EM_JAVELIN = 77
    EM_FIREPATH = 78
    EM_ZSP = 79
    EM_MMIX = 80
    EM_HUANY = 81
    EM_PRISM = 82
    EM_AVR = 83
    EM_FR30 = 84
    EM_D10V = 85
    EM_D30V = 86
    EM_V850 = 87
    EM_M32R = 88
    EM_MN10300 = 89
    EM_MN10200 = 90
    EM_PJ = 91
    EM_OPENRISC = 92
    EM_ARC_COMPACT = 93
    EM_XTENSA = 94
    EM_VIDEOCORE = 95
    EM_TMM_GPP = 96
    EM_NS32K = 97
    EM_TPC = 98
    EM_SNP1K = 99
    EM_ST200 = 100
    EM_IP2K = 101
    EM_MAX = 102
    EM_CR = 103
    EM_F2MC16 = 104
    EM_MSP430 = 105
    EM_BLACKFIN = 106
    EM_SE_C33 = 107
    EM_SEP = 108
    EM_ARCA = 109
    EM_UNICORE = 110
    EM_EXCESS = 111
    EM_DXP = 112
    EM_ALTERA_NIOS2 = 113
    EM_CRX = 114
    EM_XGATE = 115
    EM_C166 = 116
    EM_M16C = 117
    EM_DSPIC30F = 118
    EM_CE = 119
    EM_M32C = 120
    EM_TSK3000 = 131
    EM_RS08 = 132
    EM_SHARC = 133
    EM_ECOG2 = 134
    EM_SCORE7 = 135
    EM_DSP24 = 136
    EM_VIDEOCORE3 = 137
    EM_LATTICEMICO32 = 138
    EM_SE_C17 = 139
    EM_TI_C6000 = 140
    EM_TI_C2000 = 141
    EM_TI_C5500 = 142
    EM_TI_ARP32 = 143
    EM_TI_PRU = 144
    EM_MMDSP_PLUS = 160
    EM_CYPRESS_M8C = 161
    EM_R32C = 162
    EM_TRIMEDIA = 163
    EM_QDSP6 = 164
    EM_8051 = 165
    EM_STXP7X = 166
    EM_NDS32 = 167
    EM_ECOG1X = 168
    EM_MAXQ30 = 169
    EM_XIMO16 = 170
    EM_MANIK = 171
    EM_CRAYNV2 = 172
    EM_RX = 173
    EM_METAG = 174
    EM_MCST_ELBRUS = 175
    EM_ECOG16 = 176
    EM_CR16 = 177
    EM_ETPU = 178
    EM_SLE9X = 179
    EM_L10M = 180
    EM_K10M = 181
    EM_AARCH64 = 183
    EM_AVR32 = 185
    EM_STM8 = 186
    EM_TILE64 = 187
    EM_TILEPRO = 188
    EM_MICROBLAZE = 189
    EM_CUDA = 190
    EM_TILEGX = 191
    EM_CLOUDSHIELD = 192
    EM_COREA_1ST = 193
    EM_COREA_2ND = 194
    EM_ARCV2 = 195
    EM_OPEN8 = 196
    EM_RL78 = 197
    EM_VIDEOCORE5 = 198
    EM_78KOR = 199
    EM_56800EX = 200
    EM_BA1 = 201
    EM_BA2 = 202
    EM_XCORE = 203
    EM_MCHP_PIC = 204
    EM_INTELGT = 205
    EM_KM32 = 210
    EM_KMX32 = 211
    EM_EMX16 = 212
    EM_EMX8 = 213
    EM_KVARC = 214
    EM_CDP = 215
    EM_COGE = 216
    EM_COOL = 217
    EM_NORC = 218
    EM_CSR_KALIMBA = 219
    EM_Z80 = 220
    EM_VISIUM = 221
    EM_FT32 = 222
    EM_MOXIE = 223
    EM_AMDGPU = 224
    EM_RISCV = 243
    EM_BPF = 247
    EM_CSKY = 252
    EM_LOONGARCH = 258
    EM_NUM = 259
    EM_ALPHA = 0x9026

class Et(_OpenIntEnum):
    """ELF file type.  Type of ET_* values and the Ehdr.e_type field."""
    ET_NONE = 0
    ET_REL = 1
    ET_EXEC = 2
    ET_DYN = 3
    ET_CORE = 4

class Shn(_OpenIntEnum):
    """ELF reserved section indices."""
    SHN_UNDEF = 0
    SHN_BEFORE = 0xff00
    SHN_AFTER = 0xff01
    SHN_ABS = 0xfff1
    SHN_COMMON = 0xfff2
    SHN_XINDEX = 0xffff

class ShnMIPS(enum.Enum):
    """Supplemental SHN_* constants for EM_MIPS."""
    SHN_MIPS_ACOMMON = 0xff00
    SHN_MIPS_TEXT = 0xff01
    SHN_MIPS_DATA = 0xff02
    SHN_MIPS_SCOMMON = 0xff03
    SHN_MIPS_SUNDEFINED = 0xff04

class ShnPARISC(enum.Enum):
    """Supplemental SHN_* constants for EM_PARISC."""
    SHN_PARISC_ANSI_COMMON = 0xff00
    SHN_PARISC_HUGE_COMMON = 0xff01

class Sht(_OpenIntEnum):
    """ELF section types.  Type of SHT_* values."""
    SHT_NULL = 0
    SHT_PROGBITS = 1
    SHT_SYMTAB = 2
    SHT_STRTAB = 3
    SHT_RELA = 4
    SHT_HASH = 5
    SHT_DYNAMIC = 6
    SHT_NOTE = 7
    SHT_NOBITS = 8
    SHT_REL = 9
    SHT_SHLIB = 10
    SHT_DYNSYM = 11
    SHT_INIT_ARRAY = 14
    SHT_FINI_ARRAY = 15
    SHT_PREINIT_ARRAY = 16
    SHT_GROUP = 17
    SHT_SYMTAB_SHNDX = 18
    SHT_RELR = 19
    SHT_GNU_ATTRIBUTES = 0x6ffffff5
    SHT_GNU_HASH = 0x6ffffff6
    SHT_GNU_LIBLIST = 0x6ffffff7
    SHT_CHECKSUM = 0x6ffffff8
    SHT_SUNW_move = 0x6ffffffa
    SHT_SUNW_COMDAT = 0x6ffffffb
    SHT_SUNW_syminfo = 0x6ffffffc
    SHT_GNU_verdef = 0x6ffffffd
    SHT_GNU_verneed = 0x6ffffffe
    SHT_GNU_versym = 0x6fffffff

class ShtALPHA(enum.Enum):
    """Supplemental SHT_* constants for EM_ALPHA."""
    SHT_ALPHA_DEBUG = 0x70000001
    SHT_ALPHA_REGINFO = 0x70000002

class ShtARM(enum.Enum):
    """Supplemental SHT_* constants for EM_ARM."""
    SHT_ARM_EXIDX = 0x70000001
    SHT_ARM_PREEMPTMAP = 0x70000002
    SHT_ARM_ATTRIBUTES = 0x70000003

class ShtCSKY(enum.Enum):
    """Supplemental SHT_* constants for EM_CSKY."""
    SHT_CSKY_ATTRIBUTES = 0x70000001

class ShtIA_64(enum.Enum):
    """Supplemental SHT_* constants for EM_IA_64."""
    SHT_IA_64_EXT = 0x70000000
    SHT_IA_64_UNWIND = 0x70000001

class ShtMIPS(enum.Enum):
    """Supplemental SHT_* constants for EM_MIPS."""
    SHT_MIPS_LIBLIST = 0x70000000
    SHT_MIPS_MSYM = 0x70000001
    SHT_MIPS_CONFLICT = 0x70000002
    SHT_MIPS_GPTAB = 0x70000003
    SHT_MIPS_UCODE = 0x70000004
    SHT_MIPS_DEBUG = 0x70000005
    SHT_MIPS_REGINFO = 0x70000006
    SHT_MIPS_PACKAGE = 0x70000007
    SHT_MIPS_PACKSYM = 0x70000008
    SHT_MIPS_RELD = 0x70000009
    SHT_MIPS_IFACE = 0x7000000b
    SHT_MIPS_CONTENT = 0x7000000c
    SHT_MIPS_OPTIONS = 0x7000000d
    SHT_MIPS_SHDR = 0x70000010
    SHT_MIPS_FDESC = 0x70000011
    SHT_MIPS_EXTSYM = 0x70000012
    SHT_MIPS_DENSE = 0x70000013
    SHT_MIPS_PDESC = 0x70000014
    SHT_MIPS_LOCSYM = 0x70000015
    SHT_MIPS_AUXSYM = 0x70000016
    SHT_MIPS_OPTSYM = 0x70000017
    SHT_MIPS_LOCSTR = 0x70000018
    SHT_MIPS_LINE = 0x70000019
    SHT_MIPS_RFDESC = 0x7000001a
    SHT_MIPS_DELTASYM = 0x7000001b
    SHT_MIPS_DELTAINST = 0x7000001c
    SHT_MIPS_DELTACLASS = 0x7000001d
    SHT_MIPS_DWARF = 0x7000001e
    SHT_MIPS_DELTADECL = 0x7000001f
    SHT_MIPS_SYMBOL_LIB = 0x70000020
    SHT_MIPS_EVENTS = 0x70000021
    SHT_MIPS_TRANSLATE = 0x70000022
    SHT_MIPS_PIXIE = 0x70000023
    SHT_MIPS_XLATE = 0x70000024
    SHT_MIPS_XLATE_DEBUG = 0x70000025
    SHT_MIPS_WHIRL = 0x70000026
    SHT_MIPS_EH_REGION = 0x70000027
    SHT_MIPS_XLATE_OLD = 0x70000028
    SHT_MIPS_PDR_EXCEPTION = 0x70000029
    SHT_MIPS_XHASH = 0x7000002b

class ShtPARISC(enum.Enum):
    """Supplemental SHT_* constants for EM_PARISC."""
    SHT_PARISC_EXT = 0x70000000
    SHT_PARISC_UNWIND = 0x70000001
    SHT_PARISC_DOC = 0x70000002

class ShtRISCV(enum.Enum):
    """Supplemental SHT_* constants for EM_RISCV."""
    SHT_RISCV_ATTRIBUTES = 0x70000003

class Pf(enum.IntFlag):
    """Program header flags.  Type of Phdr.p_flags values."""
    PF_X = 1
    PF_W = 2
    PF_R = 4

class PfARM(enum.IntFlag):
    """Supplemental PF_* flags for EM_ARM."""
    PF_ARM_SB = 0x10000000
    PF_ARM_PI = 0x20000000
    PF_ARM_ABS = 0x40000000

class PfPARISC(enum.IntFlag):
    """Supplemental PF_* flags for EM_PARISC."""
    PF_HP_PAGE_SIZE = 0x00100000
    PF_HP_FAR_SHARED = 0x00200000
    PF_HP_NEAR_SHARED = 0x00400000
    PF_HP_CODE = 0x01000000
    PF_HP_MODIFY = 0x02000000
    PF_HP_LAZYSWAP = 0x04000000
    PF_HP_SBP = 0x08000000

class PfIA_64(enum.IntFlag):
    """Supplemental PF_* flags for EM_IA_64."""
    PF_IA_64_NORECOV = 0x80000000

class PfMIPS(enum.IntFlag):
    """Supplemental PF_* flags for EM_MIPS."""
    PF_MIPS_LOCAL = 0x10000000

class Shf(enum.IntFlag):
    """Section flags.  Type of Shdr.sh_type values."""
    SHF_WRITE = 1 << 0
    SHF_ALLOC = 1 << 1
    SHF_EXECINSTR = 1 << 2
    SHF_MERGE = 1 << 4
    SHF_STRINGS = 1 << 5
    SHF_INFO_LINK = 1 << 6
    SHF_LINK_ORDER = 1 << 7
    SHF_OS_NONCONFORMING = 256
    SHF_GROUP = 1 << 9
    SHF_TLS = 1 << 10
    SHF_COMPRESSED = 1 << 11
    SHF_GNU_RETAIN = 1 << 21
    SHF_ORDERED = 1 << 30
    SHF_EXCLUDE = 1 << 31

class ShfALPHA(enum.IntFlag):
    """Supplemental SHF_* constants for EM_ALPHA."""
    SHF_ALPHA_GPREL = 0x10000000

class ShfARM(enum.IntFlag):
    """Supplemental SHF_* constants for EM_ARM."""
    SHF_ARM_ENTRYSECT = 0x10000000
    SHF_ARM_COMDEF = 0x80000000

class ShfIA_64(enum.IntFlag):
    """Supplemental SHF_* constants for EM_IA_64."""
    SHF_IA_64_SHORT  = 0x10000000
    SHF_IA_64_NORECOV = 0x20000000

class ShfMIPS(enum.IntFlag):
    """Supplemental SHF_* constants for EM_MIPS."""
    SHF_MIPS_GPREL = 0x10000000
    SHF_MIPS_MERGE = 0x20000000
    SHF_MIPS_ADDR = 0x40000000
    SHF_MIPS_STRINGS = 0x80000000
    SHF_MIPS_NOSTRIP = 0x08000000
    SHF_MIPS_LOCAL = 0x04000000
    SHF_MIPS_NAMES = 0x02000000
    SHF_MIPS_NODUPE = 0x01000000

class ShfPARISC(enum.IntFlag):
    """Supplemental SHF_* constants for EM_PARISC."""
    SHF_PARISC_SHORT = 0x20000000
    SHF_PARISC_HUGE = 0x40000000
    SHF_PARISC_SBP = 0x80000000

class Stb(_OpenIntEnum):
    """ELF symbol binding type."""
    STB_LOCAL = 0
    STB_GLOBAL = 1
    STB_WEAK = 2
    STB_GNU_UNIQUE = 10
    STB_MIPS_SPLIT_COMMON = 13

class Stt(_OpenIntEnum):
    """ELF symbol type."""
    STT_NOTYPE = 0
    STT_OBJECT = 1
    STT_FUNC = 2
    STT_SECTION = 3
    STT_FILE = 4
    STT_COMMON = 5
    STT_TLS = 6
    STT_GNU_IFUNC = 10

class SttARM(enum.Enum):
    """Supplemental STT_* constants for EM_ARM."""
    STT_ARM_TFUNC = 13
    STT_ARM_16BIT = 15

class SttPARISC(enum.Enum):
    """Supplemental STT_* constants for EM_PARISC."""
    STT_HP_OPAQUE = 11
    STT_HP_STUB = 12
    STT_PARISC_MILLICODE = 13

class SttSPARC(enum.Enum):
    """Supplemental STT_* constants for EM_SPARC."""
    STT_SPARC_REGISTER = 13

class SttX86_64(enum.Enum):
    """Supplemental STT_* constants for EM_X86_64."""
    SHT_X86_64_UNWIND = 0x70000001

class Pt(_OpenIntEnum):
    """ELF program header types.  Type of Phdr.p_type."""
    PT_NULL = 0
    PT_LOAD = 1
    PT_DYNAMIC = 2
    PT_INTERP = 3
    PT_NOTE = 4
    PT_SHLIB = 5
    PT_PHDR = 6
    PT_TLS = 7
    PT_NUM = 8
    PT_GNU_EH_FRAME = 0x6474e550
    PT_GNU_STACK = 0x6474e551
    PT_GNU_RELRO = 0x6474e552
    PT_GNU_PROPERTY = 0x6474e553
    PT_SUNWBSS = 0x6ffffffa
    PT_SUNWSTACK = 0x6ffffffb

class PtAARCH64(enum.Enum):
    """Supplemental PT_* constants for EM_AARCH64."""
    PT_AARCH64_MEMTAG_MTE = 0x70000002

class PtARM(enum.Enum):
    """Supplemental PT_* constants for EM_ARM."""
    PT_ARM_EXIDX = 0x70000001

class PtIA_64(enum.Enum):
    """Supplemental PT_* constants for EM_IA_64."""
    PT_IA_64_HP_OPT_ANOT = 0x60000012
    PT_IA_64_HP_HSL_ANOT = 0x60000013
    PT_IA_64_HP_STACK = 0x60000014
    PT_IA_64_ARCHEXT = 0x70000000
    PT_IA_64_UNWIND = 0x70000001

class PtMIPS(enum.Enum):
    """Supplemental PT_* constants for EM_MIPS."""
    PT_MIPS_REGINFO = 0x70000000
    PT_MIPS_RTPROC = 0x70000001
    PT_MIPS_OPTIONS = 0x70000002
    PT_MIPS_ABIFLAGS = 0x70000003

class PtPARISC(enum.Enum):
    """Supplemental PT_* constants for EM_PARISC."""
    PT_HP_TLS = 0x60000000
    PT_HP_CORE_NONE = 0x60000001
    PT_HP_CORE_VERSION = 0x60000002
    PT_HP_CORE_KERNEL = 0x60000003
    PT_HP_CORE_COMM = 0x60000004
    PT_HP_CORE_PROC = 0x60000005
    PT_HP_CORE_LOADABLE = 0x60000006
    PT_HP_CORE_STACK = 0x60000007
    PT_HP_CORE_SHM = 0x60000008
    PT_HP_CORE_MMF = 0x60000009
    PT_HP_PARALLEL = 0x60000010
    PT_HP_FASTBIND = 0x60000011
    PT_HP_OPT_ANNOT = 0x60000012
    PT_HP_HSL_ANNOT = 0x60000013
    PT_HP_STACK = 0x60000014
    PT_PARISC_ARCHEXT = 0x70000000
    PT_PARISC_UNWIND = 0x70000001

class PtRISCV(enum.Enum):
    """Supplemental PT_* constants for EM_RISCV."""
    PT_RISCV_ATTRIBUTES = 0x70000003

class Dt(_OpenIntEnum):
    """ELF dynamic segment tags.  Type of Dyn.d_val."""
    DT_NULL = 0
    DT_NEEDED = 1
    DT_PLTRELSZ = 2
    DT_PLTGOT = 3
    DT_HASH = 4
    DT_STRTAB = 5
    DT_SYMTAB = 6
    DT_RELA = 7
    DT_RELASZ = 8
    DT_RELAENT = 9
    DT_STRSZ = 10
    DT_SYMENT = 11
    DT_INIT = 12
    DT_FINI = 13
    DT_SONAME = 14
    DT_RPATH = 15
    DT_SYMBOLIC = 16
    DT_REL = 17
    DT_RELSZ = 18
    DT_RELENT = 19
    DT_PLTREL = 20
    DT_DEBUG = 21
    DT_TEXTREL = 22
    DT_JMPREL = 23
    DT_BIND_NOW = 24
    DT_INIT_ARRAY = 25
    DT_FINI_ARRAY = 26
    DT_INIT_ARRAYSZ = 27
    DT_FINI_ARRAYSZ = 28
    DT_RUNPATH = 29
    DT_FLAGS = 30
    DT_PREINIT_ARRAY = 32
    DT_PREINIT_ARRAYSZ = 33
    DT_SYMTAB_SHNDX = 34
    DT_RELRSZ = 35
    DT_RELR = 36
    DT_RELRENT = 37
    DT_GNU_PRELINKED = 0x6ffffdf5
    DT_GNU_CONFLICTSZ = 0x6ffffdf6
    DT_GNU_LIBLISTSZ = 0x6ffffdf7
    DT_CHECKSUM = 0x6ffffdf8
    DT_PLTPADSZ = 0x6ffffdf9
    DT_MOVEENT = 0x6ffffdfa
    DT_MOVESZ = 0x6ffffdfb
    DT_FEATURE_1 = 0x6ffffdfc
    DT_POSFLAG_1 = 0x6ffffdfd
    DT_SYMINSZ = 0x6ffffdfe
    DT_SYMINENT = 0x6ffffdff
    DT_GNU_HASH = 0x6ffffef5
    DT_TLSDESC_PLT = 0x6ffffef6
    DT_TLSDESC_GOT = 0x6ffffef7
    DT_GNU_CONFLICT = 0x6ffffef8
    DT_GNU_LIBLIST = 0x6ffffef9
    DT_CONFIG = 0x6ffffefa
    DT_DEPAUDIT = 0x6ffffefb
    DT_AUDIT = 0x6ffffefc
    DT_PLTPAD = 0x6ffffefd
    DT_MOVETAB = 0x6ffffefe
    DT_SYMINFO = 0x6ffffeff
    DT_VERSYM = 0x6ffffff0
    DT_RELACOUNT = 0x6ffffff9
    DT_RELCOUNT = 0x6ffffffa
    DT_FLAGS_1 = 0x6ffffffb
    DT_VERDEF = 0x6ffffffc
    DT_VERDEFNUM = 0x6ffffffd
    DT_VERNEED = 0x6ffffffe
    DT_VERNEEDNUM = 0x6fffffff
    DT_AUXILIARY = 0x7ffffffd
    DT_FILTER = 0x7fffffff

class DtAARCH64(enum.Enum):
    """Supplemental DT_* constants for EM_AARCH64."""
    DT_AARCH64_BTI_PLT = 0x70000001
    DT_AARCH64_PAC_PLT = 0x70000003
    DT_AARCH64_VARIANT_PCS = 0x70000005

class DtALPHA(enum.Enum):
    """Supplemental DT_* constants for EM_ALPHA."""
    DT_ALPHA_PLTRO = 0x70000000

class DtALTERA_NIOS2(enum.Enum):
    """Supplemental DT_* constants for EM_ALTERA_NIOS2."""
    DT_NIOS2_GP = 0x70000002

class DtIA_64(enum.Enum):
    """Supplemental DT_* constants for EM_IA_64."""
    DT_IA_64_PLT_RESERVE = 0x70000000

class DtMIPS(enum.Enum):
    """Supplemental DT_* constants for EM_MIPS."""
    DT_MIPS_RLD_VERSION = 0x70000001
    DT_MIPS_TIME_STAMP = 0x70000002
    DT_MIPS_ICHECKSUM = 0x70000003
    DT_MIPS_IVERSION = 0x70000004
    DT_MIPS_FLAGS = 0x70000005
    DT_MIPS_BASE_ADDRESS = 0x70000006
    DT_MIPS_MSYM = 0x70000007
    DT_MIPS_CONFLICT = 0x70000008
    DT_MIPS_LIBLIST = 0x70000009
    DT_MIPS_LOCAL_GOTNO = 0x7000000a
    DT_MIPS_CONFLICTNO = 0x7000000b
    DT_MIPS_LIBLISTNO = 0x70000010
    DT_MIPS_SYMTABNO = 0x70000011
    DT_MIPS_UNREFEXTNO = 0x70000012
    DT_MIPS_GOTSYM = 0x70000013
    DT_MIPS_HIPAGENO = 0x70000014
    DT_MIPS_RLD_MAP = 0x70000016
    DT_MIPS_DELTA_CLASS = 0x70000017
    DT_MIPS_DELTA_CLASS_NO = 0x70000018
    DT_MIPS_DELTA_INSTANCE = 0x70000019
    DT_MIPS_DELTA_INSTANCE_NO = 0x7000001a
    DT_MIPS_DELTA_RELOC = 0x7000001b
    DT_MIPS_DELTA_RELOC_NO = 0x7000001c
    DT_MIPS_DELTA_SYM = 0x7000001d
    DT_MIPS_DELTA_SYM_NO = 0x7000001e
    DT_MIPS_DELTA_CLASSSYM = 0x70000020
    DT_MIPS_DELTA_CLASSSYM_NO = 0x70000021
    DT_MIPS_CXX_FLAGS = 0x70000022
    DT_MIPS_PIXIE_INIT = 0x70000023
    DT_MIPS_SYMBOL_LIB = 0x70000024
    DT_MIPS_LOCALPAGE_GOTIDX = 0x70000025
    DT_MIPS_LOCAL_GOTIDX = 0x70000026
    DT_MIPS_HIDDEN_GOTIDX = 0x70000027
    DT_MIPS_PROTECTED_GOTIDX = 0x70000028
    DT_MIPS_OPTIONS = 0x70000029
    DT_MIPS_INTERFACE = 0x7000002a
    DT_MIPS_DYNSTR_ALIGN = 0x7000002b
    DT_MIPS_INTERFACE_SIZE = 0x7000002c
    DT_MIPS_RLD_TEXT_RESOLVE_ADDR = 0x7000002d
    DT_MIPS_PERF_SUFFIX = 0x7000002e
    DT_MIPS_COMPACT_SIZE = 0x7000002f
    DT_MIPS_GP_VALUE = 0x70000030
    DT_MIPS_AUX_DYNAMIC = 0x70000031
    DT_MIPS_PLTGOT = 0x70000032
    DT_MIPS_RWPLT = 0x70000034
    DT_MIPS_RLD_MAP_REL = 0x70000035
    DT_MIPS_XHASH = 0x70000036

class DtPPC(enum.Enum):
    """Supplemental DT_* constants for EM_PPC."""
    DT_PPC_GOT = 0x70000000
    DT_PPC_OPT = 0x70000001

class DtPPC64(enum.Enum):
    """Supplemental DT_* constants for EM_PPC64."""
    DT_PPC64_GLINK = 0x70000000
    DT_PPC64_OPD = 0x70000001
    DT_PPC64_OPDSZ = 0x70000002
    DT_PPC64_OPT = 0x70000003

class DtRISCV(enum.Enum):
    """Supplemental DT_* constants for EM_RISCV."""
    DT_RISCV_VARIANT_CC = 0x70000001

class DtSPARC(enum.Enum):
    """Supplemental DT_* constants for EM_SPARC."""
    DT_SPARC_REGISTER = 0x70000001

class StInfo:
    """ELF symbol binding and type.  Type of the Sym.st_info field."""
    def __init__(self, arg0, arg1=None):
        if isinstance(arg0, int) and arg1 is None:
            self.bind = Stb(arg0 >> 4)
            self.type = Stt(arg0 & 15)
        else:
            self.bind = Stb(arg0)
            self.type = Stt(arg1)

    def value(self):
        """Returns the raw value for the bind/type combination."""
        return (self.bind.value() << 4) | (self.type.value())

# Type in an ELF file.  Used for deserialization.
_Layout = collections.namedtuple('_Layout', 'unpack size')

def _define_layouts(baseclass: type, layout32: str, layout64: str,
                    types=None, fields32=None):
    """Assign variants dict to baseclass.

    The variants dict is indexed by (ElfClass, ElfData) pairs, and its
    values are _Layout instances.

    """
    struct32 = struct.Struct(layout32)
    struct64 = struct.Struct(layout64)

    # Check that the struct formats yield the right number of components.
    for s in (struct32, struct64):
        example = s.unpack(b' ' * s.size)
        if len(example) != len(baseclass._fields):
            raise ValueError('{!r} yields wrong field count: {} != {}'.format(
                s.format, len(example),  len(baseclass._fields)))

    # Check that field names in types are correct.
    if types is None:
        types = ()
    for n in types:
        if n not in baseclass._fields:
            raise ValueError('{} does not have field {!r}'.format(
                baseclass.__name__, n))

    if fields32 is not None \
       and set(fields32) != set(baseclass._fields):
        raise ValueError('{!r} is not a permutation of the fields {!r}'.format(
            fields32, baseclass._fields))

    def unique_name(name, used_names = (set((baseclass.__name__,))
                                        | set(baseclass._fields)
                                        | {n.__name__
                                           for n in (types or {}).values()})):
        """Find a name that is not used for a class or field name."""
        candidate = name
        n = 0
        while candidate in used_names:
            n += 1
            candidate = '{}{}'.format(name, n)
        used_names.add(candidate)
        return candidate

    blob_name = unique_name('blob')
    struct_unpack_name = unique_name('struct_unpack')
    comps_name = unique_name('comps')

    layouts = {}
    for (bits, elfclass, layout, fields) in (
            (32, ElfClass.ELFCLASS32, layout32, fields32),
            (64, ElfClass.ELFCLASS64, layout64, None),
    ):
        for (elfdata, structprefix, funcsuffix) in (
                (ElfData.ELFDATA2LSB, '<', 'LE'),
                (ElfData.ELFDATA2MSB, '>', 'BE'),
        ):
            env = {
                baseclass.__name__: baseclass,
                struct_unpack_name: struct.unpack,
            }

            # Add the type converters.
            if types:
                for cls in types.values():
                    env[cls.__name__] = cls

            funcname = ''.join(
                ('unpack_', baseclass.__name__, str(bits), funcsuffix))

            code = '''
def {funcname}({blob_name}):
'''.format(funcname=funcname, blob_name=blob_name)

            indent = ' ' * 4
            unpack_call = '{}({!r}, {})'.format(
                struct_unpack_name, structprefix + layout, blob_name)
            field_names = ', '.join(baseclass._fields)
            if types is None and fields is None:
                code += '{}return {}({})\n'.format(
                    indent, baseclass.__name__, unpack_call)
            else:
                # Destructuring tuple assignment.
                if fields is None:
                    code += '{}{} = {}\n'.format(
                        indent, field_names, unpack_call)
                else:
                    # Use custom field order.
                    code += '{}{} = {}\n'.format(
                        indent, ', '.join(fields), unpack_call)

                # Perform the type conversions.
                for n in baseclass._fields:
                    if n in types:
                        code += '{}{} = {}({})\n'.format(
                            indent, n, types[n].__name__, n)
                # Create the named tuple.
                code += '{}return {}({})\n'.format(
                    indent, baseclass.__name__, field_names)

            exec(code, env)
            layouts[(elfclass, elfdata)] = _Layout(
                env[funcname], struct.calcsize(layout))
    baseclass.layouts = layouts


# Corresponds to EI_* indices into Elf*_Ehdr.e_indent.
class Ident(collections.namedtuple('Ident',
    'ei_mag ei_class ei_data ei_version ei_osabi ei_abiversion ei_pad')):

    def __new__(cls, *args):
        """Construct an object from a blob or its constituent fields."""
        if len(args) == 1:
            return cls.unpack(args[0])
        return cls.__base__.__new__(cls, *args)

    @staticmethod
    def unpack(blob: memoryview) -> 'Ident':
        """Parse raws data into a tuple."""
        ei_mag, ei_class, ei_data, ei_version, ei_osabi, ei_abiversion, \
            ei_pad = struct.unpack('4s5B7s', blob)
        return Ident(ei_mag, ElfClass(ei_class), ElfData(ei_data),
                     ei_version, ei_osabi, ei_abiversion, ei_pad)
    size = 16

# Corresponds to Elf32_Ehdr and Elf64_Ehdr.
Ehdr = collections.namedtuple('Ehdr',
   'e_ident e_type e_machine e_version e_entry e_phoff e_shoff e_flags'
    + ' e_ehsize e_phentsize e_phnum e_shentsize e_shnum e_shstrndx')
_define_layouts(Ehdr,
                layout32='16s2H5I6H',
                layout64='16s2HI3QI6H',
                types=dict(e_ident=Ident,
                           e_machine=Machine,
                           e_type=Et,
                           e_shstrndx=Shn))

# Corresponds to Elf32_Phdr and Elf64_Pdhr.  Order follows the latter.
Phdr = collections.namedtuple('Phdr',
    'p_type p_flags p_offset p_vaddr p_paddr p_filesz p_memsz p_align')
_define_layouts(Phdr,
                layout32='8I',
                fields32=('p_type', 'p_offset', 'p_vaddr', 'p_paddr',
                          'p_filesz', 'p_memsz', 'p_flags', 'p_align'),
                layout64='2I6Q',
            types=dict(p_type=Pt, p_flags=Pf))


# Corresponds to Elf32_Shdr and Elf64_Shdr.
class Shdr(collections.namedtuple('Shdr',
    'sh_name sh_type sh_flags sh_addr sh_offset sh_size sh_link sh_info'
    + ' sh_addralign sh_entsize')):
    def resolve(self, strtab: 'StringTable') -> 'Shdr':
        """Resolve sh_name using a string table."""
        return self.__class__(strtab.get(self[0]), *self[1:])
_define_layouts(Shdr,
                layout32='10I',
                layout64='2I4Q2I2Q',
                types=dict(sh_type=Sht,
                           sh_flags=Shf,
                           sh_link=Shn))

# Corresponds to Elf32_Dyn and Elf64_Dyn.  The nesting through the
# d_un union is skipped, and d_ptr is missing (its representation in
# Python would be identical to d_val).
Dyn = collections.namedtuple('Dyn', 'd_tag d_val')
_define_layouts(Dyn,
                layout32='2i',
                layout64='2q',
                types=dict(d_tag=Dt))

# Corresponds to Elf32_Sym and Elf64_Sym.
class Sym(collections.namedtuple('Sym',
    'st_name st_info st_other st_shndx st_value st_size')):
    def resolve(self, strtab: 'StringTable') -> 'Sym':
        """Resolve st_name using a string table."""
        return self.__class__(strtab.get(self[0]), *self[1:])
_define_layouts(Sym,
                layout32='3I2BH',
                layout64='I2BH2Q',
                fields32=('st_name', 'st_value', 'st_size', 'st_info',
                          'st_other', 'st_shndx'),
                types=dict(st_shndx=Shn,
                           st_info=StInfo))

# Corresponds to Elf32_Rel and Elf64_Rel.
Rel = collections.namedtuple('Rel', 'r_offset r_info')
_define_layouts(Rel,
                layout32='2I',
                layout64='2Q')

# Corresponds to Elf32_Rel and Elf64_Rel.
Rela = collections.namedtuple('Rela', 'r_offset r_info r_addend')
_define_layouts(Rela,
                layout32='3I',
                layout64='3Q')

class StringTable:
    """ELF string table."""
    def __init__(self, blob):
        """Create a new string table backed by the data in the blob.

        blob: a memoryview-like object

        """
        self.blob = blob

    def get(self, index) -> bytes:
        """Returns the null-terminated byte string at the index."""
        blob = self.blob
        endindex = index
        while True:
            if blob[endindex] == 0:
                return bytes(blob[index:endindex])
            endindex += 1

class Image:
    """ELF image parser."""
    def __init__(self, image):
        """Create an ELF image from binary image data.

        image: a memoryview-like object that supports efficient range
        subscripting.

        """
        self.image = image
        ident = self.read(Ident, 0)
        classdata = (ident.ei_class, ident.ei_data)
        # Set self.Ehdr etc. to the subtypes with the right parsers.
        for typ in (Ehdr, Phdr, Shdr, Dyn, Sym, Rel, Rela):
            setattr(self, typ.__name__, typ.layouts.get(classdata, None))

        if self.Ehdr is not None:
            self.ehdr = self.read(self.Ehdr, 0)
            self._shdr_num = self._compute_shdr_num()
        else:
            self.ehdr = None
            self._shdr_num = 0

        self._section = {}
        self._stringtab = {}

        if self._shdr_num > 0:
            self._shdr_strtab = self._find_shdr_strtab()
        else:
            self._shdr_strtab = None

    @staticmethod
    def readfile(path: str) -> 'Image':
        """Reads the ELF file at the specified path."""
        with open(path, 'rb') as inp:
            return Image(memoryview(inp.read()))

    def _compute_shdr_num(self) -> int:
        """Computes the actual number of section headers."""
        shnum = self.ehdr.e_shnum
        if shnum == 0:
            if self.ehdr.e_shoff == 0 or self.ehdr.e_shentsize == 0:
                # No section headers.
                return 0
            # Otherwise the extension mechanism is used (which may be
            # needed because e_shnum is just 16 bits).
            return self.read(self.Shdr, self.ehdr.e_shoff).sh_size
        return shnum

    def _find_shdr_strtab(self) -> StringTable:
        """Finds the section header string table (maybe via extensions)."""
        shstrndx = self.ehdr.e_shstrndx
        if shstrndx == Shn.SHN_XINDEX:
            shstrndx = self.read(self.Shdr, self.ehdr.e_shoff).sh_link
        return self._find_stringtab(shstrndx)

    def read(self, typ: type, offset:int ):
        """Reads an object at a specific offset.

        The type must have been enhanced using _define_variants.

        """
        return typ.unpack(self.image[offset: offset + typ.size])

    def phdrs(self) -> Phdr:
        """Generator iterating over the program headers."""
        if self.ehdr is None:
            return
        size = self.ehdr.e_phentsize
        if size != self.Phdr.size:
            raise ValueError('Unexpected Phdr size in ELF header: {} != {}'
                             .format(size, self.Phdr.size))

        offset = self.ehdr.e_phoff
        for _ in range(self.ehdr.e_phnum):
            yield self.read(self.Phdr, offset)
            offset += size

    def shdrs(self, resolve: bool=True) -> Shdr:
        """Generator iterating over the section headers.

        If resolve, section names are automatically translated
        using the section header string table.

        """
        if self._shdr_num == 0:
            return

        size = self.ehdr.e_shentsize
        if size != self.Shdr.size:
            raise ValueError('Unexpected Shdr size in ELF header: {} != {}'
                             .format(size, self.Shdr.size))

        offset = self.ehdr.e_shoff
        for _ in range(self._shdr_num):
            shdr = self.read(self.Shdr, offset)
            if resolve:
                shdr = shdr.resolve(self._shdr_strtab)
            yield shdr
            offset += size

    def dynamic(self) -> Dyn:
        """Generator iterating over the dynamic segment."""
        for phdr in self.phdrs():
            if phdr.p_type == Pt.PT_DYNAMIC:
                # Pick the first dynamic segment, like the loader.
                if phdr.p_filesz == 0:
                    # Probably separated debuginfo.
                    return
                offset = phdr.p_offset
                end = offset + phdr.p_memsz
                size = self.Dyn.size
                while True:
                    next_offset = offset + size
                    if next_offset > end:
                        raise ValueError(
                            'Dynamic segment size {} is not a multiple of Dyn size {}'.format(
                                phdr.p_memsz, size))
                    yield self.read(self.Dyn, offset)
                    if next_offset == end:
                        return
                    offset = next_offset

    def syms(self, shdr: Shdr, resolve: bool=True) -> Sym:
        """A generator iterating over a symbol table.

        If resolve, symbol names are automatically translated using
        the string table for the symbol table.

        """
        assert shdr.sh_type == Sht.SHT_SYMTAB
        size = shdr.sh_entsize
        if size != self.Sym.size:
            raise ValueError('Invalid symbol table entry size {}'.format(size))
        offset = shdr.sh_offset
        end = shdr.sh_offset + shdr.sh_size
        if resolve:
            strtab = self._find_stringtab(shdr.sh_link)
        while offset < end:
            sym = self.read(self.Sym, offset)
            if resolve:
                sym = sym.resolve(strtab)
            yield sym
            offset += size
        if offset != end:
            raise ValueError('Symbol table is not a multiple of entry size')

    def lookup_string(self, strtab_index: int, strtab_offset: int) -> bytes:
        """Looks up a string in a string table identified by its link index."""
        try:
            strtab = self._stringtab[strtab_index]
        except KeyError:
            strtab = self._find_stringtab(strtab_index)
        return strtab.get(strtab_offset)

    def find_section(self, shndx: Shn) -> Shdr:
        """Returns the section header for the indexed section.

        The section name is not resolved.
        """
        try:
            return self._section[shndx]
        except KeyError:
            pass
        if shndx in Shn:
            raise ValueError('Reserved section index {}'.format(shndx))
        idx = shndx.value
        if idx < 0 or idx > self._shdr_num:
            raise ValueError('Section index {} out of range [0, {})'.format(
                idx, self._shdr_num))
        shdr = self.read(
            self.Shdr, self.ehdr.e_shoff + idx * self.Shdr.size)
        self._section[shndx] = shdr
        return shdr

    def _find_stringtab(self, sh_link: int) -> StringTable:
        if sh_link in self._stringtab:
            return self._stringtab
        if sh_link < 0 or sh_link >= self._shdr_num:
            raise ValueError('Section index {} out of range [0, {})'.format(
                sh_link, self._shdr_num))
        shdr = self.read(
            self.Shdr, self.ehdr.e_shoff + sh_link * self.Shdr.size)
        if shdr.sh_type != Sht.SHT_STRTAB:
            raise ValueError(
                'Section {} is not a string table: {}'.format(
                    sh_link, shdr.sh_type))
        strtab = StringTable(
            self.image[shdr.sh_offset:shdr.sh_offset + shdr.sh_size])
        # This could retrain essentially arbitrary amounts of data,
        # but caching string tables seems important for performance.
        self._stringtab[sh_link] = strtab
        return strtab


__all__ = [name for name in dir() if name[0].isupper()]
