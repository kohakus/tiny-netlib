#include <fmt/format.h>
#include <cstring>

void print32CharHex(const char* carr) {
    for (int i = 0; i < 31; i++) {
        fmt::print("{:X} ", carr[i]);
    }
    fmt::print("{:X}\n", carr[31]);
}

// test format_to function.
// here we use format_to like snprinf, but type safe. However, not memory safe.
int main() {
    char cbuf[32];
    char ccbuf[32];

    for (int i = 0; i < 32; i++) {
        cbuf[i] = '\0';
        ccbuf[i] = 'a';
    }

    fmt::print("current cbuf: ");
    print32CharHex(cbuf);
    fmt::print("\n");

    fmt::print("current ccbuf: ");
    print32CharHex(ccbuf);
    fmt::print("\n");

    fmt::print("Now format 42.3423513 to cbuf.\n");
    const char* cbuf_ptr = fmt::format_to(cbuf, "{}", 42.3423513);
    fmt::print("cbuf added length: {}\n", cbuf_ptr - cbuf);
    fmt::print("current cbuf: ");
    print32CharHex(cbuf);
    fmt::print("\n");

    fmt::print("Now format 42.3423513 to ccbuf.\n");
    const char* ccbuf_ptr = fmt::format_to(ccbuf, "{}", 42.3423513);
    fmt::print("ccbuf added length: {}\n", ccbuf_ptr - ccbuf);
    fmt::print("current ccbuf: ");
    print32CharHex(ccbuf);
    fmt::print("\n");

    // test pointer format
    char pbuf[32];
    memset(pbuf, 0, sizeof pbuf);
    char x = 'm';
    fmt::print("Now we test fmt pointer format.\n");
    const char* pbuf_ptr = fmt::format_to(pbuf, "{}", static_cast<const void*>(&x));
    fmt::print("pbuf added length: {}\n", pbuf_ptr - pbuf);
    fmt::print("the format pointer(adress) is: {}\n", static_cast<const void*>(&x));
    fmt::print("current pbuf: ");
    print32CharHex(pbuf);
}