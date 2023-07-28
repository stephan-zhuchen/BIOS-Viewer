#ifndef ELF_H
#define ELF_H
#include "SymbolDefinition.h"

typedef struct {
  UINT32    n_namesz; /* Length of name. */
  UINT32    n_descsz; /* Length of descriptor. */
  UINT32    n_type;   /* Type of this note. */
} Elf_Note;

/* Indexes into the e_ident array.  Keep synced with
   http://www.sco.com/developers/gabi/latest/ch4.eheader.html */
#define EI_MAG0        0  /* Magic number, byte 0. */
#define EI_MAG1        1  /* Magic number, byte 1. */
#define EI_MAG2        2  /* Magic number, byte 2. */
#define EI_MAG3        3  /* Magic number, byte 3. */
#define EI_CLASS       4  /* Class of machine. */
#define EI_DATA        5  /* Data format. */
#define EI_VERSION     6  /* ELF format version. */
#define EI_OSABI       7  /* Operating system / ABI identification */
#define EI_ABIVERSION  8  /* ABI version */
#define OLD_EI_BRAND   8  /* Start of architecture identification. */
#define EI_PAD         9  /* Start of padding (per SVR4 ABI). */
#define EI_NIDENT      16 /* Size of e_ident array. */

/* Values for the magic number bytes. */
#define ELFMAG0  0x7f
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'
#define ELFMAG   "\177ELF" /* magic string */
#define SELFMAG  4         /* magic string size */

/* Values for e_ident[EI_VERSION] and e_version. */
#define EV_NONE     0
#define EV_CURRENT  1

/* Values for e_ident[EI_CLASS]. */
#define ELFCLASSNONE  0 /* Unknown class. */
#define ELFCLASS32    1 /* 32-bit architecture. */
#define ELFCLASS64    2 /* 64-bit architecture. */

/* Values for e_ident[EI_DATA]. */
#define ELFDATANONE  0  /* Unknown data format. */
#define ELFDATA2LSB  1  /* 2's complement little-endian. */
#define ELFDATA2MSB  2  /* 2's complement big-endian. */

/* Values for e_ident[EI_OSABI]. */
#define ELFOSABI_NONE        0   /* UNIX System V ABI */
#define ELFOSABI_HPUX        1   /* HP-UX operating system */
#define ELFOSABI_NETBSD      2   /* NetBSD */
#define ELFOSABI_LINUX       3   /* GNU/Linux */
#define ELFOSABI_HURD        4   /* GNU/Hurd */
#define ELFOSABI_86OPEN      5   /* 86Open common IA32 ABI */
#define ELFOSABI_SOLARIS     6   /* Solaris */
#define ELFOSABI_AIX         7   /* AIX */
#define ELFOSABI_IRIX        8   /* IRIX */
#define ELFOSABI_FREEBSD     9   /* FreeBSD */
#define ELFOSABI_TRU64       10  /* TRU64 UNIX */
#define ELFOSABI_MODESTO     11  /* Novell Modesto */
#define ELFOSABI_OPENBSD     12  /* OpenBSD */
#define ELFOSABI_OPENVMS     13  /* Open VMS */
#define ELFOSABI_NSK         14  /* HP Non-Stop Kernel */
#define ELFOSABI_ARM         97  /* ARM */
#define ELFOSABI_STANDALONE  255 /* Standalone (embedded) application */

#define ELFOSABI_SYSV      ELFOSABI_NONE /* symbol used in old spec */
#define ELFOSABI_MONTEREY  ELFOSABI_AIX  /* Monterey */

/* e_ident */
#define IS_ELF(ehdr)  ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
       (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
       (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
       (ehdr).e_ident[EI_MAG3] == ELFMAG3)

/* Values for e_type. */
#define ET_NONE    0      /* Unknown type. */
#define ET_REL     1      /* Relocatable. */
#define ET_EXEC    2      /* Executable. */
#define ET_DYN     3      /* Shared object. */
#define ET_CORE    4      /* Core file. */
#define ET_LOOS    0xfe00 /* First operating system specific. */
#define ET_HIOS    0xfeff /* Last operating system-specific. */
#define ET_LOPROC  0xff00 /* First processor-specific. */
#define ET_HIPROC  0xffff /* Last processor-specific. */

/* Values for e_machine. */
#define EM_NONE         0         /* Unknown machine. */
#define EM_M32          1         /* AT&T WE32100. */
#define EM_SPARC        2         /* Sun SPARC. */
#define EM_386          3         /* Intel i386. */
#define EM_68K          4         /* Motorola 68000. */
#define EM_88K          5         /* Motorola 88000. */
#define EM_860          7         /* Intel i860. */
#define EM_MIPS         8         /* MIPS R3000 Big-Endian only. */
#define EM_S370         9         /* IBM System/370. */
#define EM_MIPS_RS3_LE  10        /* MIPS R3000 Little-Endian. */
#define EM_PARISC       15        /* HP PA-RISC. */
#define EM_VPP500       17        /* Fujitsu VPP500. */
#define EM_SPARC32PLUS  18        /* SPARC v8plus. */
#define EM_960          19        /* Intel 80960. */
#define EM_PPC          20        /* PowerPC 32-bit. */
#define EM_PPC64        21        /* PowerPC 64-bit. */
#define EM_S390         22        /* IBM System/390. */
#define EM_V800         36        /* NEC V800. */
#define EM_FR20         37        /* Fujitsu FR20. */
#define EM_RH32         38        /* TRW RH-32. */
#define EM_RCE          39        /* Motorola RCE. */
#define EM_ARM          40        /* ARM. */
#define EM_SH           42        /* Hitachi SH. */
#define EM_SPARCV9      43        /* SPARC v9 64-bit. */
#define EM_TRICORE      44        /* Siemens TriCore embedded processor. */
#define EM_ARC          45        /* Argonaut RISC Core. */
#define EM_H8_300       46        /* Hitachi H8/300. */
#define EM_H8_300H      47        /* Hitachi H8/300H. */
#define EM_H8S          48        /* Hitachi H8S. */
#define EM_H8_500       49        /* Hitachi H8/500. */
#define EM_MIPS_X       51        /* Stanford MIPS-X. */
#define EM_COLDFIRE     52        /* Motorola ColdFire. */
#define EM_68HC12       53        /* Motorola M68HC12. */
#define EM_MMA          54        /* Fujitsu MMA. */
#define EM_PCP          55        /* Siemens PCP. */
#define EM_NCPU         56        /* Sony nCPU. */
#define EM_NDR1         57        /* Denso NDR1 microprocessor. */
#define EM_STARCORE     58        /* Motorola Star*Core processor. */
#define EM_ME16         59        /* Toyota ME16 processor. */
#define EM_ST100        60        /* STMicroelectronics ST100 processor. */
#define EM_TINYJ        61        /* Advanced Logic Corp. TinyJ processor. */
#define EM_X86_64       62        /* Advanced Micro Devices x86-64 */
#define  EM_AMD64       EM_X86_64 /* Advanced Micro Devices x86-64 (compat) */
#define EM_AARCH64      183       /* ARM 64bit Architecture */

/* Non-standard or deprecated. */
#define EM_486          6      /* Intel i486. */
#define EM_MIPS_RS4_BE  10     /* MIPS R4000 Big-Endian */
#define EM_ALPHA_STD    41     /* Digital Alpha (standard value). */
#define EM_ALPHA        0x9026 /* Alpha (written in the absence of an ABI) */

/* Special section indexes. */
#define SHN_UNDEF      0      /* Undefined, missing, irrelevant. */
#define SHN_LORESERVE  0xff00 /* First of reserved range. */
#define SHN_LOPROC     0xff00 /* First processor-specific. */
#define SHN_HIPROC     0xff1f /* Last processor-specific. */
#define SHN_LOOS       0xff20 /* First operating system-specific. */
#define SHN_HIOS       0xff3f /* Last operating system-specific. */
#define SHN_ABS        0xfff1 /* Absolute values. */
#define SHN_COMMON     0xfff2 /* Common data. */
#define SHN_XINDEX     0xffff /* Escape -- index stored elsewhere. */
#define SHN_HIRESERVE  0xffff /* Last of reserved range. */

/* sh_type */
#define SHT_NULL            0          /* inactive */
#define SHT_PROGBITS        1          /* program defined information */
#define SHT_SYMTAB          2          /* symbol table section */
#define SHT_STRTAB          3          /* string table section */
#define SHT_RELA            4          /* relocation section with addends */
#define SHT_HASH            5          /* symbol hash table section */
#define SHT_DYNAMIC         6          /* dynamic section */
#define SHT_NOTE            7          /* note section */
#define SHT_NOBITS          8          /* no space section */
#define SHT_REL             9          /* relocation section - no addends */
#define SHT_SHLIB           10         /* reserved - purpose unknown */
#define SHT_DYNSYM          11         /* dynamic symbol table section */
#define SHT_INIT_ARRAY      14         /* Initialization function pointers. */
#define SHT_FINI_ARRAY      15         /* Termination function pointers. */
#define SHT_PREINIT_ARRAY   16         /* Pre-initialization function ptrs. */
#define SHT_GROUP           17         /* Section group. */
#define SHT_SYMTAB_SHNDX    18         /* Section indexes (see SHN_XINDEX). */
#define SHT_LOOS            0x60000000 /* First of OS specific semantics */
#define SHT_LOSUNW          0x6ffffff4
#define SHT_SUNW_dof        0x6ffffff4
#define SHT_SUNW_cap        0x6ffffff5
#define SHT_SUNW_SIGNATURE  0x6ffffff6
#define SHT_SUNW_ANNOTATE   0x6ffffff7
#define SHT_SUNW_DEBUGSTR   0x6ffffff8
#define SHT_SUNW_DEBUG      0x6ffffff9
#define SHT_SUNW_move       0x6ffffffa
#define SHT_SUNW_COMDAT     0x6ffffffb
#define SHT_SUNW_syminfo    0x6ffffffc
#define SHT_SUNW_verdef     0x6ffffffd
#define SHT_GNU_verdef      0x6ffffffd/* Symbol versions provided */
#define SHT_SUNW_verneed    0x6ffffffe
#define SHT_GNU_verneed     0x6ffffffe /* Symbol versions required */
#define SHT_SUNW_versym     0x6fffffff
#define SHT_GNU_versym      0x6fffffff/* Symbol version table */
#define SHT_HISUNW          0x6fffffff
#define SHT_HIOS            0x6fffffff /* Last of OS specific semantics */
#define SHT_LOPROC          0x70000000 /* reserved range for processor */
#define SHT_AMD64_UNWIND    0x70000001 /* unwind information */
#define SHT_HIPROC          0x7fffffff /* specific section header types */
#define SHT_LOUSER          0x80000000 /* reserved range for application */
#define SHT_HIUSER          0xffffffff /* specific indexes */

/* Flags for sh_flags. */
#define SHF_WRITE             0x1        /* Section contains writable data. */
#define SHF_ALLOC             0x2        /* Section occupies memory. */
#define SHF_EXECINSTR         0x4        /* Section contains instructions. */
#define SHF_MERGE             0x10       /* Section may be merged. */
#define SHF_STRINGS           0x20       /* Section contains strings. */
#define SHF_INFO_LINK         0x40       /* sh_info holds section index. */
#define SHF_LINK_ORDER        0x80       /* Special ordering requirements. */
#define SHF_OS_NONCONFORMING  0x100      /* OS-specific processing required. */
#define SHF_GROUP             0x200      /* Member of section group. */
#define SHF_TLS               0x400      /* Section contains TLS data. */
#define SHF_MASKOS            0x0ff00000 /* OS-specific semantics. */
#define SHF_MASKPROC          0xf0000000 /* Processor-specific semantics. */

/* Values for p_type. */
#define PT_NULL           0          /* Unused entry. */
#define PT_LOAD           1          /* Loadable segment. */
#define PT_DYNAMIC        2          /* Dynamic linking information segment. */
#define PT_INTERP         3          /* Pathname of interpreter. */
#define PT_NOTE           4          /* Auxiliary information. */
#define PT_SHLIB          5          /* Reserved (not used). */
#define PT_PHDR           6          /* Location of program header itself. */
#define  PT_TLS           7          /* Thread local storage segment */
#define PT_LOOS           0x60000000 /* First OS-specific. */
#define  PT_SUNW_UNWIND   0x6464e550 /* amd64 UNWIND program header */
#define  PT_GNU_EH_FRAME  0x6474e550
#define  PT_LOSUNW        0x6ffffffa
#define  PT_SUNWBSS       0x6ffffffa /* Sun Specific segment */
#define  PT_SUNWSTACK     0x6ffffffb /* describes the stack segment */
#define  PT_SUNWDTRACE    0x6ffffffc /* private */
#define  PT_SUNWCAP       0x6ffffffd /* hard/soft capabilities segment */
#define  PT_HISUNW        0x6fffffff
#define PT_HIOS           0x6fffffff /* Last OS-specific. */
#define PT_LOPROC         0x70000000 /* First processor-specific type. */
#define PT_HIPROC         0x7fffffff /* Last processor-specific type. */

/* Values for p_flags. */
#define PF_X         0x1        /* Executable. */
#define PF_W         0x2        /* Writable. */
#define PF_R         0x4        /* Readable. */
#define PF_MASKOS    0x0ff00000 /* Operating system-specific. */
#define PF_MASKPROC  0xf0000000 /* Processor-specific. */

/* Extended program header index. */
#define  PN_XNUM  0xffff

/* Values for d_tag. */
#define DT_NULL              0          /* Terminating entry. */
#define DT_NEEDED            1          /* String table offset of a needed shared
           library. */
#define DT_PLTRELSZ          2          /* Total size in bytes of PLT relocations. */
#define DT_PLTGOT            3          /* Processor-dependent address. */
#define DT_HASH              4          /* Address of symbol hash table. */
#define DT_STRTAB            5          /* Address of string table. */
#define DT_SYMTAB            6          /* Address of symbol table. */
#define DT_RELA              7          /* Address of ElfNN_Rela relocations. */
#define DT_RELASZ            8          /* Total size of ElfNN_Rela relocations. */
#define DT_RELAENT           9          /* Size of each ElfNN_Rela relocation entry. */
#define DT_STRSZ             10         /* Size of string table. */
#define DT_SYMENT            11         /* Size of each symbol table entry. */
#define DT_INIT              12         /* Address of initialization function. */
#define DT_FINI              13         /* Address of finalization function. */
#define DT_SONAME            14         /* String table offset of shared object
           name. */
#define DT_RPATH             15         /* String table offset of library path. [sup] */
#define DT_SYMBOLIC          16         /* Indicates "symbolic" linking. [sup] */
#define DT_REL               17         /* Address of ElfNN_Rel relocations. */
#define DT_RELSZ             18         /* Total size of ElfNN_Rel relocations. */
#define DT_RELENT            19         /* Size of each ElfNN_Rel relocation. */
#define DT_PLTREL            20         /* Type of relocation used for PLT. */
#define DT_DEBUG             21         /* Reserved (not used). */
#define DT_TEXTREL           22         /* Indicates there may be relocations in
           non-writable segments. [sup] */
#define DT_JMPREL            23         /* Address of PLT relocations. */
#define  DT_BIND_NOW         24         /* [sup] */
#define  DT_INIT_ARRAY       25         /* Address of the array of pointers to
           initialization functions */
#define  DT_FINI_ARRAY       26         /* Address of the array of pointers to
           termination functions */
#define  DT_INIT_ARRAYSZ     27         /* Size in bytes of the array of
           initialization functions. */
#define  DT_FINI_ARRAYSZ     28         /* Size in bytes of the array of
           terminationfunctions. */
#define  DT_RUNPATH          29         /* String table offset of a null-terminated
           library search path string. */
#define  DT_FLAGS            30         /* Object specific flag values. */
#define  DT_ENCODING         32         /* Values greater than or equal to DT_ENCODING
           and less than DT_LOOS follow the rules for
           the interpretation of the d_un union
           as follows: even == 'd_ptr', even == 'd_val'
           or none */
#define  DT_PREINIT_ARRAY    32         /* Address of the array of pointers to
           pre-initialization functions. */
#define  DT_PREINIT_ARRAYSZ  33         /* Size in bytes of the array of
           pre-initialization functions. */
#define  DT_MAXPOSTAGS       34         /* number of positive tags */
#define  DT_LOOS             0x6000000d /* First OS-specific */
#define  DT_SUNW_AUXILIARY   0x6000000d /* symbol auxiliary name */
#define  DT_SUNW_RTLDINF     0x6000000e /* ld.so.1 info (private) */
#define  DT_SUNW_FILTER      0x6000000f /* symbol filter name */
#define  DT_SUNW_CAP         0x60000010 /* hardware/software */
#define  DT_HIOS             0x6ffff000 /* Last OS-specific */

/*
 * DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
 * Dyn.d_un.d_val field of the Elf*_Dyn structure.
 */
#define  DT_VALRNGLO   0x6ffffd00
#define  DT_CHECKSUM   0x6ffffdf8 /* elf checksum */
#define  DT_PLTPADSZ   0x6ffffdf9 /* pltpadding size */
#define  DT_MOVEENT    0x6ffffdfa /* move table entry size */
#define  DT_MOVESZ     0x6ffffdfb /* move table size */
#define  DT_FEATURE_1  0x6ffffdfc /* feature holder */
#define  DT_POSFLAG_1  0x6ffffdfd /* flags for DT_* entries, effecting */
/*  the following DT_* entry. */
/*  See DF_P1_* definitions */
#define  DT_SYMINSZ   0x6ffffdfe /* syminfo table size (in bytes) */
#define  DT_SYMINENT  0x6ffffdff /* syminfo entry size (in bytes) */
#define  DT_VALRNGHI  0x6ffffdff

/*
 * DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the
 * Dyn.d_un.d_ptr field of the Elf*_Dyn structure.
 *
 * If any adjustment is made to the ELF object after it has been
 * built, these entries will need to be adjusted.
 */
#define  DT_ADDRRNGLO  0x6ffffe00
#define  DT_CONFIG     0x6ffffefa /* configuration information */
#define  DT_DEPAUDIT   0x6ffffefb /* dependency auditing */
#define  DT_AUDIT      0x6ffffefc /* object auditing */
#define  DT_PLTPAD     0x6ffffefd /* pltpadding (sparcv9) */
#define  DT_MOVETAB    0x6ffffefe /* move table */
#define  DT_SYMINFO    0x6ffffeff /* syminfo table */
#define  DT_ADDRRNGHI  0x6ffffeff

#define  DT_VERSYM      0x6ffffff0 /* Address of versym section. */
#define  DT_RELACOUNT   0x6ffffff9 /* number of RELATIVE relocations */
#define  DT_RELCOUNT    0x6ffffffa /* number of RELATIVE relocations */
#define  DT_FLAGS_1     0x6ffffffb /* state flags - see DF_1_* defs */
#define  DT_VERDEF      0x6ffffffc /* Address of verdef section. */
#define  DT_VERDEFNUM   0x6ffffffd /* Number of elems in verdef section */
#define  DT_VERNEED     0x6ffffffe /* Address of verneed section. */
#define  DT_VERNEEDNUM  0x6fffffff /* Number of elems in verneed section */

#define  DT_LOPROC                     0x70000000/* First processor-specific type. */
#define  DT_DEPRECATED_SPARC_REGISTER  0x7000001
#define  DT_AUXILIARY                  0x7ffffffd /* shared library auxiliary name */
#define  DT_USED                       0x7ffffffe /* ignored - same as needed */
#define  DT_FILTER                     0x7fffffff /* shared library filter name */
#define  DT_HIPROC                     0x7fffffff /* Last processor-specific type. */

/* Values for DT_FLAGS */
#define  DF_ORIGIN      0x0001 /* Indicates that the object being loaded may
           make reference to the $ORIGIN substitution
           string */
#define  DF_SYMBOLIC    0x0002 /* Indicates "symbolic" linking. */
#define  DF_TEXTREL     0x0004 /* Indicates there may be relocations in
           non-writable segments. */
#define  DF_BIND_NOW    0x0008 /* Indicates that the dynamic linker should
           process all relocations for the object
           containing this entry before transferring
           control to the program. */
#define  DF_STATIC_TLS  0x0010 /* Indicates that the shared object or
           executable contains code using a static
           thread-local storage scheme. */

/* Values for n_type.  Used in core files. */
#define NT_PRSTATUS  1  /* Process status. */
#define NT_FPREGSET  2  /* Floating point registers. */
#define NT_PRPSINFO  3  /* Process state info. */

/* Symbol Binding - ELFNN_ST_BIND - st_info */
#define STB_LOCAL   0  /* Local symbol */
#define STB_GLOBAL  1  /* Global symbol */
#define STB_WEAK    2  /* like global - lower precedence */
#define STB_LOOS    10 /* Reserved range for operating system */
#define STB_HIOS    12 /*   specific semantics. */
#define STB_LOPROC  13 /* reserved range for processor */
#define STB_HIPROC  15 /*   specific semantics. */

/* Symbol type - ELFNN_ST_TYPE - st_info */
#define STT_NOTYPE   0 /* Unspecified type. */
#define STT_OBJECT   1 /* Data object. */
#define STT_FUNC     2 /* Function. */
#define STT_SECTION  3 /* Section. */
#define STT_FILE     4 /* Source file. */
#define STT_COMMON   5 /* Uninitialized common block. */
#define STT_TLS      6 /* TLS object. */
#define STT_NUM      7
#define STT_LOOS     10 /* Reserved range for operating system */
#define STT_HIOS     12 /*   specific semantics. */
#define STT_LOPROC   13 /* reserved range for processor */
#define STT_HIPROC   15 /*   specific semantics. */

/* Symbol visibility - ELFNN_ST_VISIBILITY - st_other */
#define STV_DEFAULT    0x0 /* Default visibility (see binding). */
#define STV_INTERNAL   0x1 /* Special meaning in relocatable objects. */
#define STV_HIDDEN     0x2 /* Not visible. */
#define STV_PROTECTED  0x3 /* Visible but not preemptible. */

/* Special symbol table indexes. */
#define STN_UNDEF  0  /* Undefined symbol index. */

/* Symbol versioning flags. */
#define  VER_DEF_CURRENT  1
#define VER_DEF_IDX(x)  VER_NDX(x)

#define  VER_FLG_BASE  0x01
#define  VER_FLG_WEAK  0x02

#define  VER_NEED_CURRENT  1
#define VER_NEED_WEAK      (1u << 15)
#define VER_NEED_HIDDEN    VER_NDX_HIDDEN
#define VER_NEED_IDX(x)  VER_NDX(x)

#define  VER_NDX_LOCAL   0
#define  VER_NDX_GLOBAL  1
#define VER_NDX_GIVEN    2

#define VER_NDX_HIDDEN  (1u << 15)
#define VER_NDX(x)  ((x) & ~(1u << 15))

#define  CA_SUNW_NULL  0
#define  CA_SUNW_HW_1  1    /* first hardware capabilities entry */
#define  CA_SUNW_SF_1  2    /* first software capabilities entry */

/*
 * Syminfo flag values
 */
#define  SYMINFO_FLG_DIRECT  0x0001  /* symbol ref has direct association */
/*  to object containing defn. */
#define  SYMINFO_FLG_PASSTHRU  0x0002 /* ignored - see SYMINFO_FLG_FILTER */
#define  SYMINFO_FLG_COPY      0x0004 /* symbol is a copy-reloc */
#define  SYMINFO_FLG_LAZYLOAD  0x0008 /* object containing defn should be */
/*  lazily-loaded */
#define  SYMINFO_FLG_DIRECTBIND  0x0010  /* ref should be bound directly to */
/*  object containing defn. */
#define  SYMINFO_FLG_NOEXTDIRECT  0x0020  /* don't let an external reference */
/*  directly bind to this symbol */
#define  SYMINFO_FLG_FILTER     0x0002 /* symbol ref is associated to a */
#define  SYMINFO_FLG_AUXILIARY  0x0040 /*   standard or auxiliary filter */

/*
 * Syminfo.si_boundto values.
 */
#define  SYMINFO_BT_SELF        0xffff /* symbol bound to self */
#define  SYMINFO_BT_PARENT      0xfffe /* symbol bound to parent */
#define  SYMINFO_BT_NONE        0xfffd /* no special symbol binding */
#define  SYMINFO_BT_EXTERN      0xfffc /* symbol defined as external */
#define  SYMINFO_BT_LOWRESERVE  0xff00 /* beginning of reserved entries */

/*
 * Syminfo version values.
 */
#define  SYMINFO_NONE     0 /* Syminfo version */
#define  SYMINFO_CURRENT  1
#define  SYMINFO_NUM      2

/*
 * Relocation types.
 *
 * All machine architectures are defined here to allow tools on one to
 * handle others.
 */

#define  R_386_NONE          0  /* No relocation. */
#define  R_386_32            1  /* Add symbol value. */
#define  R_386_PC32          2  /* Add PC-relative symbol value. */
#define  R_386_GOT32         3  /* Add PC-relative GOT offset. */
#define  R_386_PLT32         4  /* Add PC-relative PLT offset. */
#define  R_386_COPY          5  /* Copy data from shared object. */
#define  R_386_GLOB_DAT      6  /* Set GOT entry to data address. */
#define  R_386_JMP_SLOT      7  /* Set GOT entry to code address. */
#define  R_386_RELATIVE      8  /* Add load address of shared object. */
#define  R_386_GOTOFF        9  /* Add GOT-relative symbol address. */
#define  R_386_GOTPC         10 /* Add PC-relative GOT table address. */
#define  R_386_TLS_TPOFF     14 /* Negative offset in static TLS block */
#define  R_386_TLS_IE        15 /* Absolute address of GOT for -ve static TLS */
#define  R_386_TLS_GOTIE     16 /* GOT entry for negative static TLS block */
#define  R_386_TLS_LE        17 /* Negative offset relative to static TLS */
#define  R_386_TLS_GD        18 /* 32 bit offset to GOT (index,off) pair */
#define  R_386_TLS_LDM       19 /* 32 bit offset to GOT (index,zero) pair */
#define  R_386_TLS_GD_32     24 /* 32 bit offset to GOT (index,off) pair */
#define  R_386_TLS_GD_PUSH   25 /* pushl instruction for Sun ABI GD sequence */
#define  R_386_TLS_GD_CALL   26 /* call instruction for Sun ABI GD sequence */
#define  R_386_TLS_GD_POP    27 /* popl instruction for Sun ABI GD sequence */
#define  R_386_TLS_LDM_32    28 /* 32 bit offset to GOT (index,zero) pair */
#define  R_386_TLS_LDM_PUSH  29 /* pushl instruction for Sun ABI LD sequence */
#define  R_386_TLS_LDM_CALL  30 /* call instruction for Sun ABI LD sequence */
#define  R_386_TLS_LDM_POP   31 /* popl instruction for Sun ABI LD sequence */
#define  R_386_TLS_LDO_32    32 /* 32 bit offset from start of TLS block */
#define  R_386_TLS_IE_32     33 /* 32 bit offset to GOT static TLS offset entry */
#define  R_386_TLS_LE_32     34 /* 32 bit offset within static TLS block */
#define  R_386_TLS_DTPMOD32  35 /* GOT entry containing TLS index */
#define  R_386_TLS_DTPOFF32  36 /* GOT entry containing TLS offset */
#define  R_386_TLS_TPOFF32   37 /* GOT entry of -ve static TLS offset */

/* Null relocation */
#define  R_AARCH64_NONE  256        /* No relocation */
/* Static AArch64 relocations */
/* Static data relocations */
#define  R_AARCH64_ABS64   257      /* S + A */
#define  R_AARCH64_ABS32   258      /* S + A */
#define  R_AARCH64_ABS16   259      /* S + A */
#define  R_AARCH64_PREL64  260      /* S + A - P */
#define  R_AARCH64_PREL32  261      /* S + A - P */
#define  R_AARCH64_PREL16  262      /* S + A - P */
/* Group relocations to create a 16, 32, 48, or 64 bit unsigned data value or address inline */
#define  R_AARCH64_MOVW_UABS_G0     263   /* S + A */
#define  R_AARCH64_MOVW_UABS_G0_NC  264   /* S + A */
#define  R_AARCH64_MOVW_UABS_G1     265   /* S + A */
#define  R_AARCH64_MOVW_UABS_G1_NC  266   /* S + A */
#define  R_AARCH64_MOVW_UABS_G2     267   /* S + A */
#define  R_AARCH64_MOVW_UABS_G2_NC  268   /* S + A */
#define  R_AARCH64_MOVW_UABS_G3     269   /* S + A */
/* Group relocations to create a 16, 32, 48, or 64 bit signed data or offset value inline */
#define  R_AARCH64_MOVW_SABS_G0  270      /* S + A */
#define  R_AARCH64_MOVW_SABS_G1  271      /* S + A */
#define  R_AARCH64_MOVW_SABS_G2  272      /* S + A */
/* Relocations to generate 19, 21 and 33 bit PC-relative addresses */
#define  R_AARCH64_LD_PREL_LO19         273 /* S + A - P */
#define  R_AARCH64_ADR_PREL_LO21        274 /* S + A - P */
#define  R_AARCH64_ADR_PREL_PG_HI21     275 /* Page(S+A) - Page(P) */
#define  R_AARCH64_ADR_PREL_PG_HI21_NC  276 /* Page(S+A) - Page(P) */
#define  R_AARCH64_ADD_ABS_LO12_NC      277 /* S + A */
#define  R_AARCH64_LDST8_ABS_LO12_NC    278 /* S + A */
#define  R_AARCH64_LDST16_ABS_LO12_NC   284 /* S + A */
#define  R_AARCH64_LDST32_ABS_LO12_NC   285 /* S + A */
#define  R_AARCH64_LDST64_ABS_LO12_NC   286 /* S + A */
#define  R_AARCH64_LDST128_ABS_LO12_NC  299 /* S + A */
/* Relocations for control-flow instructions - all offsets are a multiple of 4 */
#define  R_AARCH64_TSTBR14   279    /* S+A-P */
#define  R_AARCH64_CONDBR19  280    /* S+A-P */
#define  R_AARCH64_JUMP26    282    /* S+A-P */
#define  R_AARCH64_CALL26    283    /* S+A-P */
/* Group relocations to create a 16, 32, 48, or 64 bit PC-relative offset inline */
#define  R_AARCH64_MOVW_PREL_G0     287   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G0_NC  288   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G1     289   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G1_NC  290   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G2     291   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G2_NC  292   /* S+A-P */
#define  R_AARCH64_MOVW_PREL_G3     293   /* S+A-P */
/* Group relocations to create a 16, 32, 48, or 64 bit GOT-relative offsets inline */
#define  R_AARCH64_MOVW_GOTOFF_G0     300 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G0_NC  301 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G1     302 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G1_NC  303 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G2     304 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G2_NC  305 /* G(S)-GOT */
#define  R_AARCH64_MOVW_GOTOFF_G3     306 /* G(S)-GOT */
/*  GOT-relative data relocations */
#define  R_AARCH64_GOTREL64  307      /* S+A-GOT */
#define  R_AARCH64_GOTREL32  308      /* S+A-GOT */
/* GOT-relative instruction relocations */
#define  R_AARCH64_GOT_LD_PREL19      309 /* G(S)-P */
#define  R_AARCH64_LD64_GOTOFF_LO15   310 /* G(S)-GOT */
#define  R_AARCH64_ADR_GOT_PAGE       311 /* Page(G(S))-Page(P) */
#define  R_AARCH64_LD64_GOT_LO12_NC   312 /* G(S) */
#define  R_AARCH64_LD64_GOTPAGE_LO15  313 /* G(S)-Page(GOT) */
/* Relocations for thread-local storage */
/* General Dynamic TLS relocations */
#define  R_AARCH64_TLSGD_ADR_PREL21   512  /* G(TLSIDX(S+A)) - P */
#define  R_AARCH64_TLSGD_ADR_PAGE21   513  /* Page(G(TLSIDX(S+A))) - Page(P) */
#define  R_AARCH64_TLSGD_ADD_LO12_NC  514  /* G(TLSIDX(S+A)) */
#define  R_AARCH64_TLSGD_MOVW_G1      515  /* G(TLSIDX(S+A)) - GOT */
#define  R_AARCH64_TLSGD_MOVW_G0_NC   516  /* G(TLSIDX(S+A)) - GOT */
/* Local Dynamic TLS relocations */
#define  R_AARCH64_TLSLD_ADR_PREL21             517 /* G(LDM(S))) - P */
#define  R_AARCH64_TLSLD_ADR_PAGE21             518 /* Page(G(LDM(S)))-Page(P) */
#define  R_AARCH64_TLSLD_ADD_LO12_NC            519 /* G(LDM(S)) */
#define  R_AARCH64_TLSLD_MOVW_G1                520 /* G(LDM(S)) - GOT */
#define  R_AARCH64_TLSLD_MOVW_G0_NC             521 /* G(LDM(S)) - GOT */
#define  R_AARCH64_TLSLD_LD_PREL19              522 /* G(LDM(S)) - P */
#define  R_AARCH64_TLSLD_MOVW_DTPREL_G2         523 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_MOVW_DTPREL_G1         524 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_MOVW_DTPREL_G1_NC      525 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_MOVW_DTPREL_G0         526 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC      527 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_ADD_DTPREL_HI12        528 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_ADD_DTPREL_LO12        529 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC     530 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST8_DTPREL_LO12      531 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC   532 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST16_DTPREL_LO12     533 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC  534 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST32_DTPREL_LO12     535 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC  536 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST64_DTPREL_LO12     537 /* DTPREL(S+A) */
#define  R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC  538 /* DTPREL(S+A) */
/* Initial Exec TLS relocations */
#define  R_AARCH64_TLSIE_MOVW_GOTTPREL_G1       539 /* G(TPREL(S+A)) - GOT */
#define  R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC    540 /* G(TPREL(S+A)) - GOT */
#define  R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21    541 /* Page(G(TPREL(S+A))) - Page(P) */
#define  R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC  542 /* G(TPREL(S+A)) */
#define  R_AARCH64_TLSIE_LD_GOTTPREL_PREL19     543 /* G(TPREL(S+A)) - P */
/* Local Exec TLS relocations */
#define  R_AARCH64_TLSLE_MOVW_TPREL_G2         544 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_MOVW_TPREL_G1         545 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_MOVW_TPREL_G1_NC      546 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_MOVW_TPREL_G0         547 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_MOVW_TPREL_G0_NC      548 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_ADD_TPREL_HI12        549 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_ADD_TPREL_LO12        550 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_ADD_TPREL_LO12_NC     551 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST8_TPREL_LO12      552 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC   553 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST16_TPREL_LO12     554 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC  555 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST32_TPREL_LO12     556 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC  557 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST64_TPREL_LO12     558 /* TPREL(S+A) */
#define  R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC  559 /* TPREL(S+A) */
/* Dynamic relocations */
/* Dynamic relocations */
#define  R_AARCH64_COPY          1024
#define  R_AARCH64_GLOB_DAT      1025  /* S + A */
#define  R_AARCH64_JUMP_SLOT     1026  /* S + A */
#define  R_AARCH64_RELATIVE      1027  /* Delta(S) + A , Delta(P) + A */
#define  R_AARCH64_TLS_DTPREL64  1028  /* DTPREL(S+A) */
#define  R_AARCH64_TLS_DTPMOD64  1029  /* LDM(S) */
#define  R_AARCH64_TLS_TPREL64   1030  /* TPREL(S+A) */
#define  R_AARCH64_TLS_DTPREL32  1031  /* DTPREL(S+A) */
#define  R_AARCH64_TLS_DTPMOD32  1032  /* LDM(S) */
#define  R_AARCH64_TLS_TPREL32   1033  /* DTPREL(S+A) */

#define  R_ALPHA_NONE            0  /* No reloc */
#define  R_ALPHA_REFLONG         1  /* Direct 32 bit */
#define  R_ALPHA_REFQUAD         2  /* Direct 64 bit */
#define  R_ALPHA_GPREL32         3  /* GP relative 32 bit */
#define  R_ALPHA_LITERAL         4  /* GP relative 16 bit w/optimization */
#define  R_ALPHA_LITUSE          5  /* Optimization hint for LITERAL */
#define  R_ALPHA_GPDISP          6  /* Add displacement to GP */
#define  R_ALPHA_BRADDR          7  /* PC+4 relative 23 bit shifted */
#define  R_ALPHA_HINT            8  /* PC+4 relative 16 bit shifted */
#define  R_ALPHA_SREL16          9  /* PC relative 16 bit */
#define  R_ALPHA_SREL32          10 /* PC relative 32 bit */
#define  R_ALPHA_SREL64          11 /* PC relative 64 bit */
#define  R_ALPHA_OP_PUSH         12 /* OP stack push */
#define  R_ALPHA_OP_STORE        13 /* OP stack pop and store */
#define  R_ALPHA_OP_PSUB         14 /* OP stack subtract */
#define  R_ALPHA_OP_PRSHIFT      15 /* OP stack right shift */
#define  R_ALPHA_GPVALUE         16
#define  R_ALPHA_GPRELHIGH       17
#define  R_ALPHA_GPRELLOW        18
#define  R_ALPHA_IMMED_GP_16     19
#define  R_ALPHA_IMMED_GP_HI32   20
#define  R_ALPHA_IMMED_SCN_HI32  21
#define  R_ALPHA_IMMED_BR_HI32   22
#define  R_ALPHA_IMMED_LO32      23
#define  R_ALPHA_COPY            24 /* Copy symbol at runtime */
#define  R_ALPHA_GLOB_DAT        25 /* Create GOT entry */
#define  R_ALPHA_JMP_SLOT        26 /* Create PLT entry */
#define  R_ALPHA_RELATIVE        27 /* Adjust by program base */

#define  R_ARM_NONE            0/* No relocation. */
#define  R_ARM_PC24            1
#define  R_ARM_ABS32           2
#define  R_ARM_REL32           3
#define  R_ARM_PC13            4
#define  R_ARM_ABS16           5
#define  R_ARM_ABS12           6
#define  R_ARM_THM_ABS5        7
#define  R_ARM_ABS8            8
#define  R_ARM_SBREL32         9
#define  R_ARM_THM_PC22        10
#define  R_ARM_THM_PC8         11
#define  R_ARM_AMP_VCALL9      12
#define  R_ARM_SWI24           13
#define  R_ARM_THM_SWI8        14
#define  R_ARM_XPC25           15
#define  R_ARM_THM_XPC22       16
#define  R_ARM_COPY            20 /* Copy data from shared object. */
#define  R_ARM_GLOB_DAT        21 /* Set GOT entry to data address. */
#define  R_ARM_JUMP_SLOT       22 /* Set GOT entry to code address. */
#define  R_ARM_RELATIVE        23 /* Add load address of shared object. */
#define  R_ARM_GOTOFF          24 /* Add GOT-relative symbol address. */
#define  R_ARM_GOTPC           25 /* Add PC-relative GOT table address. */
#define  R_ARM_GOT32           26 /* Add PC-relative GOT offset. */
#define  R_ARM_PLT32           27 /* Add PC-relative PLT offset. */
#define R_ARM_CALL             28
#define R_ARM_JMP24            29
#define R_ARM_THM_MOVW_ABS_NC  47
#define R_ARM_THM_MOVT_ABS     48

// Block of PC-relative relocations added to work around gcc putting
// object relocations in static executables.
#define R_ARM_THM_JUMP24         30
#define R_ARM_PREL31             42
#define R_ARM_MOVW_PREL_NC       45
#define R_ARM_MOVT_PREL          46
#define R_ARM_THM_MOVW_PREL_NC   49
#define R_ARM_THM_MOVT_PREL      50
#define R_ARM_THM_JMP6           52
#define R_ARM_THM_ALU_PREL_11_0  53
#define R_ARM_THM_PC12           54
#define R_ARM_REL32_NOI          56
#define R_ARM_ALU_PC_G0_NC       57
#define R_ARM_ALU_PC_G0          58
#define R_ARM_ALU_PC_G1_NC       59
#define R_ARM_ALU_PC_G1          60
#define R_ARM_ALU_PC_G2          61
#define R_ARM_LDR_PC_G1          62
#define R_ARM_LDR_PC_G2          63
#define R_ARM_LDRS_PC_G0         64
#define R_ARM_LDRS_PC_G1         65
#define R_ARM_LDRS_PC_G2         66
#define R_ARM_LDC_PC_G0          67
#define R_ARM_LDC_PC_G1          68
#define R_ARM_LDC_PC_G2          69
#define R_ARM_GOT_PREL           96
#define R_ARM_THM_JUMP11         102
#define R_ARM_THM_JUMP8          103
#define R_ARM_TLS_GD32           104
#define R_ARM_TLS_LDM32          105
#define R_ARM_TLS_IE32           107

#define R_ARM_THM_JUMP19      51
#define  R_ARM_GNU_VTENTRY    100
#define  R_ARM_GNU_VTINHERIT  101
#define  R_ARM_RSBREL32       250
#define  R_ARM_THM_RPC22      251
#define  R_ARM_RREL32         252
#define  R_ARM_RABS32         253
#define  R_ARM_RPC24          254
#define  R_ARM_RBASE          255

#define  R_PPC_NONE             0/* No relocation. */
#define  R_PPC_ADDR32           1
#define  R_PPC_ADDR24           2
#define  R_PPC_ADDR16           3
#define  R_PPC_ADDR16_LO        4
#define  R_PPC_ADDR16_HI        5
#define  R_PPC_ADDR16_HA        6
#define  R_PPC_ADDR14           7
#define  R_PPC_ADDR14_BRTAKEN   8
#define  R_PPC_ADDR14_BRNTAKEN  9
#define  R_PPC_REL24            10
#define  R_PPC_REL14            11
#define  R_PPC_REL14_BRTAKEN    12
#define  R_PPC_REL14_BRNTAKEN   13
#define  R_PPC_GOT16            14
#define  R_PPC_GOT16_LO         15
#define  R_PPC_GOT16_HI         16
#define  R_PPC_GOT16_HA         17
#define  R_PPC_PLTREL24         18
#define  R_PPC_COPY             19
#define  R_PPC_GLOB_DAT         20
#define  R_PPC_JMP_SLOT         21
#define  R_PPC_RELATIVE         22
#define  R_PPC_LOCAL24PC        23
#define  R_PPC_UADDR32          24
#define  R_PPC_UADDR16          25
#define  R_PPC_REL32            26
#define  R_PPC_PLT32            27
#define  R_PPC_PLTREL32         28
#define  R_PPC_PLT16_LO         29
#define  R_PPC_PLT16_HI         30
#define  R_PPC_PLT16_HA         31
#define  R_PPC_SDAREL16         32
#define  R_PPC_SECTOFF          33
#define  R_PPC_SECTOFF_LO       34
#define  R_PPC_SECTOFF_HI       35
#define  R_PPC_SECTOFF_HA       36

/*
 * TLS relocations
 */
#define R_PPC_TLS             67
#define R_PPC_DTPMOD32        68
#define R_PPC_TPREL16         69
#define R_PPC_TPREL16_LO      70
#define R_PPC_TPREL16_HI      71
#define R_PPC_TPREL16_HA      72
#define R_PPC_TPREL32         73
#define R_PPC_DTPREL16        74
#define R_PPC_DTPREL16_LO     75
#define R_PPC_DTPREL16_HI     76
#define R_PPC_DTPREL16_HA     77
#define R_PPC_DTPREL32        78
#define R_PPC_GOT_TLSGD16     79
#define R_PPC_GOT_TLSGD16_LO  80
#define R_PPC_GOT_TLSGD16_HI  81
#define R_PPC_GOT_TLSGD16_HA  82
#define R_PPC_GOT_TLSLD16     83
#define R_PPC_GOT_TLSLD16_LO  84
#define R_PPC_GOT_TLSLD16_HI  85
#define R_PPC_GOT_TLSLD16_HA  86
#define R_PPC_GOT_TPREL16     87
#define R_PPC_GOT_TPREL16_LO  88
#define R_PPC_GOT_TPREL16_HI  89
#define R_PPC_GOT_TPREL16_HA  90

/*
 * The remaining relocs are from the Embedded ELF ABI, and are not in the
 *  SVR4 ELF ABI.
 */

#define  R_PPC_EMB_NADDR32     101
#define  R_PPC_EMB_NADDR16     102
#define  R_PPC_EMB_NADDR16_LO  103
#define  R_PPC_EMB_NADDR16_HI  104
#define  R_PPC_EMB_NADDR16_HA  105
#define  R_PPC_EMB_SDAI16      106
#define  R_PPC_EMB_SDA2I16     107
#define  R_PPC_EMB_SDA2REL     108
#define  R_PPC_EMB_SDA21       109
#define  R_PPC_EMB_MRKREF      110
#define  R_PPC_EMB_RELSEC16    111
#define  R_PPC_EMB_RELST_LO    112
#define  R_PPC_EMB_RELST_HI    113
#define  R_PPC_EMB_RELST_HA    114
#define  R_PPC_EMB_BIT_FLD     115
#define  R_PPC_EMB_RELSDA      116

#define  R_SPARC_NONE           0
#define  R_SPARC_8              1
#define  R_SPARC_16             2
#define  R_SPARC_32             3
#define  R_SPARC_DISP8          4
#define  R_SPARC_DISP16         5
#define  R_SPARC_DISP32         6
#define  R_SPARC_WDISP30        7
#define  R_SPARC_WDISP22        8
#define  R_SPARC_HI22           9
#define  R_SPARC_22             10
#define  R_SPARC_13             11
#define  R_SPARC_LO10           12
#define  R_SPARC_GOT10          13
#define  R_SPARC_GOT13          14
#define  R_SPARC_GOT22          15
#define  R_SPARC_PC10           16
#define  R_SPARC_PC22           17
#define  R_SPARC_WPLT30         18
#define  R_SPARC_COPY           19
#define  R_SPARC_GLOB_DAT       20
#define  R_SPARC_JMP_SLOT       21
#define  R_SPARC_RELATIVE       22
#define  R_SPARC_UA32           23
#define  R_SPARC_PLT32          24
#define  R_SPARC_HIPLT22        25
#define  R_SPARC_LOPLT10        26
#define  R_SPARC_PCPLT32        27
#define  R_SPARC_PCPLT22        28
#define  R_SPARC_PCPLT10        29
#define  R_SPARC_10             30
#define  R_SPARC_11             31
#define  R_SPARC_64             32
#define  R_SPARC_OLO10          33
#define  R_SPARC_HH22           34
#define  R_SPARC_HM10           35
#define  R_SPARC_LM22           36
#define  R_SPARC_PC_HH22        37
#define  R_SPARC_PC_HM10        38
#define  R_SPARC_PC_LM22        39
#define  R_SPARC_WDISP16        40
#define  R_SPARC_WDISP19        41
#define  R_SPARC_GLOB_JMP       42
#define  R_SPARC_7              43
#define  R_SPARC_5              44
#define  R_SPARC_6              45
#define  R_SPARC_DISP64         46
#define  R_SPARC_PLT64          47
#define  R_SPARC_HIX22          48
#define  R_SPARC_LOX10          49
#define  R_SPARC_H44            50
#define  R_SPARC_M44            51
#define  R_SPARC_L44            52
#define  R_SPARC_REGISTER       53
#define  R_SPARC_UA64           54
#define  R_SPARC_UA16           55
#define  R_SPARC_TLS_GD_HI22    56
#define  R_SPARC_TLS_GD_LO10    57
#define  R_SPARC_TLS_GD_ADD     58
#define  R_SPARC_TLS_GD_CALL    59
#define  R_SPARC_TLS_LDM_HI22   60
#define  R_SPARC_TLS_LDM_LO10   61
#define  R_SPARC_TLS_LDM_ADD    62
#define  R_SPARC_TLS_LDM_CALL   63
#define  R_SPARC_TLS_LDO_HIX22  64
#define  R_SPARC_TLS_LDO_LOX10  65
#define  R_SPARC_TLS_LDO_ADD    66
#define  R_SPARC_TLS_IE_HI22    67
#define  R_SPARC_TLS_IE_LO10    68
#define  R_SPARC_TLS_IE_LD      69
#define  R_SPARC_TLS_IE_LDX     70
#define  R_SPARC_TLS_IE_ADD     71
#define  R_SPARC_TLS_LE_HIX22   72
#define  R_SPARC_TLS_LE_LOX10   73
#define  R_SPARC_TLS_DTPMOD32   74
#define  R_SPARC_TLS_DTPMOD64   75
#define  R_SPARC_TLS_DTPOFF32   76
#define  R_SPARC_TLS_DTPOFF64   77
#define  R_SPARC_TLS_TPOFF32    78
#define  R_SPARC_TLS_TPOFF64    79

#define  R_X86_64_NONE             0  /* No relocation. */
#define  R_X86_64_64               1  /* Add 64 bit symbol value. */
#define  R_X86_64_PC32             2  /* PC-relative 32 bit signed sym value. */
#define  R_X86_64_GOT32            3  /* PC-relative 32 bit GOT offset. */
#define  R_X86_64_PLT32            4  /* PC-relative 32 bit PLT offset. */
#define  R_X86_64_COPY             5  /* Copy data from shared object. */
#define  R_X86_64_GLOB_DAT         6  /* Set GOT entry to data address. */
#define  R_X86_64_JMP_SLOT         7  /* Set GOT entry to code address. */
#define  R_X86_64_RELATIVE         8  /* Add load address of shared object. */
#define  R_X86_64_GOTPCREL         9  /* Add 32 bit signed pcrel offset to GOT. */
#define  R_X86_64_32               10 /* Add 32 bit zero extended symbol value */
#define  R_X86_64_32S              11 /* Add 32 bit sign extended symbol value */
#define  R_X86_64_16               12 /* Add 16 bit zero extended symbol value */
#define  R_X86_64_PC16             13 /* Add 16 bit signed extended pc relative symbol value */
#define  R_X86_64_8                14 /* Add 8 bit zero extended symbol value */
#define  R_X86_64_PC8              15 /* Add 8 bit signed extended pc relative symbol value */
#define  R_X86_64_DTPMOD64         16 /* ID of module containing symbol */
#define  R_X86_64_DTPOFF64         17 /* Offset in TLS block */
#define  R_X86_64_TPOFF64          18 /* Offset in static TLS block */
#define  R_X86_64_TLSGD            19 /* PC relative offset to GD GOT entry */
#define  R_X86_64_TLSLD            20 /* PC relative offset to LD GOT entry */
#define  R_X86_64_DTPOFF32         21 /* Offset in TLS block */
#define  R_X86_64_GOTTPOFF         22 /* PC relative offset to IE GOT entry */
#define  R_X86_64_TPOFF32          23 /* Offset in static TLS block */
#define  R_X86_64_PC64             24 /* PC relative 64 bit */
#define  R_X86_64_GOTOFF64         25 /* 64 bit offset to GOT */
#define  R_X86_64_GOTPC3           26 /* 32 bit signed pc relative offset to GOT */
#define  R_X86_64_GOT64            27 /* 64-bit GOT entry offset */
#define  R_X86_64_GOTPCREL64       28 /* 64-bit PC relative offset to GOT entry */
#define  R_X86_64_GOTPC64          29 /* 64-bit PC relative offset to GOT */
#define  R_X86_64_GOTPLT64         30 /* like GOT64, says PLT entry needed */
#define  R_X86_64_PLTOFF64         31 /* 64-bit GOT relative offset to PLT entry */
#define  R_X86_64_SIZE32           32 /* Size of symbol plus 32-bit addend */
#define  R_X86_64_SIZE64           33 /* Size of symbol plus 64-bit addend */
#define  R_X86_64_GOTPC32_TLSDESC  34 /* GOT offset for TLS descriptor. */
#define  R_X86_64_TLSDESC_CALL     35 /* Marker for call through TLS descriptor. */
#define  R_X86_64_TLSDESC          36 /* TLS descriptor. */
#define  R_X86_64_IRELATIVE        37 /* Adjust indirectly by program base */
#define  R_X86_64_RELATIVE64       38 /* 64-bit adjust by program base */
#define  R_X86_64_GOTPCRELX        41 /* Load from 32 bit signed pc relative offset to GOT entry without REX prefix, relaxable. */
#define  R_X86_64_REX_GOTPCRELX    42 /* Load from 32 bit signed pc relative offset to GOT entry with REX prefix, relaxable. */


/*
 * ELF definitions common to all 32-bit architectures.
 */

typedef UINT32  Elf32_Addr;
typedef UINT16  Elf32_Half;
typedef UINT32  Elf32_Off;
typedef INT32   Elf32_Sword;
typedef UINT32  Elf32_Word;
typedef UINT64  Elf32_Lword;

typedef Elf32_Word Elf32_Hashelt;

/* Non-standard class-dependent datatype used for abstraction. */
typedef Elf32_Word  Elf32_Size;
typedef Elf32_Sword Elf32_Ssize;

/*
 * ELF header.
 */

typedef struct {
    unsigned char    e_ident[EI_NIDENT]; /* File identification. */
    Elf32_Half       e_type;             /* File type. */
    Elf32_Half       e_machine;          /* Machine architecture. */
    Elf32_Word       e_version;          /* ELF format version. */
    Elf32_Addr       e_entry;            /* Entry point. */
    Elf32_Off        e_phoff;            /* Program header file offset. */
    Elf32_Off        e_shoff;            /* Section header file offset. */
    Elf32_Word       e_flags;            /* Architecture-specific flags. */
    Elf32_Half       e_ehsize;           /* Size of ELF header in bytes. */
    Elf32_Half       e_phentsize;        /* Size of program header entry. */
    Elf32_Half       e_phnum;            /* Number of program header entries. */
    Elf32_Half       e_shentsize;        /* Size of section header entry. */
    Elf32_Half       e_shnum;            /* Number of section header entries. */
    Elf32_Half       e_shstrndx;         /* Section name strings section. */
} Elf32_Ehdr;

/*
 * Section header.
 */

typedef struct {
    Elf32_Word    sh_name;      /* Section name (index into the
             section header string table). */
    Elf32_Word    sh_type;      /* Section type. */
    Elf32_Word    sh_flags;     /* Section flags. */
    Elf32_Addr    sh_addr;      /* Address in memory image. */
    Elf32_Off     sh_offset;    /* Offset in file. */
    Elf32_Word    sh_size;      /* Size in bytes. */
    Elf32_Word    sh_link;      /* Index of a related section. */
    Elf32_Word    sh_info;      /* Depends on section type. */
    Elf32_Word    sh_addralign; /* Alignment in bytes. */
    Elf32_Word    sh_entsize;   /* Size of each entry in section. */
} Elf32_Shdr;

/*
 * Program header.
 */

typedef struct {
    Elf32_Word    p_type;   /* Entry type. */
    Elf32_Off     p_offset; /* File offset of contents. */
    Elf32_Addr    p_vaddr;  /* Virtual address in memory image. */
    Elf32_Addr    p_paddr;  /* Physical address (not used). */
    Elf32_Word    p_filesz; /* Size of contents in file. */
    Elf32_Word    p_memsz;  /* Size of contents in memory. */
    Elf32_Word    p_flags;  /* Access permission flags. */
    Elf32_Word    p_align;  /* Alignment in memory and file. */
} Elf32_Phdr;

/*
 * Dynamic structure.  The ".dynamic" section contains an array of them.
 */

typedef struct {
    Elf32_Sword    d_tag;  /* Entry type. */
    union {
        Elf32_Word    d_val; /* Integer value. */
        Elf32_Addr    d_ptr; /* Address value. */
    } d_un;
} Elf32_Dyn;

/*
 * Relocation entries.
 */

/* Relocations that don't need an addend field. */
typedef struct {
    Elf32_Addr    r_offset; /* Location to be relocated. */
    Elf32_Word    r_info;   /* Relocation type and symbol index. */
} Elf32_Rel;

/* Relocations that need an addend field. */
typedef struct {
    Elf32_Addr     r_offset; /* Location to be relocated. */
    Elf32_Word     r_info;   /* Relocation type and symbol index. */
    Elf32_Sword    r_addend; /* Addend. */
} Elf32_Rela;

/* Macros for accessing the fields of r_info. */
#define ELF32_R_SYM(info)   ((info) >> 8)
#define ELF32_R_TYPE(info)  ((unsigned char)(info))

/* Macro for constructing r_info from field values. */
#define ELF32_R_INFO(sym, type)  (((sym) << 8) + (unsigned char)(type))

/*
 *  Note entry header
 */
typedef Elf_Note Elf32_Nhdr;

/*
 *  Move entry
 */
typedef struct {
    Elf32_Lword    m_value;   /* symbol value */
    Elf32_Word     m_info;    /* size + index */
    Elf32_Word     m_poffset; /* symbol offset */
    Elf32_Half     m_repeat;  /* repeat count */
    Elf32_Half     m_stride;  /* stride info */
} Elf32_Move;

/*
 *  The macros compose and decompose values for Move.r_info
 *
 *  sym = ELF32_M_SYM(M.m_info)
 *  size = ELF32_M_SIZE(M.m_info)
 *  M.m_info = ELF32_M_INFO(sym, size)
 */
#define  ELF32_M_SYM(info)        ((info)>>8)
#define  ELF32_M_SIZE(info)       ((unsigned char)(info))
#define  ELF32_M_INFO(sym, size)  (((sym)<<8)+(unsigned char)(size))

/*
 *  Hardware/Software capabilities entry
 */
typedef struct {
    Elf32_Word    c_tag;  /* how to interpret value */
    union {
        Elf32_Word    c_val;
        Elf32_Addr    c_ptr;
    } c_un;
} Elf32_Cap;

/*
 * Symbol table entries.
 */

typedef struct {
    Elf32_Word       st_name;  /* String table index of name. */
    Elf32_Addr       st_value; /* Symbol value. */
    Elf32_Word       st_size;  /* Size of associated object. */
    unsigned char    st_info;  /* Type and binding information. */
    unsigned char    st_other; /* Reserved (not used). */
    Elf32_Half       st_shndx; /* Section index of symbol. */
} Elf32_Sym;

/* Macros for accessing the fields of st_info. */
#define ELF32_ST_BIND(info)  ((info) >> 4)
#define ELF32_ST_TYPE(info)  ((info) & 0xf)

/* Macro for constructing st_info from field values. */
#define ELF32_ST_INFO(bind, type)  (((bind) << 4) + ((type) & 0xf))

/* Macro for accessing the fields of st_other. */
#define ELF32_ST_VISIBILITY(oth)  ((oth) & 0x3)

/* Structures used by Sun & GNU symbol versioning. */
typedef struct {
    Elf32_Half    vd_version;
    Elf32_Half    vd_flags;
    Elf32_Half    vd_ndx;
    Elf32_Half    vd_cnt;
    Elf32_Word    vd_hash;
    Elf32_Word    vd_aux;
    Elf32_Word    vd_next;
} Elf32_Verdef;

typedef struct {
    Elf32_Word    vda_name;
    Elf32_Word    vda_next;
} Elf32_Verdaux;

typedef struct {
    Elf32_Half    vn_version;
    Elf32_Half    vn_cnt;
    Elf32_Word    vn_file;
    Elf32_Word    vn_aux;
    Elf32_Word    vn_next;
} Elf32_Verneed;

typedef struct {
    Elf32_Word    vna_hash;
    Elf32_Half    vna_flags;
    Elf32_Half    vna_other;
    Elf32_Word    vna_name;
    Elf32_Word    vna_next;
} Elf32_Vernaux;

typedef Elf32_Half Elf32_Versym;

typedef struct {
    Elf32_Half    si_boundto; /* direct bindings - symbol bound to */
    Elf32_Half    si_flags;   /* per symbol flags */
} Elf32_Syminfo;

/*
 * ELF definitions common to all 64-bit architectures.
 */

typedef UINT64  Elf64_Addr;
typedef UINT16  Elf64_Half;
typedef UINT64  Elf64_Off;
typedef INT32   Elf64_Sword;
typedef INT64   Elf64_Sxword;
typedef UINT32  Elf64_Word;
typedef UINT64  Elf64_Lword;
typedef UINT64  Elf64_Xword;

/*
 * Types of dynamic symbol hash table bucket and chain elements.
 *
 * This is inconsistent among 64 bit architectures, so a machine dependent
 * typedef is required.
 */

typedef Elf64_Word Elf64_Hashelt;

/* Non-standard class-dependent datatype used for abstraction. */
typedef Elf64_Xword  Elf64_Size;
typedef Elf64_Sxword Elf64_Ssize;

/*
 * ELF header.
 */

typedef struct {
    unsigned char    e_ident[EI_NIDENT]; /* File identification. */
    Elf64_Half       e_type;             /* File type. */
    Elf64_Half       e_machine;          /* Machine architecture. */
    Elf64_Word       e_version;          /* ELF format version. */
    Elf64_Addr       e_entry;            /* Entry point. */
    Elf64_Off        e_phoff;            /* Program header file offset. */
    Elf64_Off        e_shoff;            /* Section header file offset. */
    Elf64_Word       e_flags;            /* Architecture-specific flags. */
    Elf64_Half       e_ehsize;           /* Size of ELF header in bytes. */
    Elf64_Half       e_phentsize;        /* Size of program header entry. */
    Elf64_Half       e_phnum;            /* Number of program header entries. */
    Elf64_Half       e_shentsize;        /* Size of section header entry. */
    Elf64_Half       e_shnum;            /* Number of section header entries. */
    Elf64_Half       e_shstrndx;         /* Section name strings section. */
} Elf64_Ehdr;

/*
 * Section header.
 */

typedef struct {
    Elf64_Word     sh_name;      /* Section name (index into the
             section header string table). */
    Elf64_Word     sh_type;      /* Section type. */
    Elf64_Xword    sh_flags;     /* Section flags. */
    Elf64_Addr     sh_addr;      /* Address in memory image. */
    Elf64_Off      sh_offset;    /* Offset in file. */
    Elf64_Xword    sh_size;      /* Size in bytes. */
    Elf64_Word     sh_link;      /* Index of a related section. */
    Elf64_Word     sh_info;      /* Depends on section type. */
    Elf64_Xword    sh_addralign; /* Alignment in bytes. */
    Elf64_Xword    sh_entsize;   /* Size of each entry in section. */
} Elf64_Shdr;

/*
 * Program header.
 */

typedef struct {
    Elf64_Word     p_type;   /* Entry type. */
    Elf64_Word     p_flags;  /* Access permission flags. */
    Elf64_Off      p_offset; /* File offset of contents. */
    Elf64_Addr     p_vaddr;  /* Virtual address in memory image. */
    Elf64_Addr     p_paddr;  /* Physical address (not used). */
    Elf64_Xword    p_filesz; /* Size of contents in file. */
    Elf64_Xword    p_memsz;  /* Size of contents in memory. */
    Elf64_Xword    p_align;  /* Alignment in memory and file. */
} Elf64_Phdr;

/*
 * Dynamic structure.  The ".dynamic" section contains an array of them.
 */

typedef struct {
    Elf64_Sxword    d_tag;  /* Entry type. */
    union {
        Elf64_Xword    d_val; /* Integer value. */
        Elf64_Addr     d_ptr; /* Address value. */
    } d_un;
} Elf64_Dyn;

/*
 * Relocation entries.
 */

/* Relocations that don't need an addend field. */
typedef struct {
    Elf64_Addr     r_offset; /* Location to be relocated. */
    Elf64_Xword    r_info;   /* Relocation type and symbol index. */
} Elf64_Rel;

/* Relocations that need an addend field. */
typedef struct {
    Elf64_Addr      r_offset; /* Location to be relocated. */
    Elf64_Xword     r_info;   /* Relocation type and symbol index. */
    Elf64_Sxword    r_addend; /* Addend. */
} Elf64_Rela;

/* Macros for accessing the fields of r_info. */
#define ELF64_R_SYM(info)   ((UINT32) RShiftU64 ((info), 32))
#define ELF64_R_TYPE(info)  ((info) & 0xffffffffL)

/* Macro for constructing r_info from field values. */
#define ELF64_R_INFO(sym, type)  (((sym) << 32) + ((type) & 0xffffffffL))

#define  ELF64_R_TYPE_DATA(info)  (((Elf64_Xword)(info)<<32)>>40)
#define  ELF64_R_TYPE_ID(info)    (((Elf64_Xword)(info)<<56)>>56)
#define  ELF64_R_TYPE_INFO(data, type)  \
(((Elf64_Xword)(data)<<8)+(Elf64_Xword)(type))

    /*
 *  Note entry header
 */
    typedef Elf_Note Elf64_Nhdr;

/*
 *  Move entry
 */
typedef struct {
    Elf64_Lword    m_value;   /* symbol value */
    Elf64_Xword    m_info;    /* size + index */
    Elf64_Xword    m_poffset; /* symbol offset */
    Elf64_Half     m_repeat;  /* repeat count */
    Elf64_Half     m_stride;  /* stride info */
} Elf64_Move;

#define  ELF64_M_SYM(info)        ((info)>>8)
#define  ELF64_M_SIZE(info)       ((unsigned char)(info))
#define  ELF64_M_INFO(sym, size)  (((sym)<<8)+(unsigned char)(size))

/*
 *  Hardware/Software capabilities entry
 */
typedef struct {
    Elf64_Xword    c_tag;  /* how to interpret value */
    union {
        Elf64_Xword    c_val;
        Elf64_Addr     c_ptr;
    } c_un;
} Elf64_Cap;

/*
 * Symbol table entries.
 */

typedef struct {
    Elf64_Word       st_name;  /* String table index of name. */
    unsigned char    st_info;  /* Type and binding information. */
    unsigned char    st_other; /* Reserved (not used). */
    Elf64_Half       st_shndx; /* Section index of symbol. */
    Elf64_Addr       st_value; /* Symbol value. */
    Elf64_Xword      st_size;  /* Size of associated object. */
} Elf64_Sym;

/* Macros for accessing the fields of st_info. */
#define ELF64_ST_BIND(info)  ((info) >> 4)
#define ELF64_ST_TYPE(info)  ((info) & 0xf)

/* Macro for constructing st_info from field values. */
#define ELF64_ST_INFO(bind, type)  (((bind) << 4) + ((type) & 0xf))

/* Macro for accessing the fields of st_other. */
#define ELF64_ST_VISIBILITY(oth)  ((oth) & 0x3)

/* Structures used by Sun & GNU-style symbol versioning. */
typedef struct {
    Elf64_Half    vd_version;
    Elf64_Half    vd_flags;
    Elf64_Half    vd_ndx;
    Elf64_Half    vd_cnt;
    Elf64_Word    vd_hash;
    Elf64_Word    vd_aux;
    Elf64_Word    vd_next;
} Elf64_Verdef;

typedef struct {
    Elf64_Word    vda_name;
    Elf64_Word    vda_next;
} Elf64_Verdaux;

typedef struct {
    Elf64_Half    vn_version;
    Elf64_Half    vn_cnt;
    Elf64_Word    vn_file;
    Elf64_Word    vn_aux;
    Elf64_Word    vn_next;
} Elf64_Verneed;

typedef struct {
    Elf64_Word    vna_hash;
    Elf64_Half    vna_flags;
    Elf64_Half    vna_other;
    Elf64_Word    vna_name;
    Elf64_Word    vna_next;
} Elf64_Vernaux;

typedef Elf64_Half Elf64_Versym;

typedef struct {
    Elf64_Half    si_boundto; /* direct bindings - symbol bound to */
    Elf64_Half    si_flags;   /* per symbol flags */
} Elf64_Syminfo;

#endif // ELF_H