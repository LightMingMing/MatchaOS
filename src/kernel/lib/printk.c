//
// Created by 赵明明 on 2019/11/18.
//
#include "x86.h"
#include "font.h"
#include "stdio.h"
#include "string.h"

static int is_digit(unsigned char ch) {
    return ch >= '0' && ch <= '9';
}

static unsigned long long get_unsigned_int(va_list ap, int lflag) {
    if (lflag > 1) {
        return va_arg(ap, unsigned long long);
    } else if (lflag) {
        return va_arg(ap, unsigned long);
    } else {
        return va_arg(ap, unsigned int);
    }
}

static long long get_int(va_list ap, int lflag) {
    if (lflag > 1) {
        return va_arg(ap, long long);
    } else if (lflag) {
        return va_arg(ap, long);
    } else {
        return va_arg(ap, int);
    }
}

static void
print_num(void (*put_char)(int, void *, void *), void *put_data, void *other, unsigned long long num, unsigned base,
          int width,
          unsigned flag_sign, unsigned flag_pad) {
    const char *digits;
    char pad_char, str[64];
    int i = 0;

    if (flag_sign & FLAG_LOWER)
        digits = "0123456789abcdef";
    else
        digits = "0123456789ABCDEF";
    pad_char = (!(flag_pad & FLAG_PAD_LEFT) && flag_pad & FLAG_PAD_ZERO) ? '0' : ' ';

    if (base == 10 && (flag_sign & (FLAG_SIGN_PLUS | FLAG_SIGN_MINUS))) {
        width--; // '+' | '-'
        if (pad_char == '0')
            put_char(flag_sign & FLAG_SIGN_MINUS ? '-' : '+', put_data, other);
    } else if (flag_sign & FLAG_SIGN_NUM) {
        width -= 2; // '0x'
        if (pad_char == '0') {
            put_char('0', put_data, other);
            put_char('x', put_data, other);
        }
    }

    if (num == 0) {
        str[i++] = '0';
    } else {
        while (num != 0)
            str[i++] = digits[do_div(num, base)];
    }
    width -= i;

    if (!(flag_pad & FLAG_PAD_LEFT)) {
        while (width-- > 0) {
            put_char(pad_char, put_data, other);
        }
    }

    if (pad_char == ' ') {
        if (base == 10 && (flag_sign & (FLAG_SIGN_PLUS | FLAG_SIGN_MINUS))) {
            put_char(flag_sign & FLAG_SIGN_MINUS ? '-' : '+', put_data, other);
        } else if (flag_sign & FLAG_SIGN_NUM) {
            put_char('0', put_data, other);
            put_char('x', put_data, other);
        } else if (base == 8) {
            put_char('0', put_data, other);
        }
    }
    while (i-- > 0)
        put_char(str[i], put_data, other);

    while (width-- > 0) {
        put_char(' ', put_data, other);
    }

}

static void
v_print_fmt(void (*put_char)(int, void *, void *), void *put_data, void *other, const char *fmt, va_list ap) {
    char *str;
    unsigned char ch;
    unsigned char flag_pad, flag_sign;
    unsigned long long num;
    int width, precision, lflag, base;
    while (1) {
        while ((ch = *(unsigned char *) fmt++) != '%') {
            if (ch == '\0')
                return;
            put_char(ch, put_data, other);
        }
        // flags
        flag_pad = 0, flag_sign = 0;
        while (1) {
            ch = *(unsigned char *) fmt++;
            // flags
            if (ch == '-')
                flag_pad |= FLAG_PAD_LEFT;
            else if (ch == ' ')
                flag_pad |= FLAG_PAD_SPACE;
            else if (ch == '0')
                flag_pad |= FLAG_PAD_ZERO;
            else if (ch == '+')
                flag_sign |= FLAG_SIGN_PLUS;
            else if (ch == '#')
                flag_sign |= FLAG_SIGN_NUM;
            else
                break;
        }
        // width
        width = 0;
        if (is_digit(ch)) {
            width = ch - '0';
            while (is_digit(ch = *(unsigned char *) fmt++))
                width = width * 10 + ch - '0';
        } else if (ch == '*') {
            ch = *(unsigned char *) fmt++;
            width = va_arg(ap, int);
            if (width < 0)
                width = -width, flag_pad |= FLAG_PAD_LEFT;
        }
        // precision
        precision = -1;
        if (ch == '.') {
            ch = *(unsigned char *) fmt++;
            if (is_digit(ch)) {
                precision = ch;
                while (is_digit(ch = *(unsigned char *) fmt++))
                    precision = precision * 10 + ch - '0';
            } else if (ch == '*') {
                ch = *(unsigned char *) fmt++;
                precision = va_arg(ap, int);
            }
        }
        // specifier
        if (ch == 'c') {
            if (!(flag_pad & FLAG_PAD_LEFT)) {
                while (--width > 0)
                    put_char(' ', put_data, other);
            }
            put_char((unsigned char) va_arg(ap, int), put_data, other);
            while (--width > 0)
                put_char(' ', put_data, other);
            continue;
        } else if (ch == 's') {
            str = (char *) va_arg(ap, char*);
            if (str == NULL)
                str = "(NULL)";
            width -= strnlen(str, precision);
            if (!(flag_pad & FLAG_PAD_LEFT)) {
                while (width-- > 0)
                    put_char(' ', put_data, other);
            }
            while ((ch = *str++) != '\0' && (precision < 0 || --precision > -1)) {
                put_char(ch, put_data, other);
            }
            while (width-- > 0) {
                put_char(' ', put_data, other);
            }
            continue;
        }
        lflag = 0;
        while (ch == 'l') {
            ch = *(unsigned char *) fmt++;
            lflag++;
        }
        base = 0;
        num = 0;
        if (ch == 'd') {
            base = 10;
            num = get_int(ap, lflag);
            if ((long long) num < 0) {
                num = -(long long) num;
                flag_sign |= FLAG_SIGN_MINUS;
            }
        } else if (ch == 'u') {
            base = 10;
            flag_sign = flag_sign & (unsigned) (!(FLAG_SIGN_MINUS + FLAG_SIGN_PLUS));
            num = get_unsigned_int(ap, lflag);
        } else if (ch == 'o') {
            base = 8;
            num = get_unsigned_int(ap, lflag);
        } else if (ch == 'x') {
            base = 16;
            flag_sign |= FLAG_LOWER;
            num = get_unsigned_int(ap, lflag);
        } else if (ch == 'X') {
            base = 16;
            flag_sign |= FLAG_UPPER;
            num = get_unsigned_int(ap, lflag);
        }
        if (base > 0) {
            print_num(put_char, put_data, other, num, base, width, flag_sign, flag_pad);
        } else if (ch == '%') {
            put_char('%', put_data, other);
        } else {
            put_char('%', put_data, other);
            for (fmt--; fmt[-1] != '%'; fmt--);
        }
    }
}

void print_fmt(void (*put_char)(int, void *, void *), void *put_data, const char *fmt, ...) {
    struct color c = {WHITE, BLACK};
    va_list ap;
    va_start(ap, fmt);
    v_print_fmt(put_char, put_data, &c, fmt, ap);
    va_end(ap);
}

static void new_line(struct position *p) {
    p->x_position = 0;
    p->y_position++;
    p->cur_address = p->FB_address + p->y_position * p->y_char_size * p->x_resolution;
}

static void print_char(int ch, struct position *p, struct color *c) {
    int i, j, old;
    const int tab = 8;
    unsigned int *address = p->cur_address;
    if (p->x_pos_max <= 0)
        p->x_pos_max = p->x_resolution / p->x_char_size;
    if (p->y_pos_max <= 0)
        p->y_pos_max = p->y_resolution / p->y_char_size;

    if (ch == '\n') {
        new_line(p);
    } else if (ch == '\t') {
        old = p->x_position;
        p->x_position = (int) (((unsigned int) p->x_position) & ((unsigned int) -tab)) + tab;
        if (p->x_position >= p->x_pos_max) {
            new_line(p);
        } else {
            p->cur_address += p->x_char_size * (p->x_position - old);
        }
    } else {
        for (i = 0; i < p->y_char_size; i++) {
            j = p->x_char_size;
            while (j-- > 0) {
                if (font_ascii_small[ch][i] & (1u << (unsigned int) j)) {
                    *address = c->fd;
                } else {
                    *address = c->bd;
                }
                address++;
            }
            address += (p->x_resolution - p->x_char_size);
        }
        p->x_position++;
        if (p->x_position == p->x_pos_max) {
            new_line(p);
        } else {
            p->cur_address += p->x_char_size;
        }
    }

    if (p->y_position == p->y_pos_max) {
        p->y_position = 0;
        p->cur_address = p->FB_address;
    }

    // clear for newline
    if (p->x_position == 0) {
        address = p->cur_address;
        for (i = 0; i < p->y_char_size; i++) {
            for (j = 0; j < p->x_resolution; j++) {
                *address = BLACK;
                address++;
            }
        }
    }

}

static void v_print(struct color *c, const char *fmt, va_list ap) {
    v_print_fmt((void *) print_char, &pos, c, fmt, ap);
}

void print(const char *fmt, ...) {
    struct color c = {WHITE, BLACK};
    va_list ap;
    va_start(ap, fmt);
    v_print(&c, fmt, ap);
    va_end(ap);
}

void println(const char *fmt, ...) {
    struct color c = {WHITE, BLACK};
    va_list ap;
    va_start(ap, fmt);
    v_print(&c, fmt, ap);
    va_end(ap);
    print("\n");
}

void print_color(unsigned fd, unsigned bd, const char *fmt, ...) {
    struct color c = {fd, bd};
    va_list ap;
    va_start(ap, fmt);
    v_print(&c, fmt, ap);
    va_end(ap);
}