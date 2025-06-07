global stMemset
section .text

stMemset:
        push rcx
        push rbx

        test rdx, rdx
        jz exit

        xor rcx, rcx
        movq xmm0, rsi
        vpbroadcastb ymm0, xmm0
        vmovdqa ymm1, ymm0
        vmovdqa ymm2, ymm1
        vmovdqa ymm3, ymm0
        vmovdqa ymm4, ymm2
        mov rax, rsi

        mov rbx, rdx
        and rbx, 7
        jnz aligned_to_1

bigger_than_8:
        mov rbx, rdx
        and rbx, 31
        jz bigger_than_32
        jmp aligned_to_8
bigger_than_32:
        mov rbx, rdx
        and rbx, 127
        jz aligned_to_128
        jmp aligned_to_32

aligned_to_1:
        mov rbx, rdx
        and rbx, 3
        jz aligned_to_4

aligned_to_1_loop:
        mov [rdi+rcx], al

        inc rcx

        cmp rcx, rbx
        jb aligned_to_1_loop

        cmp rcx, rdx
        jge exit

aligned_to_4:
        mov rbx, rdx
        and rbx, 7
        sub rbx, rcx
        jz aligned_to_8
aligned_to_4_loop:
        mov [rdi+rcx], eax
        
        add rcx, 4
        
        cmp rcx, rbx
        jb aligned_to_4_loop

        cmp rcx, rdx
        jge exit

aligned_to_8:
        mov rbx, rdx
        and rbx, 15
        sub rbx, rcx
        jz aligned_to_16
aligned_to_8_loop:
        mov [rdi+rcx], rax
        
        add rcx, 8
        
        cmp rcx, rbx
        jb aligned_to_8_loop

        cmp rcx, rdx
        jge exit

aligned_to_16:
        mov rbx, rdx
        sub rbx, rcx
        and rbx, 31
        jz aligned_to_32
aligned_to_16_loop:
        vmovdqu [rdi+rcx], xmm0
        
        add rcx, 16
        
        cmp rcx, rbx
        jb aligned_to_16

        cmp rcx, rdx
        jge exit

aligned_to_32:
        mov rbx, rdx
        sub rbx, rcx
        and rbx, 63
        jz aligned_to_64
aligned_to_32_loop:
        vmovdqu [rdi+rcx], ymm0
        
        add rcx, 32
        
        cmp rcx, rbx
        jb aligned_to_32_loop

        cmp rcx, rdx
        jge exit

aligned_to_64:
        mov rbx, rdx
        sub rbx, rcx
        and rbx, 127
        jz aligned_to_128
aligned_to_64_loop:
        vmovdqu [rdi+rcx], ymm1
        vmovdqu 32[rdi+rcx], ymm0
        
        add rcx, 64
        
        cmp rcx, rbx
        jb aligned_to_64_loop

        cmp rcx, rdx
        jge exit

aligned_to_128:
        mov rbx, rdx
        sub rbx, rcx
        and rbx, 255
        jz aligned_to_256
aligned_to_128_loop:
        vmovdqu [rdi+rcx], ymm3
        vmovdqu 32[rdi+rcx], ymm2
        vmovdqu 64[rdi+rcx], ymm1
        vmovdqu 96[rdi+rcx], ymm0
        
        add rcx, 128
        
        cmp rcx, rbx
        jb aligned_to_128_loop

        cmp rcx, rdx
        jge exit

aligned_to_256:
        vmovdqa ymm4, ymm0
        vmovdqa ymm5, ymm1
        vmovdqa ymm6, ymm2
        vmovdqa ymm7, ymm3
aligned_to_256_loop:
        vmovdqu [rdi+rcx], ymm7
        vmovdqu 32[rdi+rcx], ymm6
        vmovdqu 64[rdi+rcx], ymm5
        vmovdqu 96[rdi+rcx], ymm4
        vmovdqu 128[rdi+rcx], ymm3
        vmovdqu 160[rdi+rcx], ymm2
        vmovdqu 192[rdi+rcx], ymm1
        vmovdqu 224[rdi+rcx], ymm0
        
        add rcx, 256
        
        cmp rcx, rdx
        jb aligned_to_256_loop

exit:
        pop rbx
        pop rcx
        mov rax, rdi
        ret

