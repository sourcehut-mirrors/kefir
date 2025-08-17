#define __atomic_add_fetch(_ptr, _val, _memorder) ({ \
        (void) (_memorder); \
        *(_Atomic(__typeof_unqual__(*(_ptr))) *) (_ptr) += (_val); \
    })
