/*
 * (C) Copyright Shao Miller, 2022-11-05
 * All rights reserved by the author.
 *
 * Compile with: gcc -ansi -pedantic -Wall -Wextra -Werror -o tinyfont tinyfont.c
 */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define countof(arr) (sizeof (arr) / sizeof *(arr))

enum
  {
    byte_all_zeroes = 0,
    byte_value_cnt = 1 << CHAR_BIT,
    byte_all_ones = byte_value_cnt - 1,
    bytes_per_font_character = 2,
    bits_per_pixel = 24,
    font_height = 5,
    font_width = 3,
    max_font_file_line_len = 11,
    max_font_file_lines = 571,
    max_write_line = 4096,
    enum_zero = 0
  };

struct framebuffer
  {
    unsigned char * buf;
    unsigned int bytes_per_pixel;
    unsigned long int cur_x;
    unsigned long int cur_y;
    unsigned int font_scale;
    unsigned long int height;
    unsigned long int width;
  };

/* 'M', 'N', 'm', 'n' contributed by Greg Olszewski */
static const unsigned char default_font[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 146, 32, 45, 0, 85, 85, 223, 125, 165, 82, 170, 106, 18, 0, 94, 102, 51, 61, 213, 85, 210, 37, 0, 40, 192, 1, 0, 8, 160, 2, 106, 43, 147, 116, 231, 115, 231, 121, 237, 73, 207, 121, 207, 123, 167, 18, 239, 123, 239, 121, 16, 4, 16, 20, 84, 68, 56, 14, 17, 21, 167, 32, 239, 115, 234, 91, 235, 58, 79, 114, 107, 59, 207, 115, 207, 19, 79, 123, 237, 91, 151, 116, 39, 123, 93, 86, 73, 114, 253, 47, 253, 95, 111, 123, 239, 19, 111, 79, 239, 90, 143, 120, 151, 36, 109, 123, 109, 43, 207, 114, 173, 90, 173, 36, 167, 114, 79, 114, 136, 8, 39, 121, 42, 0, 0, 112, 17, 0, 152, 43, 201, 123, 120, 114, 228, 123, 80, 103, 106, 22, 234, 57, 201, 91, 130, 36, 130, 52, 233, 90, 73, 50, 200, 127, 200, 91, 192, 123, 120, 31, 120, 79, 80, 19, 240, 56, 186, 36, 64, 123, 64, 43, 192, 85, 64, 85, 64, 21, 56, 117, 212, 68, 146, 36, 145, 21, 17, 69, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
static const unsigned char pixel_off[] = "   ";
static const unsigned char pixel_on[] = "###";

static void fgets_status(const char * msg, int line, FILE * file);
static void framebuffer_write(struct framebuffer * fb, const char * text);
static void pack_font(FILE * font_file);
static void printable_chars(void);
static void simulation(unsigned long int width, unsigned long int height, unsigned long int scale, FILE * simulation_file);
static void unpack_font(void);

int main(int argc, char ** argv)
  {
    char * ep;
    int i;
    int last_errno;
    char * opt_height;
    unsigned long int opt_height_val;
    char * opt_scale;
    unsigned long int opt_scale_val;
    char * opt_width;
    unsigned long int opt_width_val;
    int opt_write;

    (void) framebuffer_write;
    /* Sanity-check */
    for (i = 0; i < argc; ++i)
      {
        if (argv[i] == NULL)
          {
            fprintf(stderr, "Very strange: argc and argv disagree\n");
            return EXIT_FAILURE;
          }
      }
    /* Check mode */
    if (argc == 2 && strcmp(argv[1], "--pack-font") == 0)
      {
        pack_font(stdin);
        return EXIT_SUCCESS;
      }
    if (argc == 2 && strcmp(argv[1], "--unpack-font") == 0)
      {
        unpack_font();
        return EXIT_SUCCESS;
      }
    if (argc == 2 && strcmp(argv[1], "--printable-chars") == 0)
      {
        printable_chars();
        return EXIT_SUCCESS;
      }
    if (argc == 8)
      {
        ep = NULL;
        opt_height = NULL;
        opt_scale = NULL;
        opt_width = NULL;
        opt_write = 0;
        for (i = 1; i < argc; ++i)
          {
            if (strcmp(argv[i], "--write") == 0)
              {
                if (opt_write != 0)
                  {
                    fprintf(stderr, "--write already specified\n");
                    goto usage;
                  }
                opt_write = 1;
                continue;
              }
            if (strcmp(argv[i], "--width") == 0)
              {
                if (opt_width != NULL)
                  {
                    fprintf(stderr, "--width already specified\n");
                    goto usage;
                  }
                ++i;
                if(i >= argc)
                  {
                    fprintf(stderr, "Missing <width>\n");
                    goto usage;
                  }
                opt_width = argv[i];
                if (opt_width[0] == '\0')
                  {
                    fprintf(stderr, "Empty <width>\n");
                    goto usage;
                  }
                errno = 0;
                opt_width_val = strtoul(opt_width, &ep, 0);
                last_errno = errno;
                if (last_errno != 0)
                  {
                    fprintf(stderr, "Error processing <width> option as number:\n  errno:           %d\n  strerror(errno): %s\n", last_errno, strerror(last_errno));
                    goto usage;
                  }
                if (*ep != '\0' || opt_width_val == 0)
                  {
                    fprintf(stderr, "--width <width> must indicate a positive, non-zero number\n");
                    goto usage;
                  }
                continue;
              }
            if (strcmp(argv[i], "--height") == 0)
              {
                if (opt_height != NULL)
                  {
                    fprintf(stderr, "--height already specified\n");
                    goto usage;
                  }
                ++i;
                if (i >= argc)
                  {
                    fprintf(stderr, "Missing <height>\n");
                    goto usage;
                  }
                opt_height = argv[i];
                if (opt_height[0] == '\0')
                  {
                    fprintf(stderr, "Empty <height>\n");
                    goto usage;
                  }
                errno = 0;
                opt_height_val = strtoul(opt_height, &ep, 0);
                last_errno = errno;
                if (last_errno != 0)
                  {
                    fprintf(stderr, "Error processing <height> option as number:\n  errno:           %d\n  strerror(errno): %s\n", last_errno, strerror(last_errno));
                    goto usage;
                  }
                if (*ep != '\0' || opt_height_val == 0)
                  {
                    fprintf(stderr, "--height <height> must indicate a positive, non-zero number\n");
                    goto usage;
                  }
                continue;
              }
            if (strcmp(argv[i], "--scale") == 0)
              {
                if (opt_scale != NULL)
                  {
                    fprintf(stderr, "--scale already specified\n");
                    goto usage;
                  }
                ++i;
                if (i >= argc)
                  {
                    fprintf(stderr, "Missing <scale>\n");
                    goto usage;
                  }
                opt_scale = argv[i];
                if (opt_scale[0] == '\0')
                  {
                    fprintf(stderr, "Empty <scale>\n");
                    goto usage;
                  }
                errno = 0;
                opt_scale_val = strtoul(opt_scale, &ep, 0);
                last_errno = errno;
                if (last_errno != 0)
                  {
                    fprintf(stderr, "Error processing <scale> option as number:\n  errno:           %d\n  strerror(errno): %s\n", last_errno, strerror(last_errno));
                    goto usage;
                  }
                if (*ep != '\0' || opt_scale_val == 0)
                  {
                    fprintf(stderr, "--scale <scale> must indicate a positive, non-zero number\n");
                    goto usage;
                  }
                continue;
              }
            fprintf(stderr, "Invalid option '%s'\n", argv[i]);
            goto usage;
          }
        if (opt_write == 0 || opt_width == NULL || opt_height == NULL || opt_scale == NULL)
          goto usage;
        simulation(opt_width_val, opt_height_val, opt_scale_val, stdin);
        return EXIT_SUCCESS;
      }
    usage:
    printf("Usage:\n\n  ./tinyfont --pack-font\n    Reads a tinyfont file from stdin and outputs the encoded byte-values\n\n  ./tinyfont --unpack-font\n    Decodes the default font and outputs a tinyfont file\n\n  ./tinyfont --printable-chars\n    Display a list of printable characters\n\n  ./tinyfont --write --width <width> --height <height> --scale <scale>\n    Reads from stdin and writes to a simulated framebuffer having the specified dimensions and font-scale\n");
    return EXIT_FAILURE;
  }

static void fgets_status(const char * msg, int line, FILE * file)
  {
    int last_errno;

    last_errno = errno;
    fprintf(stderr, "fgets():         %s\nline:            %d\nfeof():          %d\nferror():        %d\nerrno:           %d\nstrerror(errno): %s\n", msg, line, feof(file), ferror(file), last_errno, strerror(last_errno));
    errno = last_errno;
  }

static void framebuffer_write(struct framebuffer * fb, const char * text)
  {
    int bit_pos;
    unsigned long int byte_pos;
    unsigned int char_height;
    unsigned int char_width;
    const unsigned char * cptr;
    unsigned long int fb_size;
    const unsigned char * pixel;
    unsigned int scaled_char_height;
    unsigned int scaled_char_width;
    unsigned int sx;
    unsigned int sy;
    unsigned int x;
    unsigned int y;

    scaled_char_height = font_height * fb->font_scale;
    scaled_char_width = font_width * fb->font_scale;
    char_height = scaled_char_height + 1;
    char_width = scaled_char_width + 1;
    fb_size = fb->width * fb->height * fb->bytes_per_pixel;
    for (cptr = (const unsigned char *) text; *cptr != 0; ++cptr)
      {
        /* Check for newline or horizontal wrap */
        if (*cptr == '\n' || fb->cur_x + char_width > fb->width + 1)
          {
            fb->cur_x = 0;
            fb->cur_y += char_height;
            if (*cptr == '\n')
              continue;
          }
        /* Check for vertical wrap */
        if (fb->cur_y + char_height > fb->height + 1)
          fb->cur_y = 0;
        bit_pos = 0;
        for (y = 0; y < scaled_char_height; y += fb->font_scale)
          {
            for (x = 0; x < scaled_char_width; x += fb->font_scale)
              {
                for (sy = 0; sy < fb->font_scale; ++sy)
                  {
                    for (sx = 0; sx < fb->font_scale; ++sx)
                      {
                        byte_pos = (fb->width * fb->bytes_per_pixel * (fb->cur_y + y + sy)) + ((fb->cur_x + x + sx) * fb->bytes_per_pixel);
                        /* Bug-guard */
                        if (byte_pos + fb->bytes_per_pixel > fb_size)
                          {
                            fprintf(stderr, "Out of bounds when writing to framebuffer\n");
                            return;
                          }
                        if (default_font[(*cptr * bytes_per_font_character) + (bit_pos / CHAR_BIT)] & (1 << (bit_pos % CHAR_BIT)))
                          pixel = pixel_on;
                          else
                          pixel = pixel_off;
                        memcpy(fb->buf + byte_pos, pixel, fb->bytes_per_pixel);
                      }
                  }
                ++bit_pos;
              }
          }
        fb->cur_x += char_width;
      }
  }

static void pack_font(FILE * font_file)
  {
    int bit_pos;
    char buf[max_font_file_line_len + 1];
    unsigned int character;
    size_t i;
    int j;
    int last_errno;
    int line;
    size_t line_len;
    unsigned char packed_encodings[byte_value_cnt][bytes_per_font_character];
    int pixel;
    int pixel_line;
    char * ret;
    char * search;

    last_errno = errno;
    /* Make unprintable characters blocks of set bits */
    for (i = 0; i < countof(packed_encodings); ++i)
      {
        for (j = 0; j < bytes_per_font_character; ++j)
          packed_encodings[i][j] = byte_all_ones;
      }
    /* Read and process font file */
    for (line = 1; line < max_font_file_lines; ++line)
      {
        /* Read a line */
        errno = 0;
        ret = fgets(buf, sizeof buf, font_file);
        if (ret == NULL)
          {
            fgets_status("NULL", line, font_file);
            break;
          }
        /* It must end with the character to be represented, then a newline */
        search = strchr(buf, '\n');
        if (search == NULL || search == buf)
          {
            fprintf(stderr, "Expected printable character, then newline on line %d\n", line);
            exit(EXIT_FAILURE);
          }
        /* The previous character needs to be printable */
        if (!isprint(search[-1]))
          {
            fprintf(stderr, "Expected printable character before newline on line %d\n", line);
            exit(EXIT_FAILURE);
          }
        /* Now we know the index of the character */
        character = *(unsigned char *) (search - 1);
        /* Clear all bits for the character */
        for (j = 0; j < bytes_per_font_character; ++j)
          packed_encodings[character][j] = byte_all_zeroes;
        /* Read the pixels */
        bit_pos = 0;
        for (pixel_line = 0; pixel_line < font_height; ++pixel_line)
          {
            ++line;
            errno = 0;
            ret = fgets(buf, sizeof buf, font_file);
            if (ret == NULL)
              {
                fgets_status("NULL", line, font_file);
                fprintf(stderr, "Expected line of pixels of '0' or '1' on line %d\n", line);
                exit(EXIT_FAILURE);
              }
            line_len = strlen(ret);
            if (line_len < font_width || (line_len > font_width && buf[font_width] != '\n'))
              {
                fprintf(stderr, "Expected %d pixels of '0' or '1' on line %d\n", font_width, line);
                exit(EXIT_FAILURE);
              }
            /* Process each pixel */
            for (pixel = 0; pixel < font_width; ++pixel)
              {
                if (buf[pixel] != '0' && buf[pixel] != '1')
                  {
                    fprintf(stderr, "Expected '0' or '1' for pixel %d on line %d\n", pixel + 1, line);
                    exit(EXIT_FAILURE);
                  }
                packed_encodings[character][bit_pos / CHAR_BIT] |= (buf[pixel] == '0' ? 0 : 1) << (bit_pos % CHAR_BIT);
                ++bit_pos;
              }
            if (feof(font_file) || ferror(font_file))
              {
                fgets_status("OK", line, font_file);
                break;
              }
          }
        /* Did we get all pixel-lines? */
        if (pixel_line < font_height)
          {
            fprintf(stderr, "Expected %d more lines of pixels after line %d\n", (font_height - 1) - pixel_line, line);
            exit(EXIT_FAILURE);
          }
        if (feof(font_file) || ferror(font_file))
          {
            fgets_status("OK", line, font_file);
            break;
          }
      }
    /* All done */
    printf("Font encodes as these bytes: ");
    i = 0;
    goto jump_in;
    for (; i < countof(packed_encodings); ++i)
      {
        printf(", ");
        jump_in:
        j = 0;
        goto jump_in2;
        for (; j < bytes_per_font_character; ++j)
          {
            printf(", ");
            jump_in2:
            printf("%d", packed_encodings[i][j]);
          }
      }
    printf("\n");
    errno = last_errno;
  }

static void printable_chars(void)
  {
    int i;

    for (i = 0; i < byte_value_cnt; ++i)
      {
        if (isprint(i))
          printf("%d 0x%02X %c\n", i, i, i);
      }
  }

static void simulation(unsigned long int width, unsigned long int height, unsigned long int scale, FILE * simulation_file)
  {
    char buf[max_write_line];
    unsigned long int byte_pos;
    char * cptr;
    struct framebuffer fb;
    int last_errno;
    int line;
    char * ret;
    unsigned long int x;
    unsigned long int y;

    last_errno = errno;

    /* Initialize framebuffer */
    fb.bytes_per_pixel = bits_per_pixel / CHAR_BIT;
    fb.cur_x = 0;
    fb.cur_y = 0;
    fb.font_scale = scale;
    fb.height = height;
    fb.width = width;
    fb.buf = malloc(width * height * fb.bytes_per_pixel);
    if (fb.buf == NULL)
      {
        fprintf(stderr, "Simulation unable to allocate memory\n");
        exit(EXIT_FAILURE);
      }

    /* Turn all pixels off */
    for (y = 0; y < height; ++y)
      {
        for (x = 0; x < width; ++x)
          {
            byte_pos = (width * fb.bytes_per_pixel * y) + (x * fb.bytes_per_pixel);
            memcpy(fb.buf + byte_pos, pixel_off, fb.bytes_per_pixel);
          }
      }
    /* Read and write all input */
    for (line = 1; 1; ++line)
      {
        errno = 0;
        ret = fgets(buf, sizeof buf, stdin);
        if (ret == NULL)
          {
            fgets_status("NULL", line, simulation_file);
            break;
          }
        framebuffer_write(&fb, buf);
        if (feof(simulation_file) || ferror(simulation_file))
          {
            fgets_status("OK", line, simulation_file);
            break;
          }
      }
    /* Display the content of the framebuffer */
    printf("\n+");
    for (x = 0; x < width; ++x)
      printf("-");
    printf("+\n");
    cptr = (char *) fb.buf;
    for (y = 0; y < height; ++y)
      {
        printf("|");
        for (x = 0; x < width; ++x)
          {
            for (byte_pos = 1; byte_pos < fb.bytes_per_pixel; ++byte_pos)
              {
                if (cptr[byte_pos] != *cptr)
                  {
                    printf("X");
                    break;
                  }
              }
            if (byte_pos == fb.bytes_per_pixel)
              printf("%c", *cptr);
            cptr += fb.bytes_per_pixel;
          }
        printf("|\n");
      }
    printf("+");
    for (x = 0; x < width; ++x)
      printf("-");
    printf("+\n");
    free(fb.buf);
    errno = last_errno;
  }

static void unpack_font(void)
  {
    int bit_pos;
    char c;
    size_t character;
    size_t i;
    int x;
    int y;

    for (i = 0; i < sizeof default_font; i += bytes_per_font_character)
      {
        /* Get character index */
        character = i / bytes_per_font_character;
        /* Convert to character */
        c = (char) character;
        /* Skip non-printable characters */
        if (!isprint(c))
          continue;
        /* Numeric detail about character */
        printf("%d 0x%02X %c\n", c, c, c);
        /* Display pixels */
        bit_pos = 0;
        for (y = 0; y < font_height; ++y)
          {
            for (x = 0; x < font_width; ++x)
              {
                printf("%c", (default_font[i + (bit_pos / CHAR_BIT)] & (1 << (bit_pos % CHAR_BIT))) ? '1' : '0');
                ++bit_pos;
              }
            printf("\n");
          }
      }
  }
