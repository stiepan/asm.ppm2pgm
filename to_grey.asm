            section .text
            global to_grey, weight_r, weight_g, weight_b
            extern malloc

malloc_pgm: push rbp
            mov rdi,rsi
            imul rdi,rdx
            call malloc
            pop rbp
            ret

to_grey:    push rbp
            mov rbp,rsp
            push rdi
            push rsi
            push rdx
            call malloc_pgm
            pop rdx
            pop rsi
            pop rdi
            test rax,rax
            je finish

			mov rcx,rsi
			imul rcx,rdx
            mov r11,rcx
			imul r11,3
			mov rdi,[rdi]
			xor r8,r8
			xor r9,r9
			xor r10,r10
			mov r8b,[rel weight_r]
			mov r9b,[rel weight_g]
			mov r10b,[rel weight_b]
l1:			test rcx,rcx
			je finish

			xor r12,r12
			mov r12b,[rdi + r11 - 3]
			imul r12,r8

			xor rdx,rdx
			mov dl,[rdi + r11 - 2]
			imul rdx,r9
			add r12,rdx

			xor rdx,rdx
			mov dl,[rdi + r11 - 1]
			imul rdx,r10
			add r12,rdx

			shr r12,8

			mov [rax + rcx - 1],r12b

			sub r11,3
            dec rcx
			jmp l1

finish:     pop rbp
            ret
        
            section .data
weight_r    db 77 
weight_g    db 151
weight_b    db 28
