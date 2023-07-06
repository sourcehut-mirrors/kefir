extern int x, y;

void test() {
begin:
    asm("xor rax, rax" ::: "rax");
    asm("add rax, %0" ::"i"(100));
    if (x) {
        asm("sub %0, %1" : "+r"(x) : "i"(100));
    }
    if (y) {
        asm("add %0, %1" : "+r"(x), "=r"(y) : "i"(100), "r"(x), "1"(314));
    }

    asm("" :: ::begin, end);
end:
    return;
}
