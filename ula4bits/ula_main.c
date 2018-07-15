/*
	Descrição:
		Gera um arquivo binário para gravação em EEPROM. Trata-se
		de uma look-up table (LUT) para a criação de uma Unidade
		Lógica Aritimética através da EEPROM. Esse software faz 
		parte do projeto "Processador de 4 bits".

	Autor:
		Felipe Walter Dafico Pfrimer

	Data:
		09/07/2018
*/

/*********************************************************************
	Início do programa
*********************************************************************/

#include<stdio.h>		// Para uso de printf
#include<stdlib.h>		// Para uso de System("PAUSE")
#include<stdint.h>		// Tipos definidos para inteiros

/*
	Definições -->
*/

#define		BIT0	0b00000001
#define		BIT1	0b00000010
#define		BIT2	0b00000100
#define		BIT3	0b00001000
#define		BIT4	0b00010000
#define		BIT5	0b00100000
#define		BIT6	0b01000000
#define		BIT8	0b10000000


/*
	Globais -->
*/

uint8_t a;				// Representa o operando a da ula
uint8_t b;				// Representa o operando b da ula
uint8_t c0;				// Representa a entrada de carry
uint8_t c4;				// Representa a saída de carry
uint8_t z;				// Representa a saída zero
uint8_t ov;				// Representa a saída overflow
uint8_t eq;				// Representa a sa[ida igual
uint8_t f;				// Representa o resultado da ula
uint8_t op;				// Representa a operação da ula:
uint8_t data[0x1fff + 1];		// Dado a ser armazenado na EEPROM
						/*
							op = 0000 --> f <= b
							op = 0001 --> f <= ~a
							op = 0010 --> f <= a and b
							op = 0011 --> f <= a or b
							op = 0100 --> f <= a xor b
							op = 0101 --> f <= a sl b (shift left)
							op = 0110 --> f <= a sr b (shift right)
							op = 0111 --> f <= a + 1
							op = 1000 --> f <= a - 1
							op = 1001 --> f <= a + b
							op = 1010 --> f <= a + b + c0
							op = 1011 --> f <= a - b
							op = 1100 --> f <= a - b - c0
							op = 1101 --> f <= a * b (nibble 0)
							op = 1110 --> f <= a * b (nibble 1)
							op = 1111 --> f <= a / b
						*/

uint16_t addr = 0;			// Endereço da EEPROM:
						/*
							addr[12..9] --> op
							addr[8]		--> c0
							addr[7..4]  --> b
							addr[3..0]  --> a
						*/

/*
	Início do programa -->
*/

int main(void) 
{
	// Criando ou substituindo o arquivo alu_lut.bin:
	FILE * alu_lut;
	if (!(alu_lut = fopen("alu_lut.bin", "wb")))	// Verifica se o arquivo foi criado
	{
		printf("\nErro!\n");
		exit(1);
	}

	// Criando ou substituindo o arquivo alu_lut.bin:
	FILE * log;
	if (!(log = fopen("log.txt", "w+")))	// Verifica se o arquivo foi criado
	{
		printf("\nErro!\n");
		exit(1);
	}
	

	// Criação da LUT:
	for (addr = 0; addr <= 0x1fff; addr++)
	{
		a	= (addr & 0x000f);			// Isolando a
		b	= (addr & 0x00f0) >> 4;		// Isolando b
		c0	= (addr & 0x0100) >> 8;		// Isolando c0
		op	= (addr & 0x1e00) >> 9;		// Isolando op

		c4 = 0; z = 0; ov = 0; eq = 0;	// Zera as flags		
		f = 0;
		data[addr] = 0;

		// Verifica a flag de igualdade:
		if (a == b)
			eq = 1;

		// Verifica cada operação:
		switch (op)
		{
		case 0b000:		// f <= b
			f = b;
			break;
			
		case 0b0001:	// f <= ~a
			f = (~a) & 0x0f;
			break;

		case 0b0010:	// f <= a and b
			f = a & b;
			break;

		case 0b0011:	// f <= a or b
			f = a | b;
			break;

		case 0b0100:	// f <= a xor b
			f = (a ^ b) & 0x0f;
			break;

		case 0b0101:	// f <= a sl b (shift left)
			f = (a << b) & 0x0f;			
			break;

		case 0b0110:	// f <= a sr b (shift right)
			f = (a >> b) & 0x0f;
			break;

		case 0b0111:	// f <= a + 1 
			f = (a + 1);
			if (f & BIT4)	c4 = 1;
			if ((f & BIT3) && (~a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1000:	// f <= a - 1 
			f = a - 1;
			if (f & BIT4)	c4 = 1;
			if ((~f & BIT3) && (a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1001:	// f <= a + b 
			f = (a + b);
			if (f & BIT4)	c4 = 1;
			if ((f & BIT3) && (~a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1010:	// f <= a + b + c0
			f = (a + b + c0); 
			if (f & BIT4)	c4 = 1;
			if ((f & BIT3) && (~a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1011:	// f <= a - b 
			f = (a - b);
			if (f & BIT4)	c4 = 1;
			if ((~f & BIT3) && (a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1100:	// f <= a - b - c0 
			f = (a - b - c0);
			if (f & BIT4)	c4 = 1;
			if ((~f & BIT3) && (a & BIT3))	ov = 1;
			f = f & 0x0f;
			break;

		case 0b1101:	// f <= a * b (nibble 0) 
			f = (a * b) & 0x0f;
			break;

		case 0b1110:	// f <= a * b (nibble 1)
			f = ((a * b) & 0xf0) >> 4;
			break;

		case 0b1111:	// f <= a / b
			if (b != 0)	f = (a / b) & 0x0f;
			break;

		default:
			break;
		}

		if (f == 0)
			z = 1;
		//c4 = (f & BIT5) >> 4;

		data[addr] = (eq << 7) | (ov << 6) | (z << 5) | (c4 << 4) | f;
		fprintf(log, "Addr:\t\t %03X --> %03X\t(c0 = %x)\n", addr, data[addr],c0);
		fprintf(log, "Operation:\t %x\n\n", op);
		fprintf(log, "Operand a:\t %x\n", a);
		fprintf(log, "Operand b:\t %x\n", b);
		fprintf(log, "Result  f:\t %x\n\n", f);
		fprintf(log, "flag c4:\t %x\n", c4);
		fprintf(log, "flag z:\t\t %x\n", z);
		fprintf(log, "flag ov:\t %x\n", ov);
		fprintf(log, "flag eq:\t %x\n", eq);

		fprintf(log, "=====================================\n\n");


	}

	fwrite(data, sizeof(uint8_t), sizeof(data), alu_lut);
	//fprintf(log, "teste\nteste\nteste\n");

	// Fecha os arquivos
	fclose(alu_lut);
	fclose(log);

	// Indica que o arquivo foi criado
	printf("Arquivo binario criado!\n");

	system("PAUSE");

	return 0;
}