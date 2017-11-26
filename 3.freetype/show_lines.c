#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


#define FONTDATAMAX 4096
#define FB_PATHNAME "/dev/fb0"
#define LCD_COLOR_BLACK 0x0
#define LCD_COLOR_WHITE 0xff
#define MAX_GLYPHS 100  /* 假设LCD一行最多能显示100个字符 */
typedef struct TGlyph_
{
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */
} TGlyph, *PGlyph;

static int Get_Glyphs_Frm_Wstr(const FT_Face *face, const wchar_t *wstr, TGlyph *glyphs, int num_glyphs, int *num_get_glyphs);
static void compute_string_bbox(const TGlyph *glyphs, int num_glyphs, FT_BBox *abbox);
static void Draw_Glyphs(const TGlyph *glyphs, int num_glyphs, const FT_Vector *pen);
static void lcd_put_pixel(int x, int y, unsigned int color);
static void draw_bitmap( FT_Bitmap*  bitmap,
                         FT_Int      x,
                         FT_Int      y);

unsigned char *fb_mem;
struct fb_var_screeninfo fb_var;    /* Current var */
int fb_pixel_byte = 0;

int main(int argc, char **argv)
{
    int fb_fd;
	struct fb_fix_screeninfo fb_fix;	/* Current fix */
    int fb_mem_size = 0;
    struct stat stat_buf;
    FT_Error      error;
    FT_Library    library;
    FT_Face       face;
    TGlyph        glyphs[MAX_GLYPHS];  /* glyphs table */
    FT_UInt       num_glyphs;
    int ret;
    FT_BBox  string_bbox ;
    int line_box_width;
    int line_box_height; 
    int           n;
    FT_Vector     pen;                    /* untransformed origin  */
    wchar_t *ce_mix_str1 =  L"裕维yw最棒"; /* 不管是英文还是中文都用4个字节来表示，就统一起来了。 */
    wchar_t *ce_mix_str2 =  L"https://github.com/wale-ftd"; /* 不管是英文还是中文都用4个字节来表示，就统一起来了。 */

    if ( argc != 2 )
    {
        fprintf ( stderr, "usage: %s <font_file>\n", argv[0] );
        exit( 1 );
    }

    fb_fd = open(FB_PATHNAME, O_RDWR);
    if(-1 == fb_fd)
    {
        printf("%s can't be opened.\n", FB_PATHNAME);
        return -1;
    }

    if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var))
    {
        printf("can't get fb_var_screeninfo.\n");
        return -1;
    }
    if(ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_fix))
    {
        printf("can't get fb_fix_screeninfo.\n");
        return -1;
    }
    printf("xres = %d, yres = %d, bpp = %d.\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel);

    fb_pixel_byte = fb_var.bits_per_pixel / 8;
    fb_mem_size = fb_var.xres * fb_var.yres * fb_pixel_byte;
    
    fb_mem = (unsigned char *)mmap(NULL, fb_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if((unsigned char *)(-1) == fb_mem)
    {
        printf("fb device can't be mmapped.\n");
        return -1;
    }

	/* 清屏: 设为黑色背景 */
    memset(fb_mem, LCD_COLOR_BLACK, fb_mem_size);

    /*------------------------ 显示矢量字体 -------------------------*/    
    error = FT_Init_FreeType( &library );              /* initialize library */
    /* error handling omitted */

    error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
    /* error handling omitted */
    
    FT_Set_Pixel_Sizes(face, 24, 0);

    /*------------------------ 显示ce_mix_str1 -------------------------*/    
    ret = Get_Glyphs_Frm_Wstr(&face, ce_mix_str1, glyphs, MAX_GLYPHS, &num_glyphs);
    if(ret)
    {
        printf("calling Get_Glyphs_Frm_Wstr() fail.\n");
        return -1;
    }

    /* get bbox of original glyph sequence */
    compute_string_bbox(glyphs, num_glyphs, &string_bbox);

    /* compute string dimensions in integer pixels */
    line_box_width  = string_bbox.xMax - string_bbox.xMin;
    line_box_height = string_bbox.yMax - string_bbox.yMin;

	/* 确定座标:
	 * lcd_x = (fb_var.xres - string_width) / 2
	 * lcd_y = (fb_var.yres - string_height) / 2
	 * 笛卡尔座标系:
	 * x = lcd_x = (fb_var.xres - string_width) / 2
	 * y = fb_var.yres - lcd_y = fb_var.yres - (fb_var.yres - string_height) / 2
	 */
    pen.x = (fb_var.xres - line_box_width) / 2 * 64;
    pen.y = (fb_var.yres + line_box_height) / 2 * 64;

    Draw_Glyphs(glyphs, num_glyphs, &pen);

    /*------------------------ 显示ce_mix_str2 -------------------------*/    
    ret = Get_Glyphs_Frm_Wstr(&face, ce_mix_str2, glyphs, MAX_GLYPHS, &num_glyphs);
    if(ret)
    {
        printf("calling Get_Glyphs_Frm_Wstr() fail.\n");
        return -1;
    }

    /* get bbox of original glyph sequence */
    compute_string_bbox(glyphs, num_glyphs, &string_bbox);

    /* compute string dimensions in integer pixels */
    line_box_width  = string_bbox.xMax - string_bbox.xMin;
    line_box_height = string_bbox.yMax - string_bbox.yMin;

	/* 确定座标:
	 * lcd_x = (fb_var.xres - string_width) / 2
	 * lcd_y = (fb_var.yres - string_height) / 2 + 24
	 * 笛卡尔座标系:
	 * x = lcd_x = (fb_var.xres - string_width) / 2
	 * y = fb_var.yres - lcd_y = fb_var.yres - (fb_var.yres - string_height) / 2 - 24
	 */
    pen.x = (fb_var.xres - line_box_width) / 2 * 64;
    pen.y = ((fb_var.yres + line_box_height) / 2 - 24) * 64;
    //pen.y = pen.y - 24 * 64;

    Draw_Glyphs(glyphs, num_glyphs, &pen);

    FT_Done_Face    ( face );
    FT_Done_FreeType( library );
    
    return 0;
}

static void draw_bitmap( FT_Bitmap*  bitmap,
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
            if ( i < 0 || j < 0 ||
                 i >= fb_var.xres || j >= fb_var.yres )
                continue;
            
            //image[j][i] |= bitmap->buffer[q * bitmap->width + p];
            lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}

/* color : 0x00RRGGBB */
static void lcd_put_pixel(int x, int y, unsigned int color)
{
    unsigned char *pen8   = fb_mem + (x+y*fb_var.xres) * fb_pixel_byte;
    unsigned short *pen16 = (unsigned short *)pen8;
    unsigned int *pen32   = (unsigned int *)pen8;

    switch(fb_var.bits_per_pixel)
    {
        case 8:
        {
            *pen8 = (unsigned char)color;
            
            break;
        }
        case 16:    /* RGB: 565 */
        {
            unsigned int r, g, b;
            
            r = color >> 16 & 0xff;
            g = color >> 8 & 0xff;
            b = color & 0xff;
            color = ((r>>3)<<11) | ((g>>2)<<5) | (b>>3);

            *pen16 = (unsigned short)color;
            
            break;
        }
        case 32:
        {
            *pen32 = color;
            
            break;
        }
        default:
        {
            printf("%dbpp can't be supported.", fb_var.bits_per_pixel);
            
            break;
        }
    }
}

static int Get_Glyphs_Frm_Wstr(const FT_Face *face, const wchar_t *wstr, TGlyph *glyphs, int num_glyphs_max, int *num_get_glyphs)
{
    int ret = 0;
    int n = 0;
    TGlyph *glyph = glyphs;
    int pen_x = 0;   /* start at (0,0) */
    int pen_y = 0;
    FT_GlyphSlot slot = (*face)->glyph;  /* a small shortcut */
    FT_Error error;

    if(!face || !wstr || !glyphs || (MAX_GLYPHS<num_glyphs_max) || !num_get_glyphs)
        return ret;

    for ( n = 0; n < wcslen(wstr); n++ )
    {
        /* 根据unicode码(宽字符的编码就是unicode码)，找到glyph在face中的索引 */
        glyph->index = FT_Get_Char_Index(*face, wstr[n]);

        /* store current pen position */
        glyph->pos.x = pen_x;
        glyph->pos.y = pen_y;

        /* 根据索引，找到glyph，再将它加载到插槽face->glyph中 */
        error = FT_Load_Glyph(*face, glyph->index, FT_LOAD_DEFAULT );
        if (error) 
        {
            ret = -1;
            continue;
        }
        
        /* 将face->glyph取出保存起来，因为下次调用FT_Load_Glyph()时，face->glyph会被覆盖 */
        error = FT_Get_Glyph((*face)->glyph, &glyph->image);
        if (error)
        {
            ret = -1;
            continue;
        }
        
        /* translate the glyph image now */
		/* 这使得glyph->image里含有位置信息 */
        FT_Glyph_Transform(glyph->image, 0, &glyph->pos);

        pen_x += slot->advance.x;   /* Unit: 1/64 point */
        
        /* increment number of glyphs */
        glyph++;
    }

	/* count number of glyphs loaded */ 
    *num_get_glyphs = (int)(glyph - glyphs);
    if(*num_get_glyphs > num_glyphs_max)
        ret = -1;

    return ret;
}

static void compute_string_bbox(const TGlyph *glyphs, int num_glyphs, FT_BBox *abbox)
{
    FT_BBox bbox;
    int n = 0;

    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;

    for ( n = 0; n < num_glyphs; n++ )
    {
        FT_BBox  glyph_bbox;

        FT_Glyph_Get_CBox( glyphs[n].image, FT_GLYPH_BBOX_TRUNCATE,
                         &glyph_bbox );

        if(bbox.xMin > glyph_bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;
        if(bbox.xMax < glyph_bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;
        if(bbox.yMin > glyph_bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;
        if(bbox.yMax < glyph_bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }

    if ( bbox.xMin > bbox.xMax )
    {
      bbox.xMin = 0;
      bbox.yMin = 0;
      bbox.xMax = 0;
      bbox.yMax = 0;
    }

    *abbox = bbox;
}

static void Draw_Glyphs(const TGlyph *glyphs, int num_glyphs, const FT_Vector *pen)
{
    int n;
    
    for ( n = 0; n < num_glyphs; n++ )
    {
        FT_Glyph  image;
        int error;

        image = glyphs[n].image;
        
        /* transform copy (this will also translate it to the correct position */
        FT_Glyph_Transform(image, 0, (FT_Vector *)pen );

        /* convert glyph image to bitmap (destroy the glyph copy!) */
        error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, 
                                   0,       /* no additional translation */
                                   1);     /* destroy copy in "image"   */
        if ( !error )
        {
            FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;
    
            draw_bitmap(&bit->bitmap, bit->left, fb_var.yres - bit->top);

            FT_Done_Glyph( image );
        }
    }
}


