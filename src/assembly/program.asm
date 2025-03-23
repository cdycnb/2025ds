struc Number
    .low dw 0
    .high dw 0
endstruc

section .data
    num1 resb Number_size
    num2 resb Number_size
    prompt db 'Enter two 32-bit numbers:', 0
    resultMsg db 'The sum is: ', 0

section .bss
    buffer resb 11  ; Buffer for storing input strings

section .text
    global _start

_start:
    ; Prompt user for input
    mov ah, 09h
    lea dx, [prompt]
    int 21h

    ; Read first number
    call ReadNumber
    mov [num1 + Number.low], ax
    mov [num1 + Number.high], dx

    ; Read second number
    call ReadNumber
    mov [num2 + Number.low], ax
    mov [num2 + Number.high], dx

    ; Perform addition
    call AddNumbers

    ; Display result
    mov ah, 09h
    lea dx, [resultMsg]
    int 21h
    call PrintNumber

    ; Exit program
    mov ah, 4Ch
    int 21h

ReadNumber:
    ; Read user input string
    mov ah, 0Ah
    lea dx, [buffer]
    int 21h

    ; Convert string to 32-bit number (implementation needed)
    ret

AddNumbers:
    ; Add two 32-bit numbers
    mov ax, [num1 + Number.low]
    add ax, [num2 + Number.low]
    mov dx, [num1 + Number.high]
    adc dx, [num2 + Number.high]
    ret

PrintNumber:
    ; Convert 32-bit number to string and display (implementation needed)
    ret
