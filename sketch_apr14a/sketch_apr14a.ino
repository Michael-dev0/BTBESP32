#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <pgmspace.h>
#include "btb_back.h"
#include "unicycle_1.h"
#include "unicycle_2.h"
#include "unicycle_3.h"

/* TODO
 * Swap button timer control system used for testing with dial
 * changes will occur in the timeSelection method
*/

// HUB75 Pins
#define R1_PIN 9
#define G1_PIN 10
#define B1_PIN 8
#define R2_PIN 18
#define G2_PIN 11
#define B2_PIN 17
#define A_PIN 16
#define B_PIN 12
#define C_PIN 15
#define D_PIN 13
#define E_PIN -1
#define LAT_PIN 14
#define OE_PIN 6
#define CLK_PIN 7

#define latchpin 4

HUB75_I2S_CFG::i2s_pins _pins = {
  R1_PIN, G1_PIN, B1_PIN,
  R2_PIN, G2_PIN, B2_PIN,
  A_PIN, B_PIN, C_PIN, D_PIN, E_PIN,
  LAT_PIN, OE_PIN, CLK_PIN
};

HUB75_I2S_CFG mxconfig(64, 32, 1, _pins);
MatrixPanel_I2S_DMA* dma_display = nullptr;

int animationDelay = 300;
int frame = 1;
int prevAnimationMillis = -1000;

int uni_x = 5;
int uni_y = 0;

enum State
{
  OPEN,
  CLOSED,
  DONE
};

//to whoever sets up the wires, make these whichever you need
int timercontrolpin = 3;

void presentFrame() {
  dma_display->flipDMABuffer();
}

void showColor(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t color = dma_display->color565(r, g, b);
  dma_display->fillScreen(color);
  presentFrame();
}

void drawImage64x32(int x0, int y0) {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      uint16_t color = pgm_read_word(&btb_back[y * 64 + x]);
      dma_display->drawPixel(x0 + x, y0 + y, color);
    }
  }
}

void drawTransparentImage15x28(int x0, int y0, const uint16_t* img, const uint8_t* mask, int w, int h) {
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int i = y * w + x;

      if (pgm_read_byte(&mask[i])) {
        uint16_t color = pgm_read_word(&img[i]);
        if(y0 + y <= 31) dma_display->drawPixel(x0 + x, y0 + y, color);
      }
    }
  }
}

void drawScene(int frameNum) {
  dma_display->clearScreen();
  drawImage64x32(0, 0);

  if (frameNum == 1) {
    drawTransparentImage15x28(uni_x, uni_y, unicycle_1, unicycle_1_mask, 15, 28);
  } else if (frameNum == 2) {
    drawTransparentImage15x28(uni_x, uni_y, unicycle_2, unicycle_2_mask, 15, 28);
  } else {
    drawTransparentImage15x28(uni_x, uni_y, unicycle_3, unicycle_3_mask, 15, 28);
  }
}

void setup() {
   
  pinMode(latchpin, INPUT_PULLUP);
  pinMode(timercontrolpin, INPUT);
 
  //for the sake of testing connection
  pinMode(BUILTIN_LED, OUTPUT);

  //add output pins you require for display:
  mxconfig.double_buff = true;
  mxconfig.clkphase = true;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  dma_display->begin();
  dma_display->setBrightness8(128);
  dma_display->setTextWrap(false);

  dma_display->clearScreen();
  drawScene(frame);
  presentFrame();

  Serial.begin(115200);

}

//I could make a class for these rather than have them as global variables but for the scale of this its not a big deal

//I cant declare these the way our lord and savior James Plank prefers I think i'll switch to vim
State s = OPEN;

unsigned long startTime = millis();

unsigned long timer = 1;
unsigned long goal = 30*1000;
int mode = 0;
int pulse = 0;
unsigned long failtime = 0;

//Bool will be 1 when the case is shut
bool latched = 0;

/* This method reads in button presses to swap between 4 modes
 * each 4 modes corrispond to how long the timer will go
*/
/*
void timeSelection()
{

  if(!pulse && digitalRead(timercontrolpin))
  {
    pulse = 250;
  }
 
  if(pulse == 250)
  {
    if(mode == 3)
    {
      mode = 0;
    }
    else
    {
      mode++;
    }
  }
  else if(pulse > 0)
  {
    pulse--;
  }

  //times 1000 to convert seconds to milleseconds
  if(mode == 0) goal = 30*1000;    // 30 seconds
  if(mode == 1) goal = 360*1000;   //  5 minutes
  if(mode == 2) goal = 1800*1000;  // 30 minutes
  if(mode == 3) goal = 3600*1000;  // 60 minutes
}
*/

/* This is a mealy state machine controling transitions between states.
 *
 * ANIMATIONS WILL GO IN THIS METHOD comments specify which sequence where
 * in order for you as the animator to extract the time
 * use the timer and divide by 1000 to get seconds
 * it is integer division i wouldnt think that would be a problem but it can change if you need it
 *
 * also if you just want to make a method that plays each animation I can integrate them where they need to go
*/
void stateMachine()
{
  unsigned long now = millis();
  //was open now closed
  if(s == OPEN && latched)
  {
    s = CLOSED;

    //start timer
    startTime = now;

    //play beginning animation

  }
  //case was opened early
  if(s == CLOSED && !latched)
  {
    s = OPEN;

    //end and reset timer
    failtime = now - startTime;
    startTime = now;

    //play failure animation

  }
  //timer is completed
  if(s == CLOSED && timer >= goal)
  {
    s = DONE;
  }

  //done and start again
  if(s == DONE)
  {
    //play success animation

    //reset
    if(!latched) s = OPEN;
 
  }

  if(s == OPEN)
  {
    timer = 1;
    //play idle animation
    //or no animation whatever happens when the case is open
    //perhaps it can display what the goal time is
    //to get that number use goal variable and divide it by 1000;

  }

}

int start = 1;


void loop() {
  unsigned long currentMillis = millis();

  latched = !digitalRead(latchpin);
  Serial.printf("latched %d\n", latched);
  Serial.printf("state: %d\n", s);
  stateMachine();

  //53 arbitrary observation
  if(timer > 0) uni_x = (int)(((double)(timer)/(double)(goal))*53); //integer division

  //initialize start
  if(start)
  {
    startTime = millis();
    start = 0;
  }

  if(timer <= goal && s == CLOSED)
  {
    timer = millis()-startTime;
    Serial.printf("timer: %ul Goal: %ul timer/goal = %lf \n", timer, goal, ((double)timer / (double)goal));
  }

  if(currentMillis-prevAnimationMillis >= animationDelay) {
    prevAnimationMillis = currentMillis;
    frame++;
    if(frame > 3) frame = 1;

    drawScene(frame);   // draw full next frame
    presentFrame();
  }

  //dma_display->clearScreen();

  drawScene(frame);
 
  //if(s == OPEN) timeSelection();

}


