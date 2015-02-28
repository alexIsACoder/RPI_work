#include <wiringPi.h>
#include <wiringPiSPI.h>

#define	TRUE	(1==1)
#define	FALSE	(!TRUE)

#define	SPI_CHAN		0
#define	NUM_TIMES		100
#define	MAX_SIZE		(1024*1024)

#define PAR_PIN		6
#define RES_PIN		0
#define DC_PIN		1

static int myFd;

int init_spi()
{
	wiringPiSetup() ;

	if ((myFd = wiringPiSPISetup (SPI_CHAN, speed * 1000000)) < 0)
	{	
		fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
		exit (EXIT_FAILURE) ;
	}

	pinMode(DC_PIN, OUTPUT);      //set pin to output 

}

void WriteCommand(unsigned com)
{
	delay(18);
	digitalWrite(DHT11PIN,HIGH);   //set to high 20-40us
	delayMicroseconds(40);

	//CS=1;
	//DC=0;
	//CS=0;
	digitalWrite(DC_PIN, LOW);    //set to low at least 18ms 
}

void WriteData(unsigned dat)
{
	unsigned char i;
	CS=1;
	DC=1;
	digitalWrite(DC_PIN, HIGH);    //set to low at least 18ms 
	CS=0;
	for(i=0;i<8;i++)
	{
		CLK=0;
		DATA=dat&0x80;
		dat<<=1;
		CLK=1;
	}
	CS=1;
}

void esbusini(void)
{
	unsigned char i=100;
	ps = 0;

	RES=1;	
	RES=0;
	while(--i)_nop_();//延时5uS以上
	RES=1;
}

void ini_dis(void)
{
	unsigned char i,j;
	for(i=0;i<8;i++)
	{
		WriteCommand (0xb0+i);    //设置显示位置—行
		WriteCommand (0x02);      //设置显示位置—列低地址
		WriteCommand (0x10);      //设置显示位置—列高地址
		for(j=0;j<128;j++)
			WriteData(0x00);        //屏幕显示，全亮
	}
}

void ini_oled(void)
{
	esbusini();
	/*************************************************
	// SSD1303 Initialization Command
	 *************************************************/
	// Lower Column Address
	WriteCommand(0x00); /* Set Lower Column Address */
	// High Column Address
	WriteCommand(0x10); /* Set Higher Column Address*/
	// Display Start Line
	WriteCommand(0x40); /* Set Display Start Line */
	// Contrast Control Register
	WriteCommand(0x81); /* Set Contrast Control */
	WriteCommand(0x1a); /* 0 ~ 255 */
	// Re-map
	WriteCommand(0xA1); /* [A0]:column address 0 is map 
						   to SEG0 , [A1]: columnaddress 131 is map to SEG0*/ 
	// Entire Display ON/OFF
	WriteCommand(0xA4); /* A4=ON */
	// Normal or Inverse Display
	WriteCommand(0XA6); /* Normal Display*/
	// Multiplex Ratio
	WriteCommand(0xA8); /* Set Multiplex Ratio */
	WriteCommand(0x3f); /* Set to 36 Mux*/
	// Set DC-DC
	WriteCommand(0xAD); /* Set DC-DC */
	WriteCommand(0x8A); /* 8B=ON, 8A=Off */
	// Display ON/OFF
	WriteCommand(0xAE); /* AF=ON , AE=OFF*/
	// Display Offset
	WriteCommand(0xD3); /* Set Display Offset */
	WriteCommand(0x00); /* No offset */
	// Display Clock Divide
	WriteCommand(0xD5); /* Set Clock Divide */
	WriteCommand(0x20); /* Set to 80Hz */
	// Area Color Mode
	WriteCommand(0xD8); /* Set Area Color On or Off*/
	WriteCommand(0x00); /* Mono Mode */
	// COM Pins Hardware Configuration
	WriteCommand(0xDA); /* Set Pins HardwareConfiguration */
	WriteCommand(0x12);
	// VCOMH
	WriteCommand(0xDB); /* Set VCOMH */
	WriteCommand(0x00);
	// VP
	WriteCommand(0xD9); /* Set VP */
	WriteCommand(0x22); /* P1=2 , P2=2 */
	WriteCommand(0xc0);//配置成标准应用
	ini_dis();

	//	P2M0=0XFF;P2M1=0X00;	
	WriteCommand(0xAD); // Set DC-DC 
	WriteCommand(0x8B); // 8B=ON, 8A=Off													   1
	WriteCommand(0xaf);

}

void Goto_xy(unsigned char x,unsigned char y)
{
	x += 2;
	WriteCommand(LCD_SET_COL_HI | (x>>4));
	WriteCommand(LCD_SET_COL_LO | (x&0x0f));
	y >>= 3;
	WriteCommand(0xb0 + y & 0x07);
}

// Set pixel on display buffer
void set_pixel(unsigned char x, unsigned char y) {
	unsigned char dat;
	Goto_xy(x, y);
	dat = 0x01 << (y&0x07);				// set dot
	Goto_xy(x, y);
	WriteData(dat);
}

void disc(unsigned char X,unsigned char Y,unsigned char * c)
{
	unsigned char n;
	WriteCommand (0xb7-(Y<<1));
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=0;n<=15;n+=2)
		WriteData(*(c+n));

	WriteCommand (0xb7-(Y<<1)-1);
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=1;n<=15;n+=2)
		WriteData(*(c+n));
}

// X  is 0 to 7
void dish(unsigned char X,unsigned char Y,unsigned char * h)
{
	unsigned char n;
	WriteCommand (0xb7-(Y<<1));
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=0;n<=31;n+=2)
		WriteData(*(h+n));

	WriteCommand (0xb7-(Y<<1)-1);
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=1;n<=31;n+=2)
		WriteData(*(h+n));
}

void disn(unsigned char X,unsigned char Y,unsigned char n)
{
	unsigned char m;
	WriteCommand (0xb7-(Y<<1));
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(m=0;m<=15;m+=2)
		WriteData(*(num[16 + n]+m));                     

	WriteCommand (0xb7-(Y<<1)-1);
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(m=1;m<=15;m+=2)
		WriteData(*(num[16 + n]+m));
}

void Disp_char(unsigned char X,unsigned char Y,unsigned char c)
{
	unsigned char n;
	WriteCommand (0xb7-(Y<<1));
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=0;n<=15;n+=2)
		WriteData(*(num[c-32]+n));

	WriteCommand (0xb7-(Y<<1)-1);
	if(X%2)
		WriteCommand (0x08);
	else
		WriteCommand (0x00);
	WriteCommand (0x10+(X>>1));
	for(n=1;n<=15;n+=2)
		WriteData(*(num[c-32]+n));
}

void Disp_str(unsigned char X,unsigned char Y,unsigned char *s)	//注意字符串长度
{
	int i;
	for(i=0;s[i]!='\0';i++)
	{
		Disp_char(X+i,Y,s[i]);
	}
}

void disp_num(int X,int Y,long n)
{
	char str[10],i,j;
	i = 9;

	str[i] = '\0';

	if(n == 0)
	{
		disn(X, Y, n);
		return;
	}

	while(n > 0)
	{
		str[--i] = n % 10;
		n /= 10;
	}

	for(j = 0;j+i != 9;j++)
	{
		disn(X+j,Y,str[i+j]);
	}

} 

int main (void)
{
	if (wiringPiSPIDataRW (SPI_CHAN, myData, size) == -1)
	{
		printf ("SPI failure: %s\n", strerror (errno)) ;
		spiFail = TRUE ;
		break ;
	}

	end = millis () ;

	if (spiFail)
		break ;

	timePerTransaction        = ((double)(end - start) / (double)NUM_TIMES) / 1000.0 ;
	dataSpeed                 =  (double)(size * 8)    / (1024.0 * 1024.0) / timePerTransaction  ;
	perfectTimePerTransaction = ((double)(size * 8))   / ((double)(speed * 1000000)) ;

	close(myFd) ;

	return 0 ;
}
