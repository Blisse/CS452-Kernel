#include <rtosc/string.h>
#include <rtosc/assert.h>
#include <rtosc/stdlib.h>

void test_string_cmp() {
    T_ASSERT(RtStrCmp("", "") == 0);
    T_ASSERT(RtStrCmp("1", "1") == 0);
    T_ASSERT(RtStrCmp("1", "2") == -1);
    T_ASSERT(RtStrCmp("1", "12") == -1);
    T_ASSERT(RtStrCmp("12", "1") == 1);
}

void test_string_len() {
    T_ASSERT(RtStrLen("") == 0);
    T_ASSERT(RtStrLen("1") == 1);
    T_ASSERT(RtStrLen("12") == 2);
    T_ASSERT(RtStrLen("123") == 3);
    T_ASSERT(RtStrLen("1234") == 4);
}

void test_string_format_nothing() {
    char* buffer;
    int length;
    int written;

    char ten[10];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "");
    T_ASSERT(written == 0);

    written = RtStrPrintFormatted(buffer, length, "1");
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == '1');

    written = RtStrPrintFormatted(buffer, length, "12");
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '2');

    written = RtStrPrintFormatted(buffer, length, "123");
    T_ASSERT(written == 3);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '2');
    T_ASSERT(buffer[2] == '3');

    written = RtStrPrintFormatted(buffer, length, "1234");
    T_ASSERT(written == 4);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '2');
    T_ASSERT(buffer[2] == '3');
    T_ASSERT(buffer[3] == '4');

    written = RtStrPrintFormatted(buffer, length, "12345");
    T_ASSERT(written == 5);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '2');
    T_ASSERT(buffer[2] == '3');
    T_ASSERT(buffer[3] == '4');
    T_ASSERT(buffer[4] == '5');
}

void test_string_format_overflow() {
    char* buffer;
    int length;
    int written;

    char ten[10];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "xxxxxxxxxx");
    T_ASSERT(length == 10);
    T_ASSERT(written == 10);
    T_ASSERT(buffer[8] == 'x');
    T_ASSERT(buffer[9] == '\0');

    written = RtStrPrintFormatted(buffer, 9, "1234567890");
    T_ASSERT(written == 10);
    T_ASSERT(buffer[7] == '8');
    T_ASSERT(buffer[8] == '\0');
    T_ASSERT(buffer[9] == '\0');
}

void test_string_format_integer() {
    char* buffer;
    int length;
    int written;

    char ten[10];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "%d", 0);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == '0');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%d", 1);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%d", 10);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%u", 0);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == '0');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%u", 10);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%d", -1);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '-');
    T_ASSERT(buffer[1] == '1');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%d", -99);
    T_ASSERT(written == 3);
    T_ASSERT(buffer[0] == '-');
    T_ASSERT(buffer[1] == '9');
    T_ASSERT(buffer[2] == '9');
    T_ASSERT(buffer[3] == '\0');
}

void test_string_format_char() {
    char* buffer;
    int length;
    int written;

    char ten[10];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "%c", 'c');
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == 'c');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%c %c", 'c', 'A');
    T_ASSERT(written == 3);
    T_ASSERT(buffer[0] == 'c');
    T_ASSERT(buffer[1] == ' ');
    T_ASSERT(buffer[2] == 'A');
    T_ASSERT(buffer[3] == '\0');
}

void test_string_format_string() {
    char* buffer;
    int length;
    int written;

    char ten[15];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "%s", "hello");
    T_ASSERT(written == 5);
    T_ASSERT(buffer[0] == 'h');
    T_ASSERT(buffer[1] == 'e');
    T_ASSERT(buffer[2] == 'l');
    T_ASSERT(buffer[3] == 'l');
    T_ASSERT(buffer[4] == 'o');
    T_ASSERT(buffer[5] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%s %s", "world", "hello");
    T_ASSERT(written == 11);
    T_ASSERT(buffer[0] == 'w');
    T_ASSERT(buffer[1] == 'o');
    T_ASSERT(buffer[2] == 'r');
    T_ASSERT(buffer[3] == 'l');
    T_ASSERT(buffer[4] == 'd');
    T_ASSERT(buffer[5] == ' ');
    T_ASSERT(buffer[6] == 'h');
    T_ASSERT(buffer[7] == 'e');
    T_ASSERT(buffer[8] == 'l');
    T_ASSERT(buffer[9] == 'l');
    T_ASSERT(buffer[10] == 'o');
    T_ASSERT(buffer[11] == '\0');
}

void test_string_format_hex() {
    char* buffer;
    int length;
    int written;

    char ten[15];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "%x", 5);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == '5');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%x", 10);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == 'a');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%x", 15);
    T_ASSERT(written == 1);
    T_ASSERT(buffer[0] == 'f');
    T_ASSERT(buffer[1] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%x", 16);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%x", 256);
    T_ASSERT(written == 3);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == '0');
    T_ASSERT(buffer[3] == '\0');
}

void test_string_format_leading() {
    char* buffer;
    int length;
    int written;

    char ten[15];

    buffer = ten;
    length = sizeof(ten);

    written = RtStrPrintFormatted(buffer, length, "%02x", 10);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '0');
    T_ASSERT(buffer[1] == 'a');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%02x", 16);
    T_ASSERT(written == 2);
    T_ASSERT(buffer[0] == '1');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%04x", 255);
    T_ASSERT(written == 4);
    T_ASSERT(buffer[0] == '0');
    T_ASSERT(buffer[1] == '0');
    T_ASSERT(buffer[2] == 'f');
    T_ASSERT(buffer[3] == 'f');
    T_ASSERT(buffer[4] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%04u", 255);
    T_ASSERT(written == 4);
    T_ASSERT(buffer[0] == '0');
    T_ASSERT(buffer[1] == '2');
    T_ASSERT(buffer[2] == '5');
    T_ASSERT(buffer[3] == '5');
    T_ASSERT(buffer[4] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%5u", 1);
    T_ASSERT(written == 5);
    T_ASSERT(buffer[0] == ' ');
    T_ASSERT(buffer[1] == ' ');
    T_ASSERT(buffer[2] == ' ');
    T_ASSERT(buffer[3] == ' ');
    T_ASSERT(buffer[4] == '1');
    T_ASSERT(buffer[5] == '\0');

    written = RtStrPrintFormatted(buffer, length, "%4u", 255);
    T_ASSERT(written == 4);
    T_ASSERT(buffer[0] == ' ');
    T_ASSERT(buffer[1] == '2');
    T_ASSERT(buffer[2] == '5');
    T_ASSERT(buffer[3] == '5');
    T_ASSERT(buffer[4] == '\0');
}

void test_string_consume() {
    char* input = "this is 6";
    char* p = input;
    char token[12];

    INT read = 0;

    read = RtStrConsumeToken(&p, token, sizeof(token));
    T_ASSERT(read == 4);
    T_ASSERT(token[0] == 't');
    T_ASSERT(token[1] == 'h');
    T_ASSERT(token[2] == 'i');
    T_ASSERT(token[3] == 's');
    T_ASSERT(token[4] == '\0');

    read = RtStrConsumeToken(&p, token, sizeof(token));
    T_ASSERT(read == 2);
    T_ASSERT(token[0] == 'i');
    T_ASSERT(token[1] == 's');
    T_ASSERT(token[2] == '\0');

    read = RtStrConsumeToken(&p, token, sizeof(token));
    T_ASSERT(read == 1);
    T_ASSERT(token[0] == '6');
    T_ASSERT(token[1] == '\0');
}

void test_string_scan_format() {
    INT read;

    INT d;
    read = RtStrScanFormatted("1", "%d", &d);

    T_ASSERT(read == 1);
    T_ASSERT(d == 1);

    read = RtStrScanFormatted("-1", "%d", &d);
    T_ASSERT(read == 2);
    T_ASSERT(d == -1);

    read = RtStrScanFormatted("10", "%d", &d);
    T_ASSERT(read == 2);
    T_ASSERT(d == 10);

    read = RtStrScanFormatted("101", "%d", &d);
    T_ASSERT(read == 3);
    T_ASSERT(d == 101);

    UINT u;
    read = RtStrScanFormatted("1", "%u", &u);
    T_ASSERT(read == 1);
    T_ASSERT(u == 1);

    read = RtStrScanFormatted("10", "%u", &u);
    T_ASSERT(read == 2);
    T_ASSERT(u == 10);

    CHAR str[80];
    read = RtStrScanFormatted("hello", "%s", str);
    T_ASSERT(read == 5);
    T_ASSERT(str[0] == 'h');
    T_ASSERT(str[1] == 'e');
    T_ASSERT(str[2] == 'l');
    T_ASSERT(str[3] == 'l');
    T_ASSERT(str[4] == 'o');

    CHAR c;
    read = RtStrScanFormatted("S", "%c", &c);
    T_ASSERT(read == 1);
    T_ASSERT(c == 'S');

    read = RtStrScanFormatted("", "%s", str);
    T_ASSERT(read == -1);

    INT a;
    INT b;
    read = RtStrScanFormatted("1 2", "%d %d", &a, &b);
    T_ASSERT(read == 2);
    T_ASSERT(a == 1);
    T_ASSERT(b == 2);

    read = RtStrScanFormatted("10 C 20", "%d %c %d", &a, &c, &b);
    T_ASSERT(read == 5);
    T_ASSERT(a == 10);
    T_ASSERT(b == 20);
    T_ASSERT(c == 'C');

    read = RtStrScanFormatted("100 D 999", "%d %c %c %d", &a, &c, &c, &b);
    T_ASSERT(read == -1);
}


int main(int argc, char* argv[]) {

    test_string_cmp();
    test_string_len();
    test_string_format_nothing();
    test_string_format_overflow();
    test_string_format_integer();
    test_string_format_char();
    test_string_format_string();
    test_string_format_hex();
    test_string_format_leading();
    test_string_consume();
    test_string_scan_format();

    return STATUS_SUCCESS;
}
