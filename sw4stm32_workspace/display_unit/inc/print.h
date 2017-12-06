
void printchar(unsigned char value, unsigned char characters, unsigned char zeroleader);
void printshort(unsigned short value, unsigned char characters, unsigned char zeroleader);
void printlong(unsigned long value, unsigned char characters, unsigned char zeroleader);

void print_ram_string(char text[]);
void printstring(char *text);
extern void (*lcd_func)(char character);
