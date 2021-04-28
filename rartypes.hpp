/***** File: rartypes.hpp *****/


#ifndef _RAR_TYPES_
#define _RAR_TYPES_

typedef unsigned char    byte;   //8 bits
typedef unsigned short   ushort; //preferably 16 bits, but can be more
typedef unsigned int     uint;   //32 bits or more

typedef unsigned int     uint32; //32 bits exactly
#define PRESENT_INT32

#if defined(__GNUC__)
typedef wchar_t wchar;
#else
typedef ushort wchar;
#endif

#define SHORT16(x) (sizeof(ushort)==2 ? (ushort)(x):((x)&0xffff))

#endif
