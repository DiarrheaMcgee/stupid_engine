global __stCpyBkwd
global __stCpyFwd
section .text

; rdi is dest
; rsi is src
; rdx is n

ALIGN 8
__stCpyFwd:
	endbr64
	test rdx, rdx
	jnz continueF
	mov rax, rdi
	ret
ALIGN 8
continueF:
	push rcx
	push rbx

	xor rcx, rcx

	mov rbx, rdx
	and rbx, 7
	jnz aligned_to_1F

ALIGN 8
bigger_than_8F:
	mov rbx, rdx
	and rbx, 31
	jz bigger_than_32F
	jmp aligned_to_8F
ALIGN 8
bigger_than_32F:
	mov rbx, rdx
	and rbx, 127
	jz aligned_to_128F
	jmp aligned_to_32F

ALIGN 8
aligned_to_1F:
	mov rbx, rdx
	and rbx, 3
	jz aligned_to_4F
ALIGN 8
aligned_to_1_loopF:
	mov al,        [rsi+rcx]
	mov [rdi+rcx], al

	inc rcx

	cmp rcx, rbx
	jb aligned_to_1_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_4F:
	mov rbx, rdx
	and rbx, 7
	sub rbx, rcx
	jz aligned_to_8F
ALIGN 8
aligned_to_4_loopF:
	mov eax,       [rsi+rcx]
	mov [rdi+rcx], eax

	add rcx, 4

	cmp rcx, rbx
	jb aligned_to_4_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_8F:
	mov rbx, rdx
	and rbx, 15
	sub rbx, rcx
	jz aligned_to_16F
ALIGN 8
aligned_to_8_loopF:
	mov rax,       [rsi+rcx]
	mov [rdi+rcx], rax

	add rcx, 8

	cmp rcx, rbx
	jb aligned_to_8_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_16F:
	mov rbx, rdx
	and rbx, 31
	sub rbx, rcx
	jz aligned_to_32F
ALIGN 8
aligned_to_16_loopF:
	vmovdqu xmm0,	   [rsi+rcx]
	vmovdqu [rdi+rcx], xmm0

	add rcx, 16

	cmp rcx, rbx
	jb aligned_to_16F

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_32F:
	mov rbx, rdx
	and rbx, 63
	sub rbx, rcx
	jz aligned_to_64F
ALIGN 8
aligned_to_32_loopF:
	vmovdqu ymm0,	   [rsi+rcx]
	vmovdqu [rdi+rcx], ymm0

	add rcx, 32

	cmp rcx, rbx
	jb aligned_to_32_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_64F:
	mov rbx, rdx
	and rbx, 127
	sub rbx, rcx
	jz aligned_to_128F
ALIGN 16
aligned_to_64_loopF:
	vmovdqu   ymm1,        [rsi+rcx]
	vmovdqu   ymm0,      32[rsi+rcx]
	vmovdqu   [rdi+rcx],   ymm1
	vmovdqu 32[rdi+rcx],   ymm0

	add rcx, 64

	cmp rcx, rbx
	jb aligned_to_64_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 8
aligned_to_128F:
	mov rbx, rdx
	and rbx, 255
	sub rbx, rcx
	jz aligned_to_256_loopF
ALIGN 16
aligned_to_128_loopF:
	vmovdqu   ymm3,        [rsi+rcx]
	vmovdqu   ymm2,      32[rsi+rcx]
	vmovdqu   ymm1,      64[rsi+rcx]
	vmovdqu   ymm0,      96[rsi+rcx]
	vmovdqu   [rdi+rcx],   ymm3
	vmovdqu 32[rdi+rcx],   ymm2
	vmovdqu 64[rdi+rcx],   ymm1
	vmovdqu 96[rdi+rcx],   ymm0

	add rcx, 128

	cmp rcx, rbx
	jb aligned_to_128_loopF

	cmp rcx, rdx
	jge exitF

ALIGN 32
aligned_to_256_loopF:
	vmovdqu    ymm7,         [rsi+rcx]
	vmovdqu    ymm6,       32[rsi+rcx]
	vmovdqu    ymm5,       64[rsi+rcx]
	vmovdqu    ymm4,       96[rsi+rcx]
	vmovdqu    ymm3,      128[rsi+rcx]
	vmovdqu    ymm2,      160[rsi+rcx]
	vmovdqu    ymm1,      192[rsi+rcx]
	vmovdqu    ymm0,      224[rsi+rcx]
	vmovdqu    [rdi+rcx],    ymm7
	vmovdqu  32[rdi+rcx],    ymm6
	vmovdqu  64[rdi+rcx],    ymm5
	vmovdqu  96[rdi+rcx],    ymm4
	vmovdqu 128[rdi+rcx],    ymm3
	vmovdqu 160[rdi+rcx],    ymm2
	vmovdqu 192[rdi+rcx],    ymm1
	vmovdqu 224[rdi+rcx],    ymm0

	add rcx, 256

	cmp rcx, rdx
	jb aligned_to_256_loopF
ALIGN 8
exitF:
	pop rbx
	pop rcx
	mov rax, rdi
	ret

ALIGN 8
__stCpyBkwd:
	endbr64
	test rdx, rdx
	jnz continueB
	mov rax, rdi
	ret
ALIGN 16
continueB:
	push rbx
	mov rbx, rdx

	cmp rbx, 256
	jae aligned_to_256_B
	cmp rbx, 128
	jae aligned_to_128_B
	cmp rbx, 64
	jae aligned_to_64_B
	cmp rbx, 32
	jae aligned_to_32_B
	cmp rbx, 16
	jae aligned_to_16_B
	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 32
aligned_to_256_B:
	vmovdqu     ymm0,      -256[rsi+rbx]
	vmovdqu     ymm1,      -224[rsi+rbx]
	vmovdqu     ymm2,      -192[rsi+rbx]
	vmovdqu     ymm3,      -160[rsi+rbx]
	vmovdqu     ymm4,      -128[rsi+rbx]
	vmovdqu     ymm5,       -96[rsi+rbx]
	vmovdqu     ymm6,       -64[rsi+rbx]
	vmovdqu     ymm7,       -32[rsi+rbx]
	vmovdqu -256[rdi+rbx],     ymm0
	vmovdqu -224[rdi+rbx],     ymm1
	vmovdqu -192[rdi+rbx],     ymm2
	vmovdqu -160[rdi+rbx],     ymm3
	vmovdqu -128[rdi+rbx],     ymm4
	vmovdqu  -96[rdi+rbx],     ymm5
	vmovdqu  -64[rdi+rbx],     ymm6
	vmovdqu  -32[rdi+rbx],     ymm7
	sub rbx, 256
	jz exitB

	cmp rbx, 256
	jae aligned_to_256_B
	cmp rbx, 128
	jae aligned_to_128_B
	cmp rbx, 64
	jae aligned_to_64_B
	cmp rbx, 32
	jae aligned_to_32_B
	cmp rbx, 16
	jae aligned_to_16_B
	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 32
aligned_to_128_B:
	vmovdqu     ymm0,      -128[rsi+rbx]
	vmovdqu     ymm1,       -96[rsi+rbx]
	vmovdqu     ymm2,       -64[rsi+rbx]
	vmovdqu     ymm3,       -32[rsi+rbx]
	vmovdqu -128[rdi+rbx],	   ymm0
	vmovdqu  -96[rdi+rbx],	   ymm1
	vmovdqu  -64[rdi+rbx],	   ymm2
	vmovdqu  -32[rdi+rbx],	   ymm3
	sub rbx, 128
	jz exitB

	cmp rbx, 64
	jae aligned_to_64_B
	cmp rbx, 32
	jae aligned_to_32_B
	cmp rbx, 16
	jae aligned_to_16_B
	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 16
aligned_to_64_B:
	vmovdqu    ymm0,      -64[rsi+rbx]
	vmovdqu    ymm1,      -32[rsi+rbx]
	vmovdqu -64[rdi+rbx],	 ymm0
	vmovdqu -32[rdi+rbx],	 ymm1
	sub rbx, 64
	jz exitB

	cmp rbx, 32
	jae aligned_to_32_B
	cmp rbx, 16
	jae aligned_to_16_B
	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 16
aligned_to_32_B:
	vmovdqu    ymm0,      -32[rsi+rbx]
	vmovdqu -32[rdi+rbx],	 ymm0
	sub rbx, 32
	jz exitB

	cmp rbx, 16
	jae aligned_to_16_B
	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 16
aligned_to_16_B:
	vmovdqu    xmm0,      -16[rsi+rbx]
	vmovdqu -16[rdi+rbx],	 xmm0
	sub rbx, 16
	jz exitB

	cmp rbx, 8
	jae aligned_to_8_B
	jmp aligned_to_1_B

ALIGN 8
aligned_to_8_B:
	mov  rax,        -8[rsi+rbx]
	mov -8[rdi+rbx],   rax
	sub rbx, 8
	jz exitB

ALIGN 8
aligned_to_1_B:
	cmp rbx, 4
	jb aligned_to_1_B
	mov eax,         -4[rsi+rbx]
	mov -1[rdi+rbx],   eax
	sub rbx, 4
	jz exitB
aligned_to_1_B_loop:
	mov al,         -1[rsi+rbx]
	mov -1[rdi+rbx],   al
	dec rbx
	jz exitB
	jmp aligned_to_1_B_loop

ALIGN 8
exitB:
	pop rbx
	mov rax, rdi
	ret
