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


#define FONTDATAMAX 4096
#define FB_PATHNAME "/dev/fb0"
#define HZK_PATHNAME "HZK16"
#define LCD_COLOR_BLACK 0x0
#define LCD_COLOR_WHITE 0xff
#define ASCIIDATA_X_MAX 8
#define ASCIIDATA_Y_MAX 16
#define HZDATA_X_MAX 16
#define HZDATA_Y_MAX 16
#define HZ_AREA 0xa1
#define HZ_FIELD 0xa1
#define HZ_NUM_PER_AREA 94
#define BITS_PER_BYTE 8

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
    FT_Library    library;
    FT_Face       face;
    FT_Error      error;
    FT_GlyphSlot  slot;
    FT_Vector     pen;                    /* untransformed origin  */
    int           n;
    wchar_t *ce_mix_str1 =  L"裕维yw最棒"; /* 不管是英文还是中文都用4个字节来表示，就统一起来了。 */
    wchar_t *ce_mix_str2 =  L"https://github.com/wale-ftd"; /* 不管是英文还是中文都用4个字节来表示，就统一起来了。 */
    int line_box_ymin = 0xffffffff;    /* 初值给大点。注意：这里的ymin和ymax定义是基于笛卡尔坐标系的 */
    int line_box_ymax = 0;
    
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

    slot = face->glyph;

	/* 确定座标:
	 * lcd_x = 0
	 * lcd_y = 24
	 * 笛卡尔座标系:
	 * x = lcd_x = 0
	 * y = fb_var.yres - lcd_y = fb_var.yres - 24
	 */
    pen.x = (0) * 64;
    pen.y = (fb_var.yres - 24) * 64;

    for ( n = 0; n < wcslen(ce_mix_str1); n++ )
    {
        /* set transformation */
        FT_Set_Transform( face, 0, &pen );

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, ce_mix_str1[n], FT_LOAD_RENDER );
        if ( error )
        {
            printf("FT_Load_Char failed.\n");
            continue;                 /* ignore errors */
        }

        /* now, draw to our target surface (convert position) */
        draw_bitmap( &slot->bitmap,
                   slot->bitmap_left,
                   fb_var.yres - slot->bitmap_top );

        /* increment pen position */
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;   // 不旋转时，y不会改变的。
    }




    
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
    
