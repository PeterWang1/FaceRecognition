#include "GST_bitmap.hpp"
#include "../util/GST_util.hpp"
#include <random>
#include <time.h>
#include <thread>

#ifdef WIN32
	#include <windows.h>
	#include <assert.h>
	#include <math.h>
#endif


/* Intrinsic declarations */
#if defined(__SSE2__) || defined(__MMX__)
#if defined(__SSE2__)
#include <emmintrin.h>
#elif defined(__MMX__)
#include <mmintrin.h>
#endif
#if defined(__GNUC__)
#include <mm_malloc.h>
#elif defined(_MSC_VER)
#include <malloc.h>
#endif
#elif defined(__ALTIVEC__)
#include <altivec.h>
#endif

/* Compiler peculiarities */
#if defined(__GNUC__)
#include <stdint.h>
#define inline __inline__
#define align(x) __attribute__ ((aligned (x)))
#elif defined(_MSC_VER)
#define inline __inline
#define align(x) __declspec(align(x))
#else
#define inline
#define align(x)
#endif

#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

/**
 * This structure represents a two-tier histogram. The first tier (known as the
 * "coarse" level) is 4 bit wide and the second tier (known as the "fine" level)
 * is 8 bit wide. Pixels inserted in the fine level also get inserted into the
 * coarse bucket designated by the 4 MSBs of the fine bucket value.
 *
 * The structure is aligned on 16 bytes, which is a prerequisite for SIMD
 * instructions. Each bucket is 16 bit wide, which means that extra care must be
 * taken to prevent overflow.
 */
typedef struct align(16)
{
    uint16_t coarse[16];
    uint16_t fine[16][16];
} Histogram;

/**
 * HOP is short for Histogram OPeration. This macro makes an operation \a op on
 * histogram \a h for pixel value \a x. It takes care of handling both levels.
 */
#define HOP(h,x,op) \
    h.coarse[x>>4] op; \
    *((uint16_t*) h.fine + x) op;

#define COP(c,j,x,op) \
    h_coarse[ 16*(n*c+j) + (x>>4) ] op; \
    h_fine[ 16 * (n*(16*c+(x>>4)) + j) + (x & 0xF) ] op;

/**
 * Adds histograms \a x and \a y and stores the result in \a y. Makes use of
 * SSE2, MMX or Altivec, if available.
 */
#if defined(__SSE2__)
static inline void histogram_add( const uint16_t x[16], uint16_t y[16] )
{
    *(__m128i*) &y[0] = _mm_add_epi16( *(__m128i*) &y[0], *(__m128i*) &x[0] );
    *(__m128i*) &y[8] = _mm_add_epi16( *(__m128i*) &y[8], *(__m128i*) &x[8] );
}
#elif defined(__MMX__)
static inline void histogram_add( const uint16_t x[16], uint16_t y[16] )
{
    *(__m64*) &y[0]  = _mm_add_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
    *(__m64*) &y[4]  = _mm_add_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
    *(__m64*) &y[8]  = _mm_add_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
    *(__m64*) &y[12] = _mm_add_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_add( const uint16_t x[16], uint16_t y[16] )
{
    *(vector unsigned short*) &y[0] = vec_add( *(vector unsigned short*) &y[0], *(vector unsigned short*) &x[0] );
    *(vector unsigned short*) &y[8] = vec_add( *(vector unsigned short*) &y[8], *(vector unsigned short*) &x[8] );
}
#else
static inline void histogram_add( const uint16_t x[16], uint16_t y[16] )
{
    int i;
    for ( i = 0; i < 16; ++i ) {
        y[i] += x[i];
    }
}
#endif

/**
 * Subtracts histogram \a x from \a y and stores the result in \a y. Makes use
 * of SSE2, MMX or Altivec, if available.
 */
#if defined(__SSE2__)
static inline void histogram_sub( const uint16_t x[16], uint16_t y[16] )
{
    *(__m128i*) &y[0] = _mm_sub_epi16( *(__m128i*) &y[0], *(__m128i*) &x[0] );
    *(__m128i*) &y[8] = _mm_sub_epi16( *(__m128i*) &y[8], *(__m128i*) &x[8] );
}
#elif defined(__MMX__)
static inline void histogram_sub( const uint16_t x[16], uint16_t y[16] )
{
    *(__m64*) &y[0]  = _mm_sub_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
    *(__m64*) &y[4]  = _mm_sub_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
    *(__m64*) &y[8]  = _mm_sub_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
    *(__m64*) &y[12] = _mm_sub_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_sub( const uint16_t x[16], uint16_t y[16] )
{
    *(vector unsigned short*) &y[0] = vec_sub( *(vector unsigned short*) &y[0], *(vector unsigned short*) &x[0] );
    *(vector unsigned short*) &y[8] = vec_sub( *(vector unsigned short*) &y[8], *(vector unsigned short*) &x[8] );
}
#else
static inline void histogram_sub( const uint16_t x[16], uint16_t y[16] )
{
    int i;
    for ( i = 0; i < 16; ++i ) {
        y[i] -= x[i];
    }
}
#endif

static inline void histogram_muladd( const uint16_t a, const uint16_t x[16],
        uint16_t y[16] )
{
    int i;
    for ( i = 0; i < 16; ++i ) {
        y[i] += a * x[i];
    }
}

static void ctmf_helper(
        const unsigned char* const src, unsigned char* const dst,
        const int width, const int height,
        const int src_step, const int dst_step,
        const int r, const int cn,
        const int pad_left, const int pad_right
        )
{
    const int m = height, n = width;
    int i, j, k, c;
    const unsigned char *p, *q;

    Histogram H[4];
    uint16_t *h_coarse, *h_fine, luc[4][16];

    assert( src );
    assert( dst );
    assert( r >= 0 );
    assert( width >= 2*r+1 );
    assert( height >= 2*r+1 );
    assert( src_step != 0 );
    assert( dst_step != 0 );

    /* SSE2 and MMX need aligned memory, provided by _mm_malloc(). */
#if defined(__SSE2__) || defined(__MMX__)
    h_coarse = (uint16_t*) _mm_malloc(  1 * 16 * n * cn * sizeof(uint16_t), 16 );
    h_fine   = (uint16_t*) _mm_malloc( 16 * 16 * n * cn * sizeof(uint16_t), 16 );
    memset( h_coarse, 0,  1 * 16 * n * cn * sizeof(uint16_t) );
    memset( h_fine,   0, 16 * 16 * n * cn * sizeof(uint16_t) );
#else
    h_coarse = (uint16_t*) calloc(  1 * 16 * n * cn, sizeof(uint16_t) );
    h_fine   = (uint16_t*) calloc( 16 * 16 * n * cn, sizeof(uint16_t) );
#endif

    /* First row initialization */
    for ( j = 0; j < n; ++j ) {
        for ( c = 0; c < cn; ++c ) {
            COP( c, j, src[cn*j+c], += r+1 );
        }
    }
    for ( i = 0; i < r; ++i ) {
        for ( j = 0; j < n; ++j ) {
            for ( c = 0; c < cn; ++c ) {
                COP( c, j, src[src_step*i+cn*j+c], ++ );
            }
        }
    }

    for ( i = 0; i < m; ++i ) {

        /* Update column histograms for entire row. */
        p = src + src_step * MAX( 0, i-r-1 );
        q = p + cn * n;
        for ( j = 0; p != q; ++j ) {
            for ( c = 0; c < cn; ++c, ++p ) {
                COP( c, j, *p, -- );
            }
        }

        p = src + src_step * MIN( m-1, i+r );
        q = p + cn * n;
        for ( j = 0; p != q; ++j ) {
            for ( c = 0; c < cn; ++c, ++p ) {
                COP( c, j, *p, ++ );
            }
        }

        /* First column initialization */
        memset( H, 0, cn*sizeof(H[0]) );
        memset( luc, 0, cn*sizeof(luc[0]) );
        if ( pad_left ) {
            for ( c = 0; c < cn; ++c ) {
                histogram_muladd( r, &h_coarse[16*n*c], H[c].coarse );
            }
        }
        for ( j = 0; j < (pad_left ? r : 2*r); ++j ) {
            for ( c = 0; c < cn; ++c ) {
                histogram_add( &h_coarse[16*(n*c+j)], H[c].coarse );
            }
        }
        for ( c = 0; c < cn; ++c ) {
            for ( k = 0; k < 16; ++k ) {
                histogram_muladd( 2*r+1, &h_fine[16*n*(16*c+k)], &H[c].fine[k][0] );
            }
        }

        for ( j = pad_left ? 0 : r; j < (pad_right ? n : n-r); ++j ) {
            for ( c = 0; c < cn; ++c ) {
                const uint16_t t = 2*r*r + 2*r;
                uint16_t sum = 0, *segment;
                int b;

                histogram_add( &h_coarse[16*(n*c + MIN(j+r,n-1))], H[c].coarse );

                /* Find median at coarse level */
                for ( k = 0; k < 16 ; ++k ) {
                    sum += H[c].coarse[k];
                    if ( sum > t ) {
                        sum -= H[c].coarse[k];
                        break;
                    }
                }
                assert( k < 16 );

                /* Update corresponding histogram segment */
                if ( luc[c][k] <= j-r ) {
                    memset( &H[c].fine[k], 0, 16 * sizeof(uint16_t) );
                    for ( luc[c][k] = j-r; luc[c][k] < MIN(j+r+1,n); ++luc[c][k] ) {
                        histogram_add( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );
                    }
                    if ( luc[c][k] < j+r+1 ) {
                        histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                        luc[c][k] = j+r+1;
                    }
                }
                else {
                    for ( ; luc[c][k] < j+r+1; ++luc[c][k] ) {
                        histogram_sub( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                        histogram_add( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                    }
                }

                histogram_sub( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                /* Find median in segment */
                segment = H[c].fine[k];
                for ( b = 0; b < 16 ; ++b ) {
                    sum += segment[b];
                    if ( sum > t ) {
                        dst[dst_step*i+cn*j+c] = 16*k + b;
                        break;
                    }
                }
                assert( b < 16 );
            }
        }
    }

#if defined(__SSE2__) || defined(__MMX__)
    _mm_empty();
    _mm_free(h_coarse);
    _mm_free(h_fine);
#else
    free(h_coarse);
    free(h_fine);
#endif
}

void ctmf(
        const BIT_8* const src, BIT_8* const dst,
        const int width, const int height,const int src_step, const int dst_step,
        const int radius, const int channels, long unsigned int memsize=512*1024
        ){    
	if( memsize==0 )		
		memsize=512*1024;
    int stripes = (int) ceil( (double) (width - 2*radius) / (memsize / sizeof(Histogram) - 2*radius) );
    int stripe_size = (int) ceil( (double) ( width + stripes*2*radius - 2*radius ) / stripes );

    int i;

    for ( i = 0; i < width; i += stripe_size - 2*radius ) {
        int stripe = stripe_size;
        /* Make sure that the filter kernel fits into one stripe. */
        if ( i + stripe_size - 2*radius >= width || width - (i + stripe_size - 2*radius) < 2*radius+1 ) {
            stripe = width - i;
        }

        ctmf_helper( src + channels*i, dst + channels*i, stripe, height, src_step, dst_step, radius, channels,
                i == 0, stripe == width - i );

        if ( stripe == width - i ) {
            break;
        }
    }
}
