bits 32
;
; División de colegio haha xd
; First divide the upper half of the dividend (since is 64 bit)
; Then use the remainder of the first division to perform the second division
; concatenating the remainder with the lower half of the dividend
; explanation: https://youtu.be/dG8PV6xqm4s?si=MUeE_or-ZpHw16ie&t=1416
;
; [ebp + 0]: old stack pointer - esp
; [ebp + 4]: old frame new instruction - eip
;
;                   [ebp + 8][ebp + 12][ebp + 16]--------[ebp + 20]-------------[ebp + 24]--------------
; void x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t *quotientOut, uint32_t *remainderOut);
;
global x86_div64_32
x86_div64_32:
  ; Make new call fram
  push ebp     ; Save old call frame
  mov ebp, esp  ; Initialize new call frame

  push ebx

  ; Divide upper 32 bits
  mov eax, [ebp + 12]  ; bp + 12: dividend (upper 32 bits)
  mov ecx, [ebp + 16]  ; bp + 16: divisor
  xor edx, edx         ; clear edx
  div ecx              ; Perform the first division: eax - quot, edx - remainder

  ; Store upper 32 bits of quotient
  mov ebx, [ebp + 20]  ; bp + 20: *quotientOut
  mov [ebx + 4], eax   ; bx + 4: upper half of the quotient

  ; Divide lower 32 bits
  mov eax, [ebp + 8]   ; bp + 8: dividend (lower 32 bits)
                       ; edx: old remainder
  div ecx              ; Perform the second division: eax - quot, edx - remainder

  ; Store the results
  mov [ebx], eax       ; bx: lower half of the quotient

  mov bx, [ebp + 24]   ; bp + 24: *remainderOut
  mov [ebx], edx       ; *remainderOut = edx

  pop ebx

  ; Restore old call frame
  mov esp, ebp
  pop ebp
  ret
