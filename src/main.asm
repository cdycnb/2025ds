data segment
    res db 16 dup(0) ; 存储结果的变量
    info1 db 0dh,0ah,'Enter the first number (up to 15 digits): $' ; 提示信息
    info2 db 0dh,0ah,'Enter the second number (up to 15 digits): $' ; 提示信息
    info3 db 0dh,0ah,'Sum is: $' ; 输出提示
    buf1 db 16 ; 初始化
         db ? ; 存储第一个数字的长度
         db 16 dup(0) ; 存储第一个数字内容
    buf2 db 16 ; 初始化
         db ? ; 存储第二个数字的长度
         db 16 dup(0) ; 存储第二个数字的内容
    chunumber  db 10 ; 用于除数
    weiofjin   db 0 ; 存储进位
data ends

stack segment stack
    db  128 dup(?) ; 初始化128字节的空间
stack ends

code segment
assume cs:code,ds:data,ss:stack
start:
    mov ax,data
    mov ds,ax
    lea dx,info1
    mov ah,9
    int 21h
    lea dx,buf1
    mov ah,10
    int 21h
    lea dx,info2
    mov ah,9
    int 21h
    lea dx,buf2
    mov ah,10
    int 21h
    call datacate
    call addwork
    call output
    mov ah,4ch
    int 21h

datacate proc near
first:
    lea si,buf1+2
    mov cl,[buf1+1]
    mov ch,0
    add si,cx
    mov byte ptr [si],0
    lea si,buf1+2
a1:
    push [si]
    inc si
    loop a1
    lea si,buf1+2
    mov cl,[buf1+1]
    mov ch,0
a2:
    pop [si]
    inc si
    loop a2
    mov byte ptr [si],0
    mov cl,[buf1+1]
    mov ch,0
    lea si,buf1+2
    mov bl,30h
a3:
    sub byte ptr [si],bl
    inc si
    loop a3
second:
    lea si,buf2+2
    mov cl,[buf2+1]
    mov ch,0
    add si,cx
    mov byte ptr [si],0
    lea si,buf2+2
b1:
    push [si]
    inc si
    loop b1
    lea si,buf2+2
    mov cl,[buf2+1]
    mov ch,0
b2:
    pop [si]
    inc si
    loop b2
    mov byte ptr [si],0
    mov cl,[buf2+1]
    mov ch,0
    lea si,buf2+2
    mov bl,30h
b3:
    sub byte ptr [si],bl
    inc si
    loop b3
    ret
datacate endp

addwork proc near
    lea si,buf1+2
    lea di,buf2+2
    lea bx,res
    mov cx,15
cc:
    mov al,byte ptr [si]
    mov dl,byte ptr [di]
    add al,dl
    add al,weiofjin
    mov ah,0
    div chunumber
    mov byte ptr [bx],ah
    mov weiofjin,al
    inc si
    inc di
    inc bx
    loop cc
    mov byte ptr [bx],al
    ret
addwork endp

output proc near
    mov al,[buf1+1]
    mov dl,[buf2+1]
    cmp al,dl
    jge d1
    mov al,dl
d1:
    lea si,res
    mov ah,0
    add si,ax
d2:
    mov cl,byte ptr [si]
    mov ch,0
    jcxz d3
    inc si
d3:
    mov cx,si
    lea ax,res
    sub cx,ax
    dec si
    mov bl,30h
    lea dx,info3
    mov ah,9
    int 21h
    mov dl,10
    mov ah,02
    int 21h
d4:
    add byte ptr [si],bl
    mov dl,byte ptr [si]
    mov ah,02
    int 21h
    dec si
    loop d4
    ret
output endp

code ends
end start
