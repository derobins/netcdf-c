#ifndef NC_NCGEN_H
#define NC_NCGEN_H
/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/ncgen.h,v 1.18 2010/06/01 15:34:53 ed Exp $
*********************************************************************/

#include "config.h"

#ifndef nulldup
 #define nulldup(x) ((x)?strdup(x):(x))
#endif

#ifdef USE_NETCDF4
#define CLASSICONLY 0
#else
#define CLASSICONLY 1
#endif

#define MAX_NC_ATTSIZE    20000	/* max size of attribute (for ncgen) */
#define MAXTRST		  5000	/* max size of string value (for ncgen) */

/* Define the possible classes of objects*/
/* extend the NC_XXX values*/
#define NC_GRP      100
#define NC_DIM      101
#define NC_VAR      102
#define NC_ATT      103
#define NC_TYPE     104
#define NC_ECONST   105
#define NC_FIELD    106
#define NC_ARRAY    107
#define NC_PRIM     108 /*Including NC_STRING */
#define NC_STRUCT  NC_COMPOUND /* alias */
#define NC_LIST    NC_COMPOUND /* alias */

/* Extend nc types with generic fill value*/
#define NC_FILLVALUE 31
/* Extend nc types with NIL value*/
#define NC_NIL       32

#define NEGNC_INFINITEF (-NC_INFINITEF)
#define NEGNC_INFINITE (-NC_INFINITEF)

/* nc_class is one of:
        NC_GRP NC_DIM NC_VAR NC_ATT NC_TYPE
*/
typedef nc_type nc_class;

/* nc_subclass is one of:
	NC_PRIM NC_OPAQUE NC_ENUM
	NC_FIELD NC_VLEN NC_COMPOUND
	NC_ECONST NC_ARRAY NC_FILLVALUE
*/
typedef nc_type nc_subclass;

/*
Define data structure
xto hold special attribute values
for a given variable.
Global values are kept as
various C global variables
*/

/* Define a bit set for indicating which*/
/* specials were explicitly specified*/
/* See also: ncgen.y.tag2name */
#define _STORAGE_FLAG       0x001
#define _CHUNKSIZES_FLAG    0x002
#define _FLETCHER32_FLAG    0x004
#define _DEFLATE_FLAG       0x008
#define _SHUFFLE_FLAG       0x010
#define _ENDIAN_FLAG        0x020
#define _NOFILL_FLAG        0x040
#define _FILLVALUE_FLAG     0x080
#define _NCPROPS_FLAG       0x100
#define _ISNETCDF4_FLAG     0x200
#define _SUPERBLOCK_FLAG    0x400
#define _FORMAT_FLAG        0x800
#define _FILTER_FLAG        0x1000
#define _CODECS_FLAG        0x2000
#define _QUANTIZEBG_FLAG    0x4000
#define _QUANTIZEGBR_FLAG   0x8000
#define _QUANTIZEBR_FLAG    0x10000

extern struct Specialtoken {
    char* name;
    int   token;
    int   tag;
} specials[];

/* Define an enumeration of supported languages */
typedef enum Language {
    L_UNDEFINED=0,
    L_BINARY=1,
    L_C=2,
    L_F77=3,
    L_JAVA=4
} Language;

struct Kvalues {
char* name;
int k_flag;
int deprecated;
};

extern struct Kvalues legalkinds[];

#define ZIP_ID  0xFFFFFFFF
#define SZIP_ID  0xFFFFFFFE
#define BZIP2_ID 307U
#define ZFP_ID 32013U
#define FPZIP_ID 32014U

/* Note: some non-var specials (i.e. _Format) are not included in this struct*/
typedef struct Specialdata {
    int flags;
    Datalist*      _Fillvalue; /* This is a per-type ; points to the _FillValue attribute node */
    int           _Storage;      /* NC_CHUNKED | NC_CONTIGUOUS | NC_COMPACT*/
    size_t*       _ChunkSizes;     /* NULL => defaults*/
        int nchunks;     /*  |_Chunksize| ; 0 => not specified*/
    int           _Fletcher32;     /* 1=>fletcher32*/
    int           _DeflateLevel; /* 0-9 => level*/
    int           _Shuffle;      /* 0 => false, 1 => true*/
    int           _Endianness;   /* 1 =>little, 2 => big*/
    int           _Fill ;        /* 0 => false, 1 => true WATCHOUT: this is inverse of NOFILL*/
    int           _Quantizer;    /* algorithm */
    int           _NSD;          /* No. of significant digits */
    NC_H5_Filterspec** _Filters;
    size_t 	   nfilters; /* |filters| */
    char*          _Codecs; /* in JSON form */
} Specialdata;

typedef struct GlobalSpecialdata {
    int           _Format ;      /* kflag */
    const char*   _NCProperties ;
    int           _IsNetcdf4 ;   /* 0 => false, 1 => true */
    int           _Superblock  ; /* HDF5 file superblock version */
} GlobalSpecialData;

/*
During the generation of binary data,
we will generate a number of references
to strings and opaques that should
be reclaimed to keep the memory
checkers happy.
*/
typedef struct BinBuffer {
    Bytebuffer* buf; /* top level data */
    List* reclaim; /* objects that need to be free'd */
} BinBuffer;

/* Track a set of dimensions*/
/* (Note: the netcdf type system is deficient here)*/
typedef struct Dimset {
    int		     ndims;
    struct Symbol*   dimsyms[NC_MAX_VAR_DIMS]; /* Symbol for dimension*/
} Dimset;

typedef struct Diminfo {
    int   isconstant; /* separate constant from named dimension*/
    int   isunlimited;
    size_t  declsize; /* 0 => unlimited/unspecified*/
} Diminfo;

typedef struct Attrinfo {
    struct Symbol*   var; /* NULL => global*/
} Attrinfo;

typedef struct Typeinfo {
        struct Symbol*  basetype;
	int             hasvlen;  /* 1 => this type contains a vlen*/
	nc_type         typecode;
        unsigned long   offset;   /* fields in struct*/
        size_t          alignment;/* fields in struct*/
        NCConstant*     econst;   /* for enum values*/
        Dimset          dimset;     /* for NC_VAR/NC_FIELD/NC_ATT*/
        size_t   size;     /* for opaque, compound, etc.*/
	size_t   cmpdalign; /* alignment needed for total size instances */
        size_t   nelems;   /* size in terms of # of datalist constants
			      it takes to represent it */
	Datalist*       _Fillvalue; /* per-type cached fillvalue
                                       (overridden by var fillvalue) */
} Typeinfo;

typedef struct Varinfo {
    int		nattributes; /* |attributes|*/
    List*       attributes;  /* List<Symbol*>*/
    Specialdata special;
} Varinfo;

typedef struct Groupinfo {
    int is_root;
} Groupinfo;

typedef struct Fileinfo {
    char* filename;
} Fileinfo;

/* store info when the symbol
   is really a reference to another
   symbol
*/
typedef struct Reference {
        int             is_ref;  /* separate name defs  from refs*/
	char*		unescaped; /* original, unescaped name */
        struct Symbol*  ref;  /* ptr to the symbol if is_ref is true*/
} Reference;

typedef struct Symbol {  /* symbol table entry*/
        nc_class        objectclass;  /* NC_DIM|NC_VLEN|NC_OPAQUE...*/
        nc_class        subclass;  /* NC_STRUCT|...*/
        char*           name;
	char*           fqn; /* cached fully qualified C or FORTRAN name*/
        struct Symbol*  container;  /* The group containing this symbol.*/
				    /* for fields or enumids, it is*/
				    /* the parent type.*/
	struct Symbol*   location;   /* current group when symbol was created*/
	List*            subnodes;  /* sublist for enum or struct or group*/
	int              is_prefixed; /* prefix was specified (vs computed).*/
        List*            prefix;  /* List<Symbol*>*/
        struct Datalist* data; /* shared by variables and attributes*/
	/* Note that we cannot union these because some kinds of symbols*/
        /* use more than one part*/
        Typeinfo  typ; /* type info for e.g. var, att, etc.*/
        Varinfo   var;
        Attrinfo  att;
        Diminfo   dim;
        Groupinfo grp;
	Fileinfo  file;
 	Reference ref; /* symbol is really a reference to another symbol*/
	/* Misc pieces of info*/
	int             lineno;  /* at point of creation*/
	int		touched; /* for sorting*/
	/* for use by -lb */
        int             nc_id;  /* from netcdf API: varid, or dimid, or etc.*/
} Symbol;


#endif /*!NC_NCGEN_H*/
