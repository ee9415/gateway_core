/*-----------------------------------------*
           c_macros.h
        coded by H.B. Lee
             Feb/2007
        Revisoned May/2009
 *-----------------------------------------*/

#define Malloc(size) malloc(size)
#define Mfree(mp) free(mp)

typedef unsigned char BYTE;

typedef signed char INT8;
typedef unsigned char UINT8;

typedef signed short INT16;
typedef unsigned short UINT16;

typedef signed long INT32;
typedef unsigned long UINT32;

typedef signed long long INT64;
typedef unsigned long long UINT64;

//////////////////////////////
// for standard debug message
//////////////////////////////
#define D00(a0) if (Dbg_msg_level >= 0) \
			printf(a0), fflush(stdout);else
#define D10(a0) if (Dbg_msg_level >= 1) \
			printf(a0), fflush(stdout);else
#define D20(a0) if (Dbg_msg_level >= 2) \
			printf(a0), fflush(stdout);else

#define D01(a0, a1) if (Dbg_msg_level >= 0) \
			printf(a0, a1), fflush(stdout);else
#define D11(a0, a1) if (Dbg_msg_level >= 1) \
			printf(a0, a1), fflush(stdout);else
#define D21(a0, a1) if (Dbg_msg_level >= 2) \
			printf(a0, a1), fflush(stdout);else

#define D02(a0, a1, a2) if (Dbg_msg_level >= 0) \
			printf(a0, a1, a2), fflush(stdout);else
#define D12(a0, a1, a2) if (Dbg_msg_level >= 1) \
			printf(a0, a1, a2), fflush(stdout);else
#define D22(a0, a1, a2) if (Dbg_msg_level >= 2) \
			printf(a0, a1, a2), fflush(stdout);else

#define D03(a0, a1, a2, a3) if (Dbg_msg_level >= 0) \
			printf(a0, a1, a2, a3), fflush(stdout);else
#define D13(a0, a1, a2, a3) if (Dbg_msg_level >= 1) \
			printf(a0, a1, a2, a3), fflush(stdout);else
#define D23(a0, a1, a2, a3) if (Dbg_msg_level >= 2) \
			printf(a0, a1, a2, a3), fflush(stdout);else

#define D04(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 0) \
			printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define D14(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 1) \
			printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define D24(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 2) \
			printf(a0, a1, a2, a3, a4), fflush(stdout);else

#define D05(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 0) \
			printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define D15(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 1) \
			printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define D25(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 2) \
			printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else

#define D06(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 0) \
			printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define D16(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 1) \
			printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define D26(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 2) \
			printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else 

#define FFLUSH(fp)	fflush(fp)
#define MDH(a0, a1)	hexa_dump(a0, a1)
#define MDH_1(a0, a1)    hexa_dump_1(a0, a1)
#define MDA(a0, a1)	ascii_dump(a0, a1)

//////////////////////////////
// for standard debug message server
//////////////////////////////
#define SV_D00(a0) if (SV_Dbg_msg_level >= 0) \
            printf(a0), fflush(stdout);else
#define SV_D10(a0) if (SV_Dbg_msg_level >= 1) \
            printf(a0), fflush(stdout);else
#define SV_D20(a0) if (SV_Dbg_msg_level >= 2) \
            printf(a0), fflush(stdout);else

#define SV_D01(a0, a1) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1), fflush(stdout);else
#define SV_D11(a0, a1) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1), fflush(stdout);else
#define SV_D21(a0, a1) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1), fflush(stdout);else

#define SV_D02(a0, a1, a2) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1, a2), fflush(stdout);else
#define SV_D12(a0, a1, a2) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1, a2), fflush(stdout);else
#define SV_D22(a0, a1, a2) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1, a2), fflush(stdout);else

#define SV_D03(a0, a1, a2, a3) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3), fflush(stdout);else
#define SV_D13(a0, a1, a2, a3) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3), fflush(stdout);else
#define SV_D23(a0, a1, a2, a3) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3), fflush(stdout);else

#define SV_D04(a0, a1, a2, a3, a4) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define SV_D14(a0, a1, a2, a3, a4) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define SV_D24(a0, a1, a2, a3, a4) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else

#define SV_D05(a0, a1, a2, a3, a4, a5) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define SV_D15(a0, a1, a2, a3, a4, a5) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define SV_D25(a0, a1, a2, a3, a4, a5) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else

#define SV_D06(a0, a1, a2, a3, a4, a5, a6) if (SV_Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define SV_D16(a0, a1, a2, a3, a4, a5, a6) if (SV_Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define SV_D26(a0, a1, a2, a3, a4, a5, a6) if (SV_Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else 

#define SV_MDH(a0, a1)    hexa_dump(a0, a1)
#define SV_MDH_1(a0, a1)    hexa_dump_1(a0, a1)

#define SV_MDA(a0, a1)    ascii_dump(a0, a1)

//////////////////////////////
// for standard debug message Node
//////////////////////////////
#define ND_D00(a0) if (Dbg_msg_level >= 0) \
            printf(a0), fflush(stdout);else
#define ND_D10(a0) if (Dbg_msg_level >= 1) \
            printf(a0), fflush(stdout);else
#define ND_D20(a0) if (Dbg_msg_level >= 2) \
            printf(a0), fflush(stdout);else

#define ND_D01(a0, a1) if (Dbg_msg_level >= 0) \
            printf(a0, a1), fflush(stdout);else
#define ND_D11(a0, a1) if (Dbg_msg_level >= 1) \
            printf(a0, a1), fflush(stdout);else
#define ND_D21(a0, a1) if (Dbg_msg_level >= 2) \
            printf(a0, a1), fflush(stdout);else

#define ND_D02(a0, a1, a2) if (Dbg_msg_level >= 0) \
            printf(a0, a1, a2), fflush(stdout);else
#define ND_D12(a0, a1, a2) if (Dbg_msg_level >= 1) \
            printf(a0, a1, a2), fflush(stdout);else
#define ND_D22(a0, a1, a2) if (Dbg_msg_level >= 2) \
            printf(a0, a1, a2), fflush(stdout);else

#define ND_D03(a0, a1, a2, a3) if (Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3), fflush(stdout);else
#define ND_D13(a0, a1, a2, a3) if (Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3), fflush(stdout);else
#define ND_D23(a0, a1, a2, a3) if (Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3), fflush(stdout);else

#define ND_D04(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define ND_D14(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else
#define ND_D24(a0, a1, a2, a3, a4) if (Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4), fflush(stdout);else

#define ND_D05(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define ND_D15(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else
#define ND_D25(a0, a1, a2, a3, a4, a5) if (Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4, a5), fflush(stdout);else

#define ND_D06(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 0) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define ND_D16(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 1) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else
#define ND_D26(a0, a1, a2, a3, a4, a5, a6) if (Dbg_msg_level >= 2) \
            printf(a0, a1, a2, a3, a4, a5, a6), fflush(stdout);else 

#define ND_MDH(a0, a1)    hexa_dump(a0, a1)
#define ND_MDH_1(a0, a1)  hexa_dump_1(a0, a1)

#define ND_MDA(a0, a1)    ascii_dump(a0, a1)

//////////////////////////
// for static log message
//////////////////////////
#if 1
#define L00(a0)
#define L10(a0)
#define L20(a0)

#define L01(a0, a1)
#define L11(a0, a1)
#define L21(a0, a1)

#define L02(a0, a1, a2)
#define L12(a0, a1, a2)
#define L22(a0, a1, a2)

#define L03(a0, a1, a2, a3)
#define L13(a0, a1, a2, a3)
#define L23(a0, a1, a2, a3)

#define L04(a0, a1, a2, a3, a4)
#define L14(a0, a1, a2, a3, a4)
#define L24(a0, a1, a2, a3, a4)

#define L05(a0, a1, a2, a3, a4, a5)
#define L15(a0, a1, a2, a3, a4, a5)
#define L25(a0, a1, a2, a3, a4, a5)

#define L06(a0, a1, a2, a3, a4, a5, a6)
#define L16(a0, a1, a2, a3, a4, a5, a6)
#define L26(a0, a1, a2, a3, a4, a5, a6)
#else
#define L00(a0) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0), fflush(Log_dev);else
#define L10(a0) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0), fflush(Log_dev);else
#define L20(a0) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0), fflush(Log_dev);else

#define L01(a0, a1) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1), fflush(Log_dev);else
#define L11(a0, a1) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1), fflush(Log_dev);else
#define L21(a0, a1) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1), fflush(Log_dev);else

#define L02(a0, a1, a2) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2), fflush(Log_dev);else
#define L12(a0, a1, a2) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2), fflush(Log_dev);else
#define L22(a0, a1, a2) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2), fflush(Log_dev);else

#define L03(a0, a1, a2, a3) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3), fflush(Log_dev);else
#define L13(a0, a1, a2, a3) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3), fflush(Log_dev);else
#define L23(a0, a1, a2, a3) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3), fflush(Log_dev);else

#define L04(a0, a1, a2, a3, a4) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4), fflush(Log_dev);else
#define L14(a0, a1, a2, a3, a4) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4), fflush(Log_dev);else
#define L24(a0, a1, a2, a3, a4) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4), fflush(Log_dev);else

#define L05(a0, a1, a2, a3, a4, a5) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5), fflush(Log_dev);else
#define L15(a0, a1, a2, a3, a4, a5) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5), fflush(Log_dev);else
#define L25(a0, a1, a2, a3, a4, a5) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5), fflush(Log_dev);else

#define L06(a0, a1, a2, a3, a4, a5, a6) if (Log_msg_level >= 0 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5, a6), fflush(Log_dev);else
#define L16(a0, a1, a2, a3, a4, a5, a6) if (Log_msg_level >= 1 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5, a6), fflush(Log_dev);else
#define L26(a0, a1, a2, a3, a4, a5, a6) if (Log_msg_level >= 2 && Log_dev) \
			fprintf(Log_dev, a0, a1, a2, a3, a4, a5, a6), fflush(Log_dev);else
#endif
//////////////////////////
// for statistics message
//////////////////////////
#if 0
#define SP0(a0)    
#define SP1(a0, a1)    
#define SP2(a0, a1, a2)
#define SP3(a0, a1, a2, a3)
#define SP4(a0, a1, a2, a3, a4)
#define SP5(a0, a1, a2, a3, a4, a5)
#define SP6(a0, a1, a2, a3, a4, a5, a6)
#else
#define SP0(a0) fprintf(stderr, a0), fflush(stderr)
#define SP1(a0, a1) fprintf(stderr, a0, a1), fflush(stderr)
#define SP2(a0, a1, a2) fprintf(stderr, a0, a1, a2), fflush(stderr)
#define SP3(a0, a1, a2, a3) fprintf(stderr, a0, a1, a2, a3), fflush(stderr)
#define SP4(a0, a1, a2, a3, a4) fprintf(stderr, a0, a1, a2, a3, a4), fflush(stderr)
#define SP5(a0, a1, a2, a3, a4, a5) fprintf(stderr, a0, a1, a2, a3, a4, a5), fflush(stderr)
#define SP6(a0, a1, a2, a3, a4, a5, a6) fprintf(stderr, a0, a1, a2, a3, a4, a5, a6), fflush(stderr)
#endif
