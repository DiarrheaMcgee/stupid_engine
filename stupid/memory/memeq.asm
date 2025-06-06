global stMemeq
section .text

; rdi is p1
; rsi is p2
; rdx is n

stMemeq:
        endbr64
        push rbx
        test rdx, rdx
        jnz continue
true:
        pop rbx
        mov rax, 1
        ret
false:
        pop rbx
        xor rax, rax
        ret

continue:
        mov rax, rdx
        mov bl, [rsi]
        cmp bl, [rdi]
        jne false

        mov bl, -1[rsi+rdx]
        cmp bl, -1[rdi+rdx]
        jne false

        shr rax, 1

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false

        mov rax, rdx

ae_128:
        cmp rax, 128
        jb ae_64

        vmovdqu  ymm0,       -128[rsi+rax]
        vpcmpeqb ymm4, ymm0, -128[rdi+rax]
        vmovdqu  ymm1,        -96[rsi+rax]
        vpcmpeqb ymm5, ymm1,  -96[rdi+rax]
        vmovdqu  ymm2,        -64[rsi+rax]
        vpcmpeqb ymm7, ymm2,  -64[rdi+rax]
        vmovdqu  ymm3,        -32[rsi+rax]
        vpcmpeqb ymm6, ymm3,  -32[rdi+rax]

        vpand     ymm0, ymm4, ymm5
        vpand     ymm1, ymm6, ymm7
        vpand     ymm2, ymm0, ymm1
	vpmovmskb ecx, ymm2
        inc ecx
        jne false

        sub rax, 128
        jz true
        cmp rax, 128
        jae ae_128

ae_64:
        cmp rax, 64
        jb ae_32

        vmovdqu  ymm0,       -64[rsi+rax]
        vpcmpeqb ymm0, ymm0, -64[rdi+rax]
        vmovdqu  ymm1,       -32[rsi+rax]
        vpcmpeqb ymm1, ymm1, -32[rdi+rax]
        vpand    ymm0, ymm0, ymm1
	vpmovmskb ecx, ymm0
        inc ecx
        jne false

        sub rax, 64
        jz true

ae_32:
        cmp rax, 32
        jb ae_8

        vmovdqu  ymm0,       -32[rsi+rax]
        vpcmpeqb ymm0, ymm0, -32[rdi+rax]
	vpmovmskb ecx, ymm0
        inc ecx
        jne false

        sub rax, 32
        jz true

ae_8:
        cmp rax, 8
        jb ae_4
        mov rbx, -8[rsi+rax]
        cmp rbx, -8[rdi+rax]
        jne false
        sub rax, 8
        jz true

        cmp rax, 8
        jb ae_4
        mov rbx, -8[rsi+rax]
        cmp rbx, -8[rdi+rax]
        jne false
        sub rax, 8
        jz true

ae_4:
        cmp rax, 4
        jb b_4
        mov ecx, -4[rsi+rax]
        cmp ecx, -4[rdi+rax]
        jne false
        sub rax, 4
        jz true

b_4:
        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        dec rax
        jz true

        mov bl, -1[rsi+rax]
        cmp bl, -1[rdi+rax]
        jne false
        jmp true

