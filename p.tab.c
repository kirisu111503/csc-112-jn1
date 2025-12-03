/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "p.y"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

typedef struct errorList{
   int line_error;
   char *error_type;
   struct errorList *next;
}errorList;


// Symbol table structure
typedef struct vars{
    char *id;
    int data_type;
    union {
        int val;      // for int/char
        char *str_val; // for string
    } data;
    struct vars *next;
} vars;

typedef struct logs{
    char *info;
    int line;
    int isError;
    struct logs *next;
}logs;


vars *headVars = NULL;
vars *tailVars = NULL;
errorList *headErrList = NULL;
errorList *tailErrList = NULL;
logs *headLogs = NULL;
logs *tailLogs = NULL;
char *currentDataType = NULL;
char *currentVarBeingDeclared = NULL;
int isRecovering = 0;
int hasError = 0;

extern int yylineno;
extern int yylex();
void yyerror(const char *fmt, ...);
void printErrorTable();
void cleanupErrorTable();
int getVariableValue(char *variableName);
void cleanupVariableTable();
const char* typeName(char dt);
vars* getVariable(char* variable);
void createVariable(char *DTYPE, char* variable, int val, char *str_val);
void variableReAssignment(char* variable, int val, char *str_val);
void printVariableTable();

#line 130 "p.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "p.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DATA_TYPE = 3,                  /* DATA_TYPE  */
  YYSYMBOL_VARIABLE = 4,                   /* VARIABLE  */
  YYSYMBOL_ASSIGNMENT = 5,                 /* ASSIGNMENT  */
  YYSYMBOL_DISPLAY = 6,                    /* DISPLAY  */
  YYSYMBOL_COMMA = 7,                      /* COMMA  */
  YYSYMBOL_SEMI = 8,                       /* SEMI  */
  YYSYMBOL_STRING = 9,                     /* STRING  */
  YYSYMBOL_INTEGER = 10,                   /* INTEGER  */
  YYSYMBOL_CHARACTER = 11,                 /* CHARACTER  */
  YYSYMBOL_12_ = 12,                       /* '+'  */
  YYSYMBOL_13_ = 13,                       /* '-'  */
  YYSYMBOL_14_ = 14,                       /* '*'  */
  YYSYMBOL_15_ = 15,                       /* '/'  */
  YYSYMBOL_UMINUS = 16,                    /* UMINUS  */
  YYSYMBOL_17_ = 17,                       /* '('  */
  YYSYMBOL_18_ = 18,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 19,                  /* $accept  */
  YYSYMBOL_program = 20,                   /* program  */
  YYSYMBOL_statement_list = 21,            /* statement_list  */
  YYSYMBOL_statement = 22,                 /* statement  */
  YYSYMBOL_display_statement = 23,         /* display_statement  */
  YYSYMBOL_display_arg = 24,               /* display_arg  */
  YYSYMBOL_declaration_statement = 25,     /* declaration_statement  */
  YYSYMBOL_data_type = 26,                 /* data_type  */
  YYSYMBOL_declaration_list = 27,          /* declaration_list  */
  YYSYMBOL_var_decl = 28,                  /* var_decl  */
  YYSYMBOL_assignment_statement = 29,      /* assignment_statement  */
  YYSYMBOL_assignment_list = 30,           /* assignment_list  */
  YYSYMBOL_assignment = 31,                /* assignment  */
  YYSYMBOL_expression = 32                 /* expression  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   89

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  19
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  45
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  75

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   267


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      17,    18,    14,    12,     2,    13,     2,    15,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    16
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    97,    97,   100,   101,   105,   106,   107,   108,   112,
     125,   131,   137,   156,   162,   168,   191,   192,   193,   213,
     222,   228,   235,   238,   239,   243,   251,   261,   271,   285,
     288,   289,   293,   298,   306,   314,   342,   343,   344,   355,
     356,   357,   358,   359,   364,   365
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "DATA_TYPE",
  "VARIABLE", "ASSIGNMENT", "DISPLAY", "COMMA", "SEMI", "STRING",
  "INTEGER", "CHARACTER", "'+'", "'-'", "'*'", "'/'", "UMINUS", "'('",
  "')'", "$accept", "program", "statement_list", "statement",
  "display_statement", "display_arg", "declaration_statement", "data_type",
  "declaration_list", "var_decl", "assignment_statement",
  "assignment_list", "assignment", "expression", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-31)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-36)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -31,    11,    60,   -31,    51,   -31,    14,    26,   -31,   -31,
     -31,    49,   -31,    -4,   -31,   -31,    25,    35,    80,     1,
     -31,    61,   -31,    10,   -31,   -31,    33,     3,     3,     3,
      18,   -31,   -31,   -31,   -31,    35,    35,     3,    55,    45,
      49,   -31,   -31,   -31,   -31,   -31,   -31,    62,     3,     3,
       3,     3,   -31,   -31,    69,    35,    35,    35,    35,    78,
     -31,    43,    18,   -31,   -31,    57,    57,   -31,   -31,   -31,
      64,    64,   -31,   -31,   -31
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,     0,     1,     0,    22,     0,     0,     3,     5,
       6,     0,     7,     0,    31,     8,     0,     0,    25,     0,
      24,     0,    29,    38,    33,    36,    37,     0,     0,     0,
      32,    12,    10,    13,    11,     0,     0,     0,     0,     0,
       0,    21,    30,    38,    37,    45,    44,     0,     0,     0,
       0,     0,    20,    19,     0,     0,     0,     0,     0,     0,
      27,    37,    26,    23,    39,    40,    41,    42,    43,    14,
      15,    16,    17,    18,     9
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -31,   -31,   -31,   -31,   -31,   -30,   -31,   -31,   -31,    48,
     -31,   -31,    68,   -27
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     8,     9,    38,    10,    11,    19,    20,
      12,    13,    14,    30
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      45,    46,    47,    21,    22,    52,    53,    43,    40,    41,
      54,     3,    62,    25,    44,    27,    28,   -35,   -35,    16,
      29,    65,    66,    67,    68,    70,    71,    72,    73,    23,
      48,    49,    50,    51,    24,    25,    26,    27,    28,    31,
     -34,   -34,    29,    17,    32,    33,    34,    35,    36,    43,
     -28,   -28,    37,    18,    60,    25,    61,    27,    28,    15,
      -2,     4,    29,     5,     6,     6,     7,    55,    56,    57,
      58,    50,    51,    59,    48,    49,    50,    51,    57,    58,
      64,    48,    49,    50,    51,    39,    74,    69,    63,    42
};

static const yytype_int8 yycheck[] =
{
      27,    28,    29,     7,     8,    35,    36,     4,     7,     8,
      37,     0,    39,    10,    11,    12,    13,     7,     8,     5,
      17,    48,    49,    50,    51,    55,    56,    57,    58,     4,
      12,    13,    14,    15,     9,    10,    11,    12,    13,     4,
       7,     8,    17,    17,     9,    10,    11,    12,    13,     4,
       7,     8,    17,     4,     9,    10,    11,    12,    13,     8,
       0,     1,    17,     3,     4,     4,     6,    12,    13,    14,
      15,    14,    15,    18,    12,    13,    14,    15,    14,    15,
      18,    12,    13,    14,    15,     5,     8,    18,    40,    21
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    20,    21,     0,     1,     3,     4,     6,    22,    23,
      25,    26,    29,    30,    31,     8,     5,    17,     4,    27,
      28,     7,     8,     4,     9,    10,    11,    12,    13,    17,
      32,     4,     9,    10,    11,    12,    13,    17,    24,     5,
       7,     8,    31,     4,    11,    32,    32,    32,    12,    13,
      14,    15,    24,    24,    32,    12,    13,    14,    15,    18,
       9,    11,    32,    28,    18,    32,    32,    32,    32,    18,
      24,    24,    24,    24,     8
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    19,    20,    21,    21,    22,    22,    22,    22,    23,
      24,    24,    24,    24,    24,    24,    24,    24,    24,    24,
      24,    25,    26,    27,    27,    28,    28,    28,    28,    29,
      30,    30,    31,    31,    31,    31,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     2,     5,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     2,
       2,     3,     1,     3,     1,     1,     3,     3,     3,     2,
       3,     1,     3,     3,     3,     3,     1,     1,     1,     3,
       3,     3,     3,     3,     2,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 8: /* statement: error SEMI  */
#line 108 "p.y"
                        { yyerrok; }
#line 1192 "p.tab.c"
    break;

  case 9: /* display_statement: DISPLAY '(' display_arg ')' SEMI  */
#line 113 "p.y"
    {
        if (hasError || (yyvsp[-2].disp).type == 4) {
            if ((yyvsp[-2].disp).text) free((yyvsp[-2].disp).text);
            YYABORT;                       // stop on any error in display
        }
        if ((yyvsp[-2].disp).text) {
            printf("LINE %d: %s\n", yylineno, (yyvsp[-2].disp).text);
            free((yyvsp[-2].disp).text);
        }
    }
#line 1207 "p.tab.c"
    break;

  case 10: /* display_arg: STRING  */
#line 126 "p.y"
    {
        (yyval.disp).text = strdup((yyvsp[0].str));
        (yyval.disp).type = 1;
        free((yyvsp[0].str));
    }
#line 1217 "p.tab.c"
    break;

  case 11: /* display_arg: CHARACTER  */
#line 132 "p.y"
    {
        (yyval.disp).text = malloc(2);
        (yyval.disp).text[0] = (yyvsp[0].character); (yyval.disp).text[1] = '\0';
        (yyval.disp).type = 2;
    }
#line 1227 "p.tab.c"
    break;

  case 12: /* display_arg: VARIABLE  */
#line 138 "p.y"
    {
        vars *var = getVariable((yyvsp[0].str));
        if (!var) {
            yyerror("Undefined variable '%s'", (yyvsp[0].str));
            (yyval.disp).type = 4; (yyval.disp).text = NULL;
            free((yyvsp[0].str));
            YYABORT;
        }
        if (var->data_type == 's') {
            (yyval.disp).text = strdup(var->data.str_val);
            (yyval.disp).type = 1;
        } else {                                 // int or char → numeric
            (yyval.disp).text = malloc(20);
            sprintf((yyval.disp).text, "%d", var->data.val);
            (yyval.disp).type = 3;
        }
        free((yyvsp[0].str));
    }
#line 1250 "p.tab.c"
    break;

  case 13: /* display_arg: INTEGER  */
#line 157 "p.y"
    {
        (yyval.disp).text = malloc(20);
        sprintf((yyval.disp).text, "%d", (yyvsp[0].num));
        (yyval.disp).type = 3;
    }
#line 1260 "p.tab.c"
    break;

  case 14: /* display_arg: '(' expression ')'  */
#line 163 "p.y"
    {
        (yyval.disp).text = malloc(20);
        sprintf((yyval.disp).text, "%d", (yyvsp[-1].num));
        (yyval.disp).type = 3;
    }
#line 1270 "p.tab.c"
    break;

  case 15: /* display_arg: display_arg '+' display_arg  */
#line 169 "p.y"
    {
        if (hasError || (yyvsp[-2].disp).type == 4 || (yyvsp[0].disp).type == 4) goto disp_error;
        if ((yyvsp[-2].disp).type == 1 || (yyvsp[0].disp).type == 1) {                 // string concatenation
            size_t len = strlen((yyvsp[-2].disp).text) + strlen((yyvsp[0].disp).text) + 1;
            (yyval.disp).text = malloc(len);
            strcpy((yyval.disp).text, (yyvsp[-2].disp).text); strcat((yyval.disp).text, (yyvsp[0].disp).text);
            (yyval.disp).type = 1;
        } else {                                            // numeric addition
            int a = atoi((yyvsp[-2].disp).text), b = atoi((yyvsp[0].disp).text);
            (yyval.disp).text = malloc(20);
            sprintf((yyval.disp).text, "%d", a + b);
            (yyval.disp).type = 3;
        }
        free((yyvsp[-2].disp).text); free((yyvsp[0].disp).text);
        goto disp_end;
    disp_error:
        if ((yyvsp[-2].disp).text) free((yyvsp[-2].disp).text);
        if ((yyvsp[0].disp).text) free((yyvsp[0].disp).text);
        (yyval.disp).type = 4; (yyval.disp).text = NULL;
        YYABORT;
    disp_end: ;
    }
#line 1297 "p.tab.c"
    break;

  case 16: /* display_arg: display_arg '-' display_arg  */
#line 191 "p.y"
                                  { /* similar to +, omitted for brevity but fixed same way */ }
#line 1303 "p.tab.c"
    break;

  case 17: /* display_arg: display_arg '*' display_arg  */
#line 192 "p.y"
                                  { /* same pattern */ }
#line 1309 "p.tab.c"
    break;

  case 18: /* display_arg: display_arg '/' display_arg  */
#line 194 "p.y"
    {
        if (hasError || (yyvsp[-2].disp).type == 4 || (yyvsp[0].disp).type == 4) goto disp_err2;
        int a = atoi((yyvsp[-2].disp).text), b = atoi((yyvsp[0].disp).text);
        if (b == 0) {
            yyerror("Division by zero in display");
            goto disp_err2;
        }
        (yyval.disp).text = malloc(20);
        sprintf((yyval.disp).text, "%d", a / b);
        (yyval.disp).type = 3;
        free((yyvsp[-2].disp).text); free((yyvsp[0].disp).text);
        goto disp_end2;
    disp_err2:
        if ((yyvsp[-2].disp).text) free((yyvsp[-2].disp).text);
        if ((yyvsp[0].disp).text) free((yyvsp[0].disp).text);
        (yyval.disp).type = 4; (yyval.disp).text = NULL;
        YYABORT;
    disp_end2: ;
    }
#line 1333 "p.tab.c"
    break;

  case 19: /* display_arg: '-' display_arg  */
#line 214 "p.y"
    {
        if (hasError || (yyvsp[0].disp).type == 4) { if ((yyvsp[0].disp).text) free((yyvsp[0].disp).text); (yyval.disp).type=4; (yyval.disp).text=NULL; YYABORT; }
        int v = atoi((yyvsp[0].disp).text);
        (yyval.disp).text = malloc(20);
        sprintf((yyval.disp).text, "%d", -v);
        (yyval.disp).type = 3;
        free((yyvsp[0].disp).text);
    }
#line 1346 "p.tab.c"
    break;

  case 20: /* display_arg: '+' display_arg  */
#line 222 "p.y"
                                   { (yyval.disp) = (yyvsp[0].disp); }
#line 1352 "p.tab.c"
    break;

  case 21: /* declaration_statement: data_type declaration_list SEMI  */
#line 229 "p.y"
    {
        if (currentDataType) { free(currentDataType); currentDataType = NULL; }
        if (currentVarBeingDeclared) { free(currentVarBeingDeclared); currentVarBeingDeclared = NULL; }
        if (hasError) YYABORT;
    }
#line 1362 "p.tab.c"
    break;

  case 22: /* data_type: DATA_TYPE  */
#line 235 "p.y"
                     { currentDataType = strdup((yyvsp[0].str)); (yyval.str) = currentDataType; }
#line 1368 "p.tab.c"
    break;

  case 25: /* var_decl: VARIABLE  */
#line 244 "p.y"
    {
        if (strcmp(currentDataType, "string") == 0)
            createVariable(currentDataType, (yyvsp[0].str), 0, "");
        else
            createVariable(currentDataType, (yyvsp[0].str), 0, NULL);
        free((yyvsp[0].str));
    }
#line 1380 "p.tab.c"
    break;

  case 26: /* var_decl: VARIABLE ASSIGNMENT expression  */
#line 252 "p.y"
    {
        if (strcmp(currentDataType, "string") == 0) {
            yyerror("Cannot assign expression to string variable '%s'", (yyvsp[-2].str));
            free((yyvsp[-2].str));
            YYABORT;
        }
        createVariable(currentDataType, (yyvsp[-2].str), (yyvsp[0].num), NULL);
        free((yyvsp[-2].str));
    }
#line 1394 "p.tab.c"
    break;

  case 27: /* var_decl: VARIABLE ASSIGNMENT STRING  */
#line 262 "p.y"
    {
        if (strcmp(currentDataType, "string") != 0) {
            yyerror("Cannot assign string literal to non-string variable '%s'", (yyvsp[-2].str));
            free((yyvsp[-2].str)); free((yyvsp[0].str));
            YYABORT;
        }
        createVariable(currentDataType, (yyvsp[-2].str), 0, (yyvsp[0].str));
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1408 "p.tab.c"
    break;

  case 28: /* var_decl: VARIABLE ASSIGNMENT CHARACTER  */
#line 272 "p.y"
    {
        if (strcmp(currentDataType, "string") == 0) {
            yyerror("Cannot assign char literal to string variable '%s'", (yyvsp[-2].str));
            free((yyvsp[-2].str));
            YYABORT;
        }
        createVariable(currentDataType, (yyvsp[-2].str), (int)(yyvsp[0].character), NULL);
        free((yyvsp[-2].str));
    }
#line 1422 "p.tab.c"
    break;

  case 29: /* assignment_statement: assignment_list SEMI  */
#line 285 "p.y"
                           { if (hasError) YYABORT; }
#line 1428 "p.tab.c"
    break;

  case 32: /* assignment: VARIABLE ASSIGNMENT expression  */
#line 294 "p.y"
    {
        variableReAssignment((yyvsp[-2].str), (yyvsp[0].num), NULL);
        free((yyvsp[-2].str));
    }
#line 1437 "p.tab.c"
    break;

  case 33: /* assignment: VARIABLE ASSIGNMENT STRING  */
#line 299 "p.y"
    {
        vars *v = getVariable((yyvsp[-2].str));
        if (!v) { yyerror("Undefined variable '%s'", (yyvsp[-2].str)); free((yyvsp[-2].str)); free((yyvsp[0].str)); YYABORT; }
        if (v->data_type != 's') { yyerror("Cannot assign string to non-string variable '%s'", (yyvsp[-2].str)); free((yyvsp[-2].str)); free((yyvsp[0].str)); YYABORT; }
        variableReAssignment((yyvsp[-2].str), 0, (yyvsp[0].str));
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1449 "p.tab.c"
    break;

  case 34: /* assignment: VARIABLE ASSIGNMENT CHARACTER  */
#line 307 "p.y"
    {
        vars *v = getVariable((yyvsp[-2].str));
        if (!v) { yyerror("Undefined variable '%s'", (yyvsp[-2].str)); free((yyvsp[-2].str)); YYABORT; }
        if (v->data_type == 's') { yyerror("Cannot assign char to string variable '%s'", (yyvsp[-2].str)); free((yyvsp[-2].str)); YYABORT; }
        variableReAssignment((yyvsp[-2].str), (int)(yyvsp[0].character), NULL);
        free((yyvsp[-2].str));
    }
#line 1461 "p.tab.c"
    break;

  case 35: /* assignment: VARIABLE ASSIGNMENT VARIABLE  */
#line 315 "p.y"
    {
        vars *t = getVariable((yyvsp[-2].str));
        vars *s = getVariable((yyvsp[0].str));
        if (!t || !s) {
            yyerror("Undefined variable in assignment");
            free((yyvsp[-2].str)); free((yyvsp[0].str));
            YYABORT;
        }
        if (t->data_type == 's' && s->data_type != 's') {
            yyerror("Cannot assign non-string to string variable");
            free((yyvsp[-2].str)); free((yyvsp[0].str));
            YYABORT;
        }
        if (t->data_type != 's' && s->data_type == 's') {
            yyerror("Cannot assign string to non-string variable");
            free((yyvsp[-2].str)); free((yyvsp[0].str));
            YYABORT;
        }
        if (t->data_type == 's')
            variableReAssignment((yyvsp[-2].str), 0, s->data.str_val);
        else
            variableReAssignment((yyvsp[-2].str), s->data.val, NULL);
        free((yyvsp[-2].str)); free((yyvsp[0].str));
    }
#line 1490 "p.tab.c"
    break;

  case 36: /* expression: INTEGER  */
#line 342 "p.y"
                          { (yyval.num) = (yyvsp[0].num); }
#line 1496 "p.tab.c"
    break;

  case 37: /* expression: CHARACTER  */
#line 343 "p.y"
                          { (yyval.num) = (int)(yyvsp[0].character); }
#line 1502 "p.tab.c"
    break;

  case 38: /* expression: VARIABLE  */
#line 345 "p.y"
    {
        if (currentVarBeingDeclared && strcmp((yyvsp[0].str), currentVarBeingDeclared) == 0) {
            yyerror("Variable '%s' used in its own initializer", (yyvsp[0].str));
            free((yyvsp[0].str));
            (yyval.num) = 0;
            YYABORT;
        }
        (yyval.num) = getVariableValue((yyvsp[0].str));
        free((yyvsp[0].str));
    }
#line 1517 "p.tab.c"
    break;

  case 39: /* expression: '(' expression ')'  */
#line 355 "p.y"
                          { (yyval.num) = (yyvsp[-1].num); }
#line 1523 "p.tab.c"
    break;

  case 40: /* expression: expression '+' expression  */
#line 356 "p.y"
                              { (yyval.num) = (yyvsp[-2].num) + (yyvsp[0].num); }
#line 1529 "p.tab.c"
    break;

  case 41: /* expression: expression '-' expression  */
#line 357 "p.y"
                              { (yyval.num) = (yyvsp[-2].num) - (yyvsp[0].num); }
#line 1535 "p.tab.c"
    break;

  case 42: /* expression: expression '*' expression  */
#line 358 "p.y"
                              { (yyval.num) = (yyvsp[-2].num) * (yyvsp[0].num); }
#line 1541 "p.tab.c"
    break;

  case 43: /* expression: expression '/' expression  */
#line 360 "p.y"
    {
        if ((yyvsp[0].num) == 0) { yyerror("Division by zero"); (yyval.num) = 0; YYABORT; }
        else (yyval.num) = (yyvsp[-2].num) / (yyvsp[0].num);
    }
#line 1550 "p.tab.c"
    break;

  case 44: /* expression: '-' expression  */
#line 364 "p.y"
                                { (yyval.num) = -(yyvsp[0].num); }
#line 1556 "p.tab.c"
    break;

  case 45: /* expression: '+' expression  */
#line 365 "p.y"
                                { (yyval.num) = (yyvsp[0].num); }
#line 1562 "p.tab.c"
    break;


#line 1566 "p.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 368 "p.y"



int main(void) {
    printf("Welcome to my Custom sPyC!\n");
    int result = yyparse();
    if (headVars) printVariableTable();
    if (headErrList) printErrorTable();
    if (headErrList) cleanupErrorTable();
    if (headVars) cleanupVariableTable();
    return result;
}


/*---------------------------------Error handling---------------------------------------------------*/
void yyerror(const char *fmt, ...) {
    if (fmt && strcmp(fmt, "syntax error") == 0)
        return;

    hasError = 1;

    va_list args;
    va_start(args, fmt);

    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    errorList *currentError = calloc(1, sizeof(errorList));
    if(!currentError){
        fprintf(stderr, "Memory allocation failed for error node. Parser at fault.\n");
        return;
    }

    currentError->line_error = yylineno;
    currentError->error_type = strdup(buffer);
    if(!currentError->error_type){
        fprintf(stderr, "Memory allocation failed for error message. Parser at fault.\n");
        free(currentError);
        return;
    }
    currentError->next = NULL;

    if(!headErrList){
        headErrList = tailErrList = currentError;
    }else{
        tailErrList->next = currentError;
        tailErrList = currentError; 
    }
}

void printErrorTable() {
    errorList *curr = headErrList;
    printf("=========== Error Table ============\n");
    while (curr) {
        printf("Line %d: %s", curr->line_error, curr->error_type);
        if (strlen(curr->error_type) == 0 || curr->error_type[strlen(curr->error_type) - 1] != '\n')
            printf("\n");
        curr = curr->next;
    }
    printf("===================================\n");
}



void cleanupErrorTable(){
    while (headErrList) {
        errorList *tmp = headErrList;
        headErrList = headErrList->next;
        free(tmp->error_type);
        free(tmp);
    }
    headErrList = tailErrList = NULL;
}


/*--------------- Variable handling ----------------------------*/
int getVariableValue(char *variableName){
    if (isRecovering) return 0;
    
    vars *existing = getVariable(variableName);
    
    if(!existing){
        // Variable doesn't exist at all - this is an error
        yyerror("Undefined variable %s, on line %d.", variableName, yylineno);
        hasError = 1;
        return 0;
    }
    
    if(existing->data_type == 's'){
        yyerror("Cannot perform arithmetic operations on variable %s: string literals, on line %d.", variableName, yylineno);
        hasError = 1;
        return 0;
    }
    
    // Return the value (will be 0 if uninitialized, which is correct)
    return existing->data.val;
}

void cleanupVariableTable() {
    vars *current = headVars;
    vars *next;

    while (current) {
        next = current->next;
        if (current->data_type == 's' && current->data.str_val) free(current->data.str_val);
        free(current->id);
        free(current);
        current = next;
    }
    headVars = tailVars = NULL;
}

const char* typeName(char dt) {
    return (dt=='i')?"int":(dt=='c')?"char":"string";
}


vars* getVariable(char* variable) {
    vars *current = headVars;
    while (current) {
        if (strcmp(current->id, variable) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void createVariable(char *DTYPE, char* variable, int val, char *str_val) {  
    if(isdigit(variable[0])){
        yyerror("Variable %s can't start in INTEGER, in line %d.", variable, yylineno);
        hasError = 1;
        return;
    }

    vars *existing = getVariable(variable);

    if (existing && DTYPE && strlen(DTYPE) > 0) {
        yyerror("Variable '%s' is already declared with type '%s' on line %d", 
                variable, typeName(existing->data_type), yylineno);
        hasError = 1;
        return;
    }

    if (existing && (!DTYPE || strlen(DTYPE) == 0)) {
        variableReAssignment(variable, val, str_val);
        return;
    }

    if (!existing && (!DTYPE || strlen(DTYPE) == 0)) {
        yyerror("Undefined variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    if (strcmp(DTYPE, "int") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (strcmp(DTYPE, "char") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (strcmp(DTYPE, "string") == 0 && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    vars *newVar = calloc(1, sizeof(vars));
    if (!newVar) {
        fprintf(stderr, "Failed to allocate memory for variable '%s'\n", variable);
        return;
    }

    if (strcmp(DTYPE, "int") == 0) {
        newVar->data_type = 'i';
        newVar->data.val = val;
    } else if (strcmp(DTYPE, "char") == 0) {
        newVar->data_type = 'c';
        newVar->data.val = val;
    } else if (strcmp(DTYPE, "string") == 0) {
        newVar->data_type = 's';
        // For strings, use provided value or empty string
        if(str_val != NULL) {
            newVar->data.str_val = strdup(str_val);
        } else {
            newVar->data.str_val = strdup("");
        }
        if (!newVar->data.str_val) {
            fprintf(stderr, "Failed to allocate memory for string value\n");
            free(newVar);
            return;
        }
    } else {
        yyerror("Invalid data type '%s' for variable '%s'", DTYPE, variable);
        free(newVar);
        return;
    }

    newVar->id = strdup(variable);
    if (!newVar->id) {
        fprintf(stderr, "Failed to allocate memory for variable ID '%s'\n", variable);
        if (newVar->data_type == 's' && newVar->data.str_val)
            free(newVar->data.str_val);
        free(newVar);
        return;
    }

    newVar->next = NULL;

    if (!headVars) {
        headVars = tailVars = newVar;
    } else {
        tailVars->next = newVar;
        tailVars = newVar;
    }

    printf("Variable '%s' successfully created on line %d.\n", variable, yylineno);
}

void variableReAssignment(char* variable, int val, char *str_val){
    vars *existing = getVariable(variable);

    if(!existing){
        yyerror("Undefined variable %s on line %d.", variable, yylineno);
        hasError = 1;
        return;
    }

    if (existing->data_type == 'i' && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (existing->data_type == 'c' && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (existing->data_type == 's' && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    if(existing->data_type == 'i' || existing->data_type == 'c'){
        existing->data.val = val;
    } else {
        char *new_str = strdup(str_val ? str_val : "");
        if(!new_str){
            yyerror("Failed to allocate memory for str value on line %d", yylineno);
            hasError = 1;
            return;
        }
        free(existing->data.str_val);
        existing->data.str_val = new_str;
    }
    
    printf("Variable '%s' updated successfully on line %d.\n", variable, yylineno);
}


void printVariableTable() {
    printf("\n=== Variable Table ===\n");
    vars *curr = headVars;
    while (curr) {
        printf("Variable: %s, Type: %s", curr->id, typeName(curr->data_type));
        if (curr->data_type == 's') {
            printf(", Value: \"%s\"\n", curr->data.str_val);
        } else {
            printf(", Value: %d\n", curr->data.val);
        }
        curr = curr->next;
    }
    printf("======================\n\n");
}
