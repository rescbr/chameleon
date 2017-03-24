//
//  lzvn.c
//
//  Based on works from Pike R. Alpha and AnV Software (Andy Vandijck).
//  Converted to C by MinusZwei on 9/14/14.
//
//  No dogs allowed.
//

#include <stdio.h>

#include <libkern/OSByteOrder.h>


static short Llzvn_tableref[256] =
{
     1,  1,  1,  1,    1,  1,  2,  3,    1,  1,  1,  1,    1,  1,  4,  3,
     1,  1,  1,  1,    1,  1,  4,  3,    1,  1,  1,  1,    1,  1,  5,  3,
     1,  1,  1,  1,    1,  1,  5,  3,    1,  1,  1,  1,    1,  1,  5,  3,
     1,  1,  1,  1,    1,  1,  5,  3,    1,  1,  1,  1,    1,  1,  5,  3,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     5,  5,  5,  5,    5,  5,  5,  5,    5,  5,  5,  5,    5,  5,  5,  5,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     6,  6,  6,  6,    6,  6,  6,  6,    6,  6,  6,  6,    6,  6,  6,  6,
     6,  6,  6,  6,    6,  6,  6,  6,    6,  6,  6,  6,    6,  6,  6,  6,
     1,  1,  1,  1,    1,  1,  0,  3,    1,  1,  1,  1,    1,  1,  0,  3,
     5,  5,  5,  5,    5,  5,  5,  5,    5,  5,  5,  5,    5,  5,  5,  5,
     7,  8,  8,  8,    8,  8,  8,  8,    8,  8,  8,  8,    8,  8,  8,  8,
     9, 10, 10, 10,   10, 10, 10, 10,   10, 10, 10, 10,   10, 10, 10, 10
};


// mov    (%rdx),%r8
// movzbq (%rdx),%r9
// jmpq   *(%rbx,%r9,8)

#define LABEL_JUMP                               \
	do {                                         \
		r8 = *(uint64_t *)rdx;                   \
		r9 = r8 & 0xFF;                          \
		switch (Llzvn_tableref[r9]) {            \
			case 0:  goto Llzvn_table0;  break;  \
			case 1:  goto Llzvn_table1;  break;  \
			case 2:  return rax;                 \
			case 3:  goto Llzvn_table3;  break;  \
			case 4:  goto Llzvn_table4;  break;  \
			case 5:  return 0;                   \
			case 6:  goto Llzvn_table6;  break;  \
			case 7:  goto Llzvn_table7;  break;  \
			case 8:  goto Llzvn_table8;  break;  \
			case 9:  goto Llzvn_table9;  break;  \
			case 10: goto Llzvn_table10; break;  \
		}                                        \
	} while (0)


size_t lzvn_decode(void *dst,
			size_t dst_size,
			const void *src,
			size_t src_size)
{
    const uint64_t rdi = (const uint64_t)dst;

    size_t   rax = 0;
    
    uint64_t rsi       = dst_size;
    uint64_t rcx       = src_size;
    uint64_t rdx       = (uint64_t)src;
    
    uint64_t r8  = 0;
    uint64_t r9  = 0;
    uint64_t r10 = 0;
    uint64_t r11 = 0;
    uint64_t r12 = 0;

    uint64_t addr           = 0;
    unsigned char byte_data = 0;

    short jmp = 0;
   
    // lea    Llzvn_tableref(%rip),%rbx
    //
    //    this will load the address of the tableref label into the %rbx
    //    register. in our code, this is the 'Llzvn_tableref' array
    //
    //  for clearness, it will be used directly.

    rax = 0;    // xor    %rax,%rax
    r12 = 0;    // xor    %r12,%r12

    // sub    $0x8,%rsi
    // jb     Llzvn_exit
    jmp = rsi < 0x8 ? 1 : 0;
    rsi -= 0x8;
    if (jmp) {
        return 0;
    }

    // lea    -0x8(%rdx,%rcx,1),%rcx
    // cmp    %rcx,%rdx
	// ja     Llzvn_exit
    rcx = rdx + rcx - 0x8;
    if (rdx > rcx) {
        return 0;
    }
    
    LABEL_JUMP;
    

    
Llzvn_table0:
    r9 >>= 0x6;           // shr    $0x6,%r9
    rdx = rdx + r9 + 0x1; // lea    0x1(%rdx,%r9,1),%rdx
    
    // cmp    %rcx,%rdx
    // ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r10 = 0x38;     // mov    $0x38,%r10
    r10 &= r8;      // and    %r8,%r10
    r8 >>= 0x8;     // shr    $0x8,%r8
    r10 >>= 0x3;    // shr    $0x3,%r10
    r10 += 0x3;     // add    $0x3,%r10
    goto Llzvn_l10; // jmp    Llzvn_l10

Llzvn_table1:
    r9 >>= 0x6;            // shr    $0x6,%r9
    rdx = rdx + r9 + 0x2;  // lea    0x2(%rdx,%r9,1),%rdx
    
    // cmp    %rcx,%rdx
	// ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r12 = r8;                // mov    %r8,%r12
    r12 = OSSwapInt64(r12);  // bswap  %r12
    r10 = r12;               // mov    %r12,%r10
    r12 <<= 0x5;             // shl    $0x5,%r12
    r12 >>= 0x35;            // shr    $0x35,%r12
    r10 <<= 0x2;             // shl    $0x2,%r10
    r10 >>= 0x3d;            // shr    $0x3d,%r10
    r10 += 0x3;              // add    $0x3,%r10
    r8  >>= 0x10;            // shr    $0x10,%r8
    goto Llzvn_l10;

    
Llzvn_table3:
    r9 >>= 0x6;           // shr    $0x6,%r9
    rdx = rdx + r9 + 0x3; // lea    0x3(%rdx,%r9,1),%rdx
    
    // cmp    %rcx,%rdx
    // ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r10 = 0x38;     // mov    $0x38,%r10
    r12 = 0xFFFF;   // mov    $0xffff,%r12
    r10 &= r8;      // and    %r8,%r10
    r8 >>= 0x8;     // shr    $0x8,%r8
    r10 >>= 0x3;    // shr    $0x3,%r10
    r12 &= r8;      // and    %r8,%r12
    r8 >>= 0x10;    // shr    $0x10,%r8
    r10 += 0x3;     // add    $0x3,%r10
    goto Llzvn_l10; // jmp    Llzvn_l10

    
Llzvn_table4:
    // add    $0x1,%rdx
    // cmp    %rcx,%rdx
    // ja     Llzvn_exit
    rdx += 1;
    if (rdx > rcx) {
        return 0;
    }
    
    LABEL_JUMP;
    
    
Llzvn_table6:
    r9 >>= 0x3;           // shr    $0x3,%r9
    r9 &= 0x3;            // and    $0x3,%r9
    rdx = rdx + r9 + 0x3; // lea    0x3(%rdx,%r9,1),%rdx
    
    // cmp    %rcx,%rdx
    // ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r10 = r8;       // mov    %r8,%r10
    r10 &= 0x307;   // and    $0x307,%r10
    r8 >>= 0xa;     // shr    $0xa,%r8
    
    // movzbq %r10b,%r12
    r12 = r10 & 0xFF;
    
    r10 >>= 0x8;    // shr    $0x8,%r10
    r12 <<= 0x2;    // shl    $0x2,%r12
    r10 |= r12;     // or     %r12,%r10
    r12 = 0x3FFF;   // mov    $0x3fff,%r12
    r10 += 0x3;     // add    $0x3,%r10
    r12 &= r8;      // and    %r8,%r12
    r8 >>= 0xE;     // shr    $0xe,%r8
    goto Llzvn_l10; // jmp    Llzvn_l10
 
    
Llzvn_table7:
    r8 >>= 0x8;           // shr    $0x8,%r8
    r8 &= 0xFF;           // and    $0xff,%r8
    r8 += 0x10;           // add    $0x10,%r8
    rdx = rdx + r8 + 0x2; // lea    0x2(%rdx,%r8,1),%rdx
    goto Llzvn_l0;
    
    
Llzvn_table8:
    r8 &= 0xF;            // and    $0xf,%r8
    rdx = rdx + r8 + 0x1; // lea    0x1(%rdx,%r8,1),%rdx
    goto Llzvn_l0;        // jmp    Llzvn_l0
    

Llzvn_table9:
    rdx += 0x2; // add    $0x2,%rdx
    
    // cmp    %rcx,%rdx
    // ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r10 = r8;    // mov    %r8,%r10
    r10 >>= 0x8; // shr    $0x8,%r10
    r10 &= 0xFF; // and    $0xff,%r10
    r10 += 0x10; // add    $0x10,%r10
    goto Llzvn_l11;


Llzvn_table10:
    rdx += 1; // add    $0x1,%rdx
    
    //cmp    %rcx,%rdx
   	//ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r10 = r8;       // mov    %r8,%r10
    r10 &= 0xF;     // and    $0xf,%r10
    goto Llzvn_l11; // jmp    Llzvn_l11
    
    
    
    
Llzvn_l10:
    r11 = rax + r9;   // lea    (%rax,%r9,1),%r11
    r11 += r10;       // add    %r10,%r11
    
    // cmp    %rsi,%r11
	// jae    Llzvn_l8
    if (r11 >= rsi) {
        goto Llzvn_l8;
    }
    
    // mov    %r8,(%rdi,%rax,1)
    addr = rdi + rax;
    *((uint64_t *)addr) = r8;
    
    rax += r9;  // add    %r9,%rax
    r8 = rax;   // mov    %rax,%r8
    
    // sub    %r12,%r8
	// jb     Llzvn_exit
    jmp = r8 < r12 ? 1 : 0;
    r8 -= r12;
    if (jmp) {
        return 0;
    }
    
    // cmp    $0x8,%r12
	// jb     Llzvn_l4
    if (r12 < 0x8) {
        goto Llzvn_l4;
    }
    
    
Llzvn_l5:
    do
    {
        // mov    (%rdi,%r8,1),%r9
        addr = rdi + r8;
        r9 = *((uint64_t *)addr);
        
        r8 += 0x8;      // add    $0x8,%r8
        
        // mov    %r9,(%rdi,%rax,1)
        addr = rdi + rax;
        *((uint64_t *)addr) = r9;
        
        rax += 0x8;     // add    $0x8,%rax
        
        // sub    $0x8,%r10
        // ja     Llzvn_l5
        jmp = r10 > 0x8 ? 1 : 0;
        r10 -= 0x8;
    }
    while (jmp);
    
    rax += r10;     // add    %r10,%rax
    
    LABEL_JUMP;
    
    
Llzvn_l8:
    // test   %r9,%r9
	// je     Llzvn_l7
    if (r9 != 0)
    {
        r11 = rsi + 0x8; // lea    0x8(%rsi),%r11
        
        do
        {
            // mov    %r8b,(%rdi,%rax,1)
            addr = rdi + rax;
            byte_data = (unsigned char)(r8 & 0xFF);
            *((unsigned char *)addr) = byte_data;
            
            rax += 0x1; // add    $0x1,%rax
            
            // cmp    %rax,%r11
            // je     Llzvn_exit2
            if (rax == r11) {
                return rax;
            }
            
            r8 >>= 0x8; // shr    $0x8,%r8
            
            // sub    $0x1,%r9
            // jne    Llzvn_l6
            jmp = r9 != 0x1 ? 1 : 0;
            r9 -= 1;
        }
        while (jmp);
    }

    // mov    %rax,%r8
	r8 = rax;
    
    // sub    %r12,%r8
	// jb     Llzvn_exit
    jmp = r8 < r12 ? 1 : 0;
    r8 -= r12;
    if (jmp) {
        return 0;
    }
    
    
Llzvn_l4:
    r11 = rsi + 0x8; // lea    0x8(%rsi),%r11
    
    do
    {
        //  movzbq (%rdi,%r8,1),%r9
        addr = rdi + r8;
        byte_data = *((unsigned char *)addr);
        r9 = byte_data;
        r9 &= 0xFF;
        
        r8 += 0x1; // add    $0x1,%r8
        
        //  mov    %r9b,(%rdi,%rax,1)
        addr = rdi + rax;
        byte_data = (unsigned char)r9;
        *((unsigned char *)addr) = byte_data;
        
        rax += 0x1; // add    $0x1,%rax
        
        // cmp    %rax,%r11
        // je     Llzvn_exit2
        if (rax == r11) {
            return rax;
        }
        
        // sub    $0x1,%r10
        // jne    Llzvn_l9
        jmp = r10 != 0x1 ? 1 : 0;
        r10 -= 0x1;
    }
    while (jmp);
    
	LABEL_JUMP;
    
    
Llzvn_l11:
    r8 = rax;        // mov    %rax,%r8
	r8 -= r12;       // sub    %r12,%r8
	r11 = rax + r10; // lea    (%rax,%r10,1),%r11
	
    // cmp    %rsi,%r11
	// jae    Llzvn_l4
    if (r11 >= rsi) {
        goto Llzvn_l4;
    }
    
	// cmp    $0x8,%r12
	// jae    Llzvn_l5
    if (r12 >= 0x8) {
        goto Llzvn_l5;
    }
    
	goto Llzvn_l4; // jmp    Llzvn_l4
    
    
Llzvn_l0:
    // cmp    %rcx,%rdx
	// ja     Llzvn_exit
    if (rdx > rcx) {
        return 0;
    }
    
    r11 = rax + r8; // lea    (%rax,%r8,1),%r11
    r8 = -r8;       // neg    %r8
    
    // cmp    %rsi,%r11
	// ja     Llzvn_l2
    if (r11 <= rsi)
    {
        r11 = rdi + r11;  // lea    (%rdi,%r11,1),%r11
        
        uint64_t check = 0;
        
        do
        {
            // mov    (%rdx,%r8,1),%r9
            addr = rdx + r8;
            r9 = *(uint64_t *)addr;
            
            // mov    %r9,(%r11,%r8,1)
            addr = r11 + r8;
            *(uint64_t *)addr = r9;
            
            // add    $0x8,%r8
            // jae    Llzvn_l1
            
            check  = UINT64_MAX;
            check -= (uint64_t)r8;
            
            r8 += 0x8;
        }
        while (check >= 0x8);
        
        rax = r11;  // mov    %r11,%rax
        rax -= rdi; // sub    %rdi,%rax
        
        LABEL_JUMP;
    }
    
    r11 = rsi + 0x8; // lea    0x8(%rsi),%r11
    
    do
    {
        //  movzbq (%rdx,%r8,1),%r9
        addr = rdx + r8;
        r9 = *((uint64_t *)addr);
        r9 &= 0xFF;
        
        //  mov    %r9b,(%rdi,%rax,1)
        addr = rdi + rax;
        byte_data = (unsigned char)r9;
        *((unsigned char *)addr) = byte_data;
        
        rax += 0x1; // add    $0x1,%rax
        
        // cmp    %rax,%r11
        // je     Llzvn_exit2
        if (r11 == rax) {
            return rax;
        }
        
        // add    $0x1,%r8
        // jne    Llzvn_l3
        jmp = ((int64_t)r8 + 0x1 == 0) ? 0 : 1;
        r8 += 0x1;
    }
    while (jmp);
    
	LABEL_JUMP;
    
    
    // should never come here.
    return 0;
}
