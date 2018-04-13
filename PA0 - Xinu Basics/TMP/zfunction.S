.global zfunction

.text

zfunction:
	movl 4(%esp), %eax
	andl $0xff803fff, %eax
	shll $4, %eax
	ret
