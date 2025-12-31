/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/

#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "ssd1306_i2c.h"

static uint32_t getMyIp( char const * const interface, char * myIp, size_t len );
static void initSignalHandlers( void );

void sigInt( int signum )
{
  ssd1306_stopscroll();
  ssd1306_clearDisplay();
  ssd1306_drawString( "SIGINT!" );
  ssd1306_display();
  ssd1306_startscrollleft(00,0x0F);
//  printf( "Caught!\n" );
  exit( 0 );
}

void sigTerm( int signum )
{
  ssd1306_stopscroll();
  ssd1306_clearDisplay();
  ssd1306_drawString( "SIGTERM!" );
  ssd1306_display();
  ssd1306_startscrollleft(00,0x0F);
//  printf( "Caught!\n" );
  exit( 0 );
}

int main( void ) {
  char lanIf[64], wifiIf[64];
  uint32_t oldLanAdr = UINT32_MAX;
  uint32_t oldWifiAdr = UINT32_MAX;
  uint32_t inLanAdr, inWifiAdr;

  initSignalHandlers();
  ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

  ssd1306_display(); //Adafruit logo is visible
  sleep( 5 );
//  char* text = "This is demo for SSD1306 i2c driver for Raspberry Pi";
//  ssd1306_drawString(text);
  for (;;) {
    inLanAdr = getMyIp( "eth0", lanIf, 64 );
    inWifiAdr = getMyIp( "wlan0", wifiIf, 64 );
    if ( ( oldLanAdr != inLanAdr ) || (  oldWifiAdr != inWifiAdr ) ) {
      ssd1306_stopscroll();
      ssd1306_clearDisplay();
      ssd1306_drawString( "lan:  " );
      ssd1306_drawString( lanIf );
      ssd1306_drawString( "wifi: " );
      ssd1306_drawString( wifiIf );
      ssd1306_display();
      ssd1306_startscrollleft(00,0x0F);
      oldLanAdr = inLanAdr;
      oldWifiAdr = inWifiAdr;
    }
    sleep( 5 );
  }
  ssd1306_dim(1);
  ssd1306_startscrollright(00,0x0F);
  sleep(5);

  ssd1306_clearDisplay();
  ssd1306_fillRect(10,10, 50, 20, WHITE);
  ssd1306_fillRect(80, 10, 130, 50, WHITE);
  ssd1306_display();
  return 0;
}

static void initSignalHandlers( void ) {
  struct sigaction action;
  memset( &action, 0, sizeof(action) );
  action.sa_handler = sigInt;
  sigaction( SIGINT, &action, NULL );

  memset( &action, 0, sizeof(action) );
  action.sa_handler = sigTerm;
  sigaction( SIGTERM, &action, NULL );
}

static uint32_t getMyIp( char const * const interface, char * myIp, size_t len )
{
  int fd;
  struct ifreq ifr;
  uint32_t inAdr;

  memset( &ifr, 0, sizeof( ifr ) );

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;

  /* I want IP address attached to the named interface string. */
  strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);
  inAdr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
  if ( 0 ==  inAdr ) {
    snprintf( myIp, len, "not connected!\n" );
  }
  else {
    snprintf( myIp, len, "%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  }
//  printf("%s %ux\n", myIp, inAdr);
  return inAdr;
}
