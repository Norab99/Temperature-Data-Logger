/* LCD module connections */
//LCD is connected to PORTD.
sbit LCD_RS at RE0_bit;
sbit LCD_EN at RE2_bit;
sbit LCD_D4 at RD4_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D7 at RD7_bit;
sbit LCD_RS_Direction at TRISE0_bit;
sbit LCD_EN_Direction at TRISE2_bit;
sbit LCD_D4_Direction at TRISD4_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;

unsigned short readFromRTC(unsigned short address) {
// To read values from RTC: (time & date)
        unsigned short readValues;    //to store the read data.
        I2C1_Start();                //I2C start signal.
        I2C1_Wr(0xD0);               //DS1307 address (0x68) + Write (zero) = 0xD0.
        I2C1_Wr(address);            //DS1307 location
        I2C1_Repeated_Start();       //Issue repeated start signal
        I2C1_Wr(0xD1);               //Address 0x68 + Read (one) = 0xD1
        readValues=I2C1_Rd(0);       //Read 1 byte from DS1307, send not acknowledge
        I2C1_Stop();                 //I2C stop signal.
        return(readValues);           // returns the stored value.
}
void writeOnRTC(unsigned short address, unsigned short writeValues) {
// To write values from RTC: (time & date).
        I2C1_Start();                     //I2C start signal.
        I2C1_Wr(0xD0);                    //DS1307 address (0x68) + Write (zero) = 0xD0.
        I2C1_Wr(address);                 //DS1307 location.
        I2C1_Wr(writeValues);             //Send the data we want to write.
        I2C1_Stop();                      //stop signal.
}
// To display MSB of the BCD number.
unsigned char MSBofBCD(unsigned char x) {
        return ((x >> 4) + '0');    // shift right x (x*16) for 2 unpacked BCD bytes.
}
// To display LSB of the BCD number.
unsigned char LSBofBCD(unsigned char x) {
        return ((x & 0x0F) + '0');  // mask x to appear the LSB only.
}
//Real-time clock vriables:
int second;
int minute;
int hour;
int day;
int month;
int year;
int h;
int sec;
int min;

// Time setting variables:
short set; // to change the value of the clock if needed.
unsigned short setCounter = 0;  // to change each variable in the real time clck (Ex: setCounter=3, changes the hour's value).
unsigned int Temp;

//For printing on LCD
char time[] = "00:00:00";  // initialize the time to apear in the LCD.
char date[] = "00-00-00";  //initialize the date to apear in the LCD.
char Temperature[] = " 00.0 C";   // initialize the temperture to apear in the LCD.

void main() {
        ADCON1 = 0x0E; //IO (7 digital & 1 Analog).
        TRISD=0x00; // clear port D as an output that will apear in LCD.
        TRISE=0X00; // clear port E as an output to initialize the time apearing in the LCD.
        TRISA = 0xFF; //PORTA push buttons as inputs.
        OSCCON = 0x73; //8MHz internal oscillator.
        I2C1_Init(100000); //DS1307 I2C running at 100KHz.
        Lcd_Init(); //Initializing the LCD.
        UART1_INIT(9600); // Initialize the terminal.
        Lcd_out(1,1,"  Digital  Clock  "); // the first line phrase that appears when the code is played.
        Lcd_out(2,1," by: DANAH,NOUF "); // the second line phrase that appears when the code is played.
        Lcd_out(3,1," NORA&RENAD ");
        Delay_ms(3000); // delay the displayed phrases.
        Lcd_Cmd(_LCD_CLEAR); // clear the LCD.
        Lcd_Cmd(_LCD_CURSOR_OFF );   //LCD cursor off
        Sound_Init(&PORTD,3); // initializing the buzzer at portD.

        while(1) {
                        Temp = ADC_Read(0) * 0.489; // Read analog voltage and convert it to degree Celsius (0.489 = 500/1023)
                        if (temp > 99) // if the Temperature is larger than 2 byte then a carry should be added as an addtionao byte.
                        Temperature[0]  = 1 + 48;              // Put the additional byte.
                    else        // if the number is smaller than 99 thus can be represented in 2 bytes.
                    Temperature[1]  = (temp / 10) % 10  + 48;  //  the 1st digit.
                    Temperature[2]  =  temp % 10  + 48;      //  the 2nd digit.
                    Temperature[5] = 223;     // represents the degree symbol ( ° ).

                        // Time setting:
                        set = 0;
                      if(PORTA.B1 == 0) { //set button is pressed.
                        Delay_ms(30);
                        setCounter++; // increment the setCounter.
                        Delay_ms(30);
                     if(setCounter >= 7) // if the counter is greater than 7 re-set it to zero.
                    setCounter = 1;
                }
                if(setCounter) { // A value other than zero.
                        if(PORTA.B3 == 0) {    // if the increment button is pressed.
                                Delay_ms(30);
                                set = 1;   // set the variable as 1 (increment it).
                        }
                        if(PORTA.B4 == 0) {  // if the decrement button is pressed.
                                Delay_ms(30);
                                set = -1;  //set the variable as -1 (decrement it).
                        }
                        if(setCounter && set) { // both setCounter & set are values other than zero.
                                switch(setCounter) {
                                        case 1: //Hours
                                        hour = readFromRTC(2);  //Read the hour from DS1307 (real time clk).
                                        hour = Bcd2Dec(hour); //Convert the read hour from BCD to decimal to inable the increment and decrement.
                                        hour = hour + set;  //increment or decrement hour.
                                        if(hour >= 24)     //If the hour is greater than or equal to 24 re-set it (user increment).
                                                hour = 0; //Reset to 0 (midnight 12am).
                                        else if(hour < 0) //If the hour is less than 0 re-set it (user decrement).
                                                hour = 23;//Go back to 23.
                                        hour = Dec2Bcd(hour);//Convert back to BCD to re-write in as BCD after the adjusments.
                                        writeOnRTC(2, hour);//Write hour on DS1307
                                        break;
                                        case 2: //Minutes
                                        minute = readFromRTC(1); //Read minute from DS1307
                                        minute = Bcd2Dec(minute);  //Convert reading to decimal
                                        minute = minute + set;  //Increment or decrement
                                        if(minute >= 60) //If at 59 and user increments
                                                minute = 0;   //Reset to 0
                                        if(minute < 0)  //If at 0 and user decrements
                                                minute = 59; //Go back to 59
                                        minute = Dec2Bcd(minute); //Convert back to BCD
                                        writeOnRTC(1, minute); //Write hour on DS1307
                                        break;
                                        case 3: //Seconds
                                        second = readFromRTC(0); //Read second from DS1307
                                        second = Bcd2Dec(second); //Convert reading to decimal
                                        second = second + set;  //Increment or decrement
                                        if(second >= 60) //If at 59 and user increments
                                                second = 0; //Reset to 0
                                        if(second < 0) //If at 0 and user decrements
                                                second = 59; //Go back to 59
                                        second = Dec2Bcd(second); //Convert back to BCD
                                        writeOnRTC(0, second); //Write second on DS1307
                                        break;
                                        case 4: //Days
                                        day = readFromRTC(4);  //Read day from DS1307
                                        day = Bcd2Dec(day); //Convert reading to decimal
                                        day = day + set; //Increment or decrement
                                        if(day >= 32) //If at 31 and user increments
                                                day = 1; //Reset to 1
                                        if(day <= 0)  //If at 1 and user decrements
                                                day = 31; //Go back to 31
                                        day = Dec2Bcd(day); //Convert back to BCD
                                        writeOnRTC(4, day); //Write day on DS1307
                                        break;
                                        case 5: //Months
                                        month = readFromRTC(5); //Read month from DS1307
                                        month = Bcd2Dec(month); //Convert reading to decimal
                                        month = month + set; //Increment or decrement
                                        if(month > 12) //If at 12 and user increments
                                                month = 1; //Reset to 1
                                        if(month <= 0) //If at 1 and user decrements
                                                month = 12;  //Go back to 12
                                        month = Dec2Bcd(month); //Convert back to BCD
                                        writeOnRTC(5,month); //Write month on DS1307
                                        break;
                                        case 6: //Years
                                        year = readFromRTC(6); //Read hour from DS1307
                                        year = Bcd2Dec(year); //Convert reading to decimal
                                        year = year + set;  //Increment or decrement
                                        if(year < 15)
                                                year = 30;
                                        if(year >= 30)
                                                year = 15;
                                        year = Dec2Bcd(year);  //Convert back to BCD
                                        writeOnRTC(6, year);   //Write hour on DS1307
                                        break;
                                } // end switch
                        } // end if(setCounter && set)
                } // end if(setCounter)
                // What to display on LCD
                        //Real-time (Arrange the numbers read as MSB & LSB)
                        //Arrange values to print it:
                        time[0] = MSBofBCD(hour);
                        time[1] = LSBofBCD(hour);
                        time[3] = MSBofBCD(minute);
                        time[4] = LSBofBCD(minute);
                        time[6] = MSBofBCD(second);
                        time[7] = LSBofBCD(second);
                        date[0] = MSBofBCD(day);
                        date[1] = LSBofBCD(day);
                        date[3] = MSBofBCD(month);
                        date[4] = LSBofBCD(month);
                        date[6] = MSBofBCD(year);
                        date[7] = LSBofBCD(year);
                        //print them on the LCD:
                                Lcd_out(1, 1, "Time:");
                                Lcd_out(2, 1, "Date:");
                                lcd_out(3, -3, "Temp:");
                                Lcd_out(1, 6, time);
                                Lcd_out(2, 6, date);
                                lcd_out(3, 2, Temperature); // Display LM35 temperature result.
                                Delay_ms(500);
                                /* Read time from DS1307 */

              //(set all Writen Data on the RTC to the arrays).
              // Calling the RTC function.
                second = readFromRTC(0);
                minute = readFromRTC(1);
                hour = readFromRTC(2);
                day = readFromRTC(4);
                month = readFromRTC(5);
                year = readFromRTC(6);
                // Coverting RTC data from BCD to Decimal to enable changes in the value.
                h = Bcd2Dec(hour);
                min = Bcd2Dec(minute);
                sec = Bcd2Dec(second);
                // Terminal Display:

                if (h==8 && min >=0 && min <=15 && sec ==0){
                UART1_WRITE_TEXT("Time:");
                delay_ms(500);
                UART1_WRITE_TEXT(time);
                UART1_WRITE(13);
                UART1_WRITE(10);
                UART1_WRITE_TEXT("Date:");
                delay_ms(500);
                UART1_WRITE_TEXT(date);
                UART1_WRITE(13);
                UART1_WRITE(10);
                UART1_WRITE_TEXT("Temp:");
                delay_ms(500);
                UART1_WRITE_TEXT(Temperature);
                UART1_WRITE(13);
                UART1_WRITE(10);
                // Buzzer codition:
                 if (h==8 && min ==15 ) {
                 Sound_Play(750, 500);
                 }
                }
        } // end while.
} // end main.