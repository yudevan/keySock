.global main
main:
	ldr r0, =srcIP
	ldr r1, =googleIP
	ldr r2, =port
	ldr r3, =hostPort
	bl makeSocket
	
	ldr r0, =msg
	bl sendMSG

	mov r0, #0
	mov r7, #1
	swi #0

srcIP:
	.asciz "192.168.1.7"

googleIP:
	.asciz "173.194.46.99"
port:
	.word 27328
hostPort:
	.word 80
msg:
	.asciz "GET\r\n\r\n"
