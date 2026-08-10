// kernel_print.hpp
#pragma once

// ======================= KONFIGURASI LAYAR =======================
constexpr int VGA_WIDTH  = 80;
constexpr int VGA_HEIGHT = 25;
constexpr uint16_t* VGA_BUFFER = reinterpret_cast<uint16_t*>(0xB8000);

// Warna teks default (putih di atas hitam)
constexpr uint8_t DEFAULT_COLOR = 0x0F;

// Posisi kursor global
extern int term_row;
extern int term_col;

// ======================= FUNGSI DASAR =======================
void clear_screen();
void set_color(uint8_t color);
void print_char(char c);
void print_string(const char* str);
void scroll_screen();

// Versi tanpa format
void print_kernel(const char* str);

// Versi dengan format (variadic)
void print_kernel(const char* fmt, ...);
