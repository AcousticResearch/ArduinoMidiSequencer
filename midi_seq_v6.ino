/*
  - RotaryEncoder Library
  - Oled
  - MIDI
  - 2x6 note sequence with timing
*/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>
#include <MIDI.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RotaryEncoder knobLeft(4, 2);
RotaryEncoder knobRight(8, 6);

int posLeft = 0;
int posLeftOld = 0;
int minLeft = 0;
int maxLeft = 27;
int posRight = 0;
int posRightOld = 0;
int minRight = 0;

MIDI_CREATE_DEFAULT_INSTANCE();

int notes[] = {40, 40, 46, 46, 40, 46, 40, 40, 44, 44, 40, 44};
int timings[] = {38, 38, 38, 38, 38, 38, 2, 2, 2, 2, 2, 2};
int note = 0;
int sequence[6][8] = {
  { 1, 1, 1, 1, 1, 1, 1, 1},
  { 2, 2, 2, 2, 2, 2, 2, 2},
  { 1, 1, 1, 1, 2, 2, 2, 2},
  { 1, 1, 2, 2, 1, 1, 2, 2},
  { 1, 2, 1, 2, 1, 2, 1, 2},
  { 1, 1, 1, 2, 1, 1, 1, 2}
};
int seq = 0;

unsigned long time_now = 0;
int tmult = 25;
int notetime = 0;

int play = 1;
int midich = 4;
int oldmidich = 4;

// the setup function runs once when you press reset or power the board
void setup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    for (;;); // Don't proceed, loop forever
  }

  pinMode(1, OUTPUT);
  MIDI.begin(4);                      // Launch MIDI and listen to channel 4
}


// the loop function runs over and over again forever
void loop() {
  for( int j = 0; j < 8; j++ )
  {
    for( int i = 0; i < 6; i++ )
    {
      note = ((sequence[seq][j] - 1) * 6) + i;

      time_now = millis();
      printPage();

      if( play == 1 )
      {
        if( timings[i] > 0 )
        {
          int current_note = notes[note];
          MIDI.sendNoteOn(current_note, 127, midich);
      
          notetime = timings[i] * (tmult+1);
      
          while (millis() < time_now + notetime)
          {
            encoderPos();
          }
      
          MIDI.sendNoteOff(current_note, 0, midich);
      
          time_now = millis();
          notetime = timings[i+6] * (tmult+1);
      
          while (millis() < time_now + notetime)
          {
            encoderPos();
          }
        }
        else
        {
          notetime = timings[i+6] * (tmult+1);
      
          while (millis() < time_now + notetime)
          {
            encoderPos();
          }
        }
      }
      else
      {
        while( play == 0 )
        {
          encoderPos();

          if( oldmidich != midich )
          {
            oldmidich = midich;
            play = 1;
            printPage();
          }
        }
      }
    }
  }
}

void printPage()
{
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  if( posLeft < 24 )
  {
    //// DOT ////
    int xpos = (note*20) + 6;
    if( note >= 6 )
    {
      xpos = ((note-6)*20) + 6;
    }
    int ypos = 10;
    display.drawPixel(xpos, ypos-1, SSD1306_WHITE);
    display.drawPixel(xpos+1, ypos, SSD1306_WHITE);
    display.drawPixel(xpos, ypos+1, SSD1306_WHITE);
  }

  if( posLeft < 12 )
  {
    display.drawLine(0, 0, 28, 0, SSD1306_WHITE);
    display.drawLine(0, 2, 28, 2, SSD1306_WHITE);
    display.drawLine(0, 4, 28, 4, SSD1306_WHITE);
    display.drawLine(0, 6, 28, 6, SSD1306_WHITE);
    
    display.setCursor(30,0);
    display.print("NOTES");

    display.drawLine(60, 0, 128, 0, SSD1306_WHITE);
    display.drawLine(60, 2, 128, 2, SSD1306_WHITE);
    display.drawLine(60, 4, 128, 4, SSD1306_WHITE);
    display.drawLine(60, 6, 128, 6, SSD1306_WHITE);

    for( int i = 0; i < 12; i++ )
    {
      int xpos = (i*20) + 6;
      int ypos = 14;
      if( i >= 6 )
      {
        xpos = ((i-6)*20) + 6;
        ypos = 24;
      }
      
      display.setCursor(xpos, ypos);
      if( posLeft == i )
      {
        display.setTextColor(BLACK, WHITE);
      }
      else
      {
        display.setTextColor(WHITE);
      }
      display.print(notes[i]);
    }
  }

  if( posLeft >= 12 && posLeft < 24 )
  {
    display.drawLine(0, 0, 28, 0, SSD1306_WHITE);
    display.drawLine(0, 2, 28, 2, SSD1306_WHITE);
    display.drawLine(0, 4, 28, 4, SSD1306_WHITE);
    display.drawLine(0, 6, 28, 6, SSD1306_WHITE);
    
    display.setCursor(30,0);
    display.print("TIMINGS");

    display.drawLine(72, 0, 128, 0, SSD1306_WHITE);
    display.drawLine(72, 2, 128, 2, SSD1306_WHITE);
    display.drawLine(72, 4, 128, 4, SSD1306_WHITE);
    display.drawLine(72, 6, 128, 6, SSD1306_WHITE);
    
    for( int i = 0; i < 12; i++ )
    {
      int xpos = (i*20) + 6;
      int ypos = 14;
      if( i >= 6 )
      {
        xpos = ((i-6)*20) + 6;
        ypos = 24;
      }
      
      display.setCursor(xpos, ypos);
      if( posLeft == (i+12) )
      {
        display.setTextColor(BLACK, WHITE);
      }
      else
      {
        display.setTextColor(WHITE);
      }
      display.print(timings[i]);
    }
  }

  if( posLeft >= 24 )
  {
    display.drawLine(0, 0, 28, 0, SSD1306_WHITE);
    display.drawLine(0, 2, 28, 2, SSD1306_WHITE);
    display.drawLine(0, 4, 28, 4, SSD1306_WHITE);
    display.drawLine(0, 6, 28, 6, SSD1306_WHITE);
    
    display.setCursor(30,0);
    display.print("SETTINGS");

    display.drawLine(78, 0, 128, 0, SSD1306_WHITE);
    display.drawLine(78, 2, 128, 2, SSD1306_WHITE);
    display.drawLine(78, 4, 128, 4, SSD1306_WHITE);
    display.drawLine(78, 6, 128, 6, SSD1306_WHITE);

    // print sequence
    if( posLeft == 24 )
    {
      display.setTextColor(BLACK, WHITE);
    }
    else
    {
      display.setTextColor(WHITE);
    }
    for( int i = 0; i < 8; i++ )
    {
      display.setCursor(((i*6)+6), 14);
      display.print(sequence[seq][i]);
    }

    // print tmult
    if( posLeft == 25 )
    {
      display.setTextColor(BLACK, WHITE);
    }
    else
    {
      display.setTextColor(WHITE);
    }
    display.setCursor(76, 14);
    display.print("T:");
    display.print(tmult);

    // print play
    if( posLeft == 26 )
    {
      display.setTextColor(BLACK, WHITE);
    }
    else
    {
      display.setTextColor(WHITE);
    }
    display.setCursor(6, 24);
    if( play == 0 )
    {
      display.print("stopped");
    }
    else
    {
      display.print("playing");
    }

    // print midi channel
    if( posLeft == 27 )
    {
      display.setTextColor(BLACK, WHITE);
    }
    else
    {
      display.setTextColor(WHITE);
    }
    display.setCursor(76, 24);
    display.print("M:");
    display.print(midich);
    
  }

  // debug
  /*
  char output [50];
  sprintf(output, "%d _ %.1f _ %d", t_total, t_factor, notetime);
  display.setCursor(0,24);
  display.println(output);
  */

  display.display();
}

void encoderPos()
{
  knobLeft.tick();
  knobRight.tick();

  //// MINS AND MAXES ////
  posLeft = knobLeft.getPosition();
  if( posLeft < minLeft )
  {
    knobLeft.setPosition(minLeft);
    posLeft = minLeft;
  }
  else if( posLeft > maxLeft )
  {
    knobLeft.setPosition(maxLeft);
    posLeft = maxLeft;
  }

  posRight = knobRight.getPosition();
  if( posRight < minRight )
  {
    knobRight.setPosition(minRight);
    posRight = minRight;
  }
  if( posLeft == 24 && posRight > 5 )
  {
    // limit to stay within the sequencer multi-array
    knobRight.setPosition(5);
    posRight = 5;
  }
  if( posLeft == 26 && posRight > 1 )
  {
    // limit to stay within play on/off options
    knobRight.setPosition(1);
    posRight = 1;
  }
  if( posLeft == 27 )
  {
    // limit to stay within midi channels
    if( posRight > 16 )
    {
      knobRight.setPosition(16);
      posRight = 16;
    }
    if( posRight < 1 )
    {
      knobRight.setPosition(1);
      posRight = 1;
    }
  }

  //// CHANGING SETTINGS ////
  if( posLeftOld != posLeft )
  {
    if( posLeft >= 0 && posLeft < 12 )
    {
      posRight = notes[posLeft];
    }
    if( posLeft >= 12 && posLeft < 24 )
    {
      posRight = timings[(posLeft-12)];
    }
    if( posLeft == 24 )
    {
      posRight = seq;
    }
    if( posLeft == 25 )
    {
      posRight = tmult;
    }
    if( posLeft == 26 )
    {
      posRight = play;
    }
    if( posLeft == 27 )
    {
      posRight = midich;
    }
    posRightOld = posRight;
    knobRight.setPosition(posRight);
    posLeftOld = posLeft;
    printPage();
  }

  //// CHANGING VALUES ////
  if( posRightOld != posRight )
  {
    if( posLeft >= 0 && posLeft < 12 )
    {
      notes[posLeft] = posRight;
    }
    if( posLeft >= 12 && posLeft < 24 )
    {
      timings[(posLeft-12)] = posRight;
    }
    if( posLeft == 24 )
    {
      seq = posRight;
    }
    if( posLeft == 25 )
    {
      tmult = posRight;
    }
    if( posLeft == 26 )
    {
      play = posRight;
    }
    if( posLeft == 27 )
    {
      midich = posRight;
      play = 0;
    }
    posRightOld = posRight;
    printPage();
  }
}
