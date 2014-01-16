#include "DFI2C.h"
#include <Wire.h>

/**inner define**/
unsigned char Buf[15];
DFI2C URM07(Buf,sizeof(Buf));

#define UNIT_CM 0
#define UNIT_MM 1
#define UNIT_INCH 2
#define UNIT_MICROSECOND 3

/**Todo:Please set parameters what you want**/
const uint8_t URM1Address = 0x11;
const uint8_t URM2Address = 33;


void setup() 
{ 
   Serial.begin(9600);
   delay(1000); //wait for ready
   
   pinMode(13, OUTPUT);
}

void loop() 
{ 
    byte address;
    static boolean ledState = LOW; 

    Serial.println( "**URM I2C test**" );

    address = URM1Address;
    if ( available( address ) )
    {
        Serial.print( "URM " );
        Serial.print( address );
        Serial.print( ": Distance =" );
        Serial.print( getDistance( address ) );
        Serial.print( getUnitString( address ) );
        Serial.print( ", Temperature =" );
        Serial.print( getTemperature( address ) );
        Serial.println( "C" );
    }
    delay(300);
    
    address = URM2Address;    
    if ( available( address ) )
    {
        Serial.print( "URM " );
        Serial.print( address );
        Serial.print( ": Distance =" );
        Serial.print( getDistance( address ) );
        Serial.print( getUnitString( address ) );
        Serial.print( ", Temperature =" );
        Serial.print( getTemperature( address ) );
        Serial.println( "C" );
    }
    
    digitalWrite( 13, ledState );
    ledState = !ledState;
      
    delay(300);   
}



/**URM functions**/

boolean available( unsigned char SSA )
{
    return (0 == URM07.Read(SSA,1,1));       
}




unsigned int setUnit( unsigned char SSA, unsigned char unit )
{
    unsigned char i;
    unsigned char ret;   

    Buf[0] = unit;
    for ( i = 0; i < 5; i++ )
    {       
        ret = URM07.Write(0,11,1);
        if ( 0 == ret )
        {
            return  0;  
        }
        
        delay(5);
    } 
    
    return 0xFF;        
}

/* return 0xFF means error*/
unsigned char getUnit( unsigned char SSA )
{
    unsigned char i;
    unsigned char ret;   

    for ( i = 0; i < 5; i++ )
    {       
        ret = URM07.Read(SSA,11,1);
        if ( 0 == ret )
        {
            return Buf[0];  
        }
        
        delay(5);
    } 
    
    return 0xFF;        
}


const char* getUnitString( unsigned char SSA )
{  
    switch ( getUnit( SSA ) )
    {
        case UNIT_CM:
            return "cm";
            break;
        
        case UNIT_MM:
            return "mm";
            break;
            
        case UNIT_INCH:
            return "inch";
            break;
            
        case UNIT_MICROSECOND:
            return "us";
            break;        
        
        default:
            return "?";
          
    }     
}



unsigned int getDistance( unsigned char SSA )
{
    Buf[0]=0xff;
    (void)URM07.Write(SSA,6,1);
    
    unsigned int distance;    
    distance = (URM07.Read(SSA,13,2)==0) ? (((unsigned int)Buf[0]<<8)+Buf[1]) : 0;
    
    return distance;
}

signed char getTemperature( unsigned char SSA )
{
    signed char temperature;    
    temperature = (URM07.Read(SSA,12,1)==0) ? (*((signed char *)(&Buf[0]))) : -128;
    
    return temperature;
}


void readAllRegister( unsigned char SSA )
{
    Serial.println("Register Value:");
    for ( unsigned char i = 0; i < 15; i++)
    {
        while(URM07.Read(SSA,i,1));
        Serial.print(" R");
        Serial.print(i);
        Serial.print("=0x");
        Serial.println(Buf[0], HEX);        
    }
}

/*If you don't know the address, you can get address when there is only ONE URM on I2C bus .*/
unsigned char getSSA( )
{
    unsigned char i;
    unsigned char ret;

    for ( i = 0; i < 5; i++ )
    {
        ret = URM07.Read(0,8,1);
        if ( 0 == ret )
        {
            return  Buf[0];  
        }
        
        delay(5);
    } 
    
    return 0xFF;
}

/*Set address of URM.
If you don't know the address of the URM, you can set the parameter 'previousSSA' to 0,but ensure there is only ONE URM on I2C bus.
*/
uint8_t setSSA( uint8_t previousSSA, uint8_t newSSA )
{
    uint8_t i;
    uint8_t ret;
    
    if ( (newSSA < 1) || (newSSA > 126) )
    {
         return 0xFF;   
    }

    if ( previousSSA > 127 )
    {
         return 0xFF;   
    }
    
    if ( 0 == previousSSA )
    {
        previousSSA = getSSA();
        if ( 0xFF == previousSSA )
        {
            return 0xFF;
        }        
    }
    
    if ( previousSSA == newSSA )
    {
        return 0; 
    }
    
    ret = 0xFF;
       
    for ( i = 0; i < 3; i++ )    
    {
        Buf[0] = newSSA;
        (void)URM07.Write(previousSSA,8,1);
        delay(10);
               
        if ( 0 == URM07.Read(newSSA,1,1) )
        {           
            break;           
        } 
    } 
	
    return ret;
}  

/*Save parameters to EEPROM*/
void saveParameter( unsigned char SSA )
{
    Buf[0] = 0xee;
    for ( byte i = 0; i < 3; i++ ) 
    {
        if ( 0 == URM07.Write(SSA,6,1) ) break; 
        delay(10); 
    }
    
    delay( 500 );     
}


