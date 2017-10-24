/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


#define WIDTH   100 //640
#define HEIGHT  100 //480


/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];


/* Replace this function with something useful. */

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

  //printf("x = %02d, y = %02d\n", x, y);

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;

      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}


void
show_image( void )
{
  int  i, j;


  for ( i = 0; i < HEIGHT; i++ )
  {
      printf("%02d", i);    

      for ( j = 0; j < WIDTH; j++ )
      putchar( image[i][j] == 0 ? ' '
                                : image[i][j] < 128 ? '+'
                                                    : '*' );
    putchar( '\n' );
  }
}


int
main( int     argc,
      char**  argv )
{
  FT_Library    library;
  FT_Face       face;

  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;

  char*         filename;
  char*         text;

  double        angle;
  int           target_height;
  int           n, num_chars;

  FT_BBox  bbox;
  FT_Glyph  glyph; /* a handle to the glyph image */

  if ( argc != 2 )
  {
    fprintf ( stderr, "usage: %s font\n", argv[0] );
    exit( 1 );
  }

  filename      = argv[1];                           /* first argument     */
  //text          = argv[2];                           /* second argument    */
  //num_chars     = strlen( text );
  //angle         = ( 25.0 / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
  angle         = ( 0.0 / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
  target_height = HEIGHT;

  int chinese_str[] = {0x5f20, 0x88d5, 0x7ef4, 0x7a};   /* 张裕维z 的Unicode码 */
//  char *str = "裕维yw";   /* 这样写不可以，会变得复杂。因为中文用两个字节表示，英文用一个字节表示。所以后来就引入了宽字符。 */
  wchar_t *str =  L"裕维yw"; /* 不管是英文还是中文都用4个字节来表示，就统一起来了。 */
  unsigned int *p_i = (unsigned int *)str;
  int i;

  printf("Unicode: ");
  for(i = 0; i < wcslen(str); i++)
  {
      printf("0x%x ", p_i[i]);
  }
  printf("\n");
  /* 裕维yw：0x88d5 0x7ef4 0x79 0x77 */
#if 0
  return 0;
#endif

  error = FT_Init_FreeType( &library );              /* initialize library */
  /* error handling omitted */

  error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
  /* error handling omitted */

#if 0
  /* use 50pt at 100dpi */
  error = FT_Set_Char_Size( face, 50 * 64, 0,
                            100, 0 );                /* set character size */
  /* error handling omitted */
#else
    FT_Set_Pixel_Sizes(face, 24, 0);
#endif

  slot = face->glyph;

  /* set up matrix */
  matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

  /* the pen position in 26.6 cartesian space coordinates; */
  /* start at (300,200) relative to the upper left corner  */
//  pen.x = 300 * 64;
//  pen.y = ( target_height - 200 ) * 64;
    pen.x = 0 * 64;
    pen.y = ( target_height - 60 ) * 64;

  //for ( n = 0; n < num_chars; n++ )
  //for ( n = 0; n < sizeof(chinese_str)/sizeof(chinese_str[0]); n++ )
  for ( n = 0; n < wcslen(str); n++ )
  {
    /* set transformation */
    FT_Set_Transform( face, &matrix, &pen );

    /* load glyph image into the slot (erase previous one) */
    //error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
    //error = FT_Load_Char( face, chinese_str[n], FT_LOAD_RENDER );
    error = FT_Load_Char( face, str[n], FT_LOAD_RENDER );
    if ( error )
      continue;                 /* ignore errors */

    error = FT_Get_Glyph( face->glyph, &glyph );
    if ( error )
    {
        printf("calling FT_Get_Glyph() fail.\n");  
        return -1;
    }

    FT_Glyph_Get_CBox( glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox );

    printf("current Unicode: 0x%x\n", str[n]);
    printf("origin.x/64 = %d, origin.y/64 = %d\n", pen.x/64, pen.y/64);
    printf("xMin = %d, xMax = %d, yMin = %d, yMax = %d\n", bbox.xMin, bbox.xMax, bbox.yMin, bbox.yMax);
    printf("slot->advance.x/64 = %d, slot->advance.y/64 = %d\n", slot->advance.x/64, slot->advance.y/64);
    //printf("glyph->advance.x/64 = %d, glyph->advance.y/64 = %d\n", glyph->advance.x/64, glyph->advance.y/64); /* what is it? */

    /* now, draw to our target surface (convert position) */
    draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }

  show_image();

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  return 0;
}

/* EOF */
