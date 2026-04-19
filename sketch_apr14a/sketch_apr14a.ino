/* TODO
 * Swap button timer control system used for testing with dial
 * changes will occur in the timeSelection method
*/

enum State
{
  OPEN,
  CLOSED,
  DONE
};

//to whoever sets up the wires, make these whichever you need
int latchpin = 2;
int timercontrolpin = 3;

void setup() {
    
    pinMode(latchpin, INPUT);
    pinMode(timercontrolpin, INPUT);
  
    //for the sake of testing connection
    pinMode(BUILTIN_LED, OUTPUT);

   //add output pins you require for display:

}

//I could make a class for these rather than have them as global variables but for the scale of this its not a big deal

//I cant declare these the way our lord and savior James Plank prefers I think i'll switch to vim
State s = OPEN;

unsigned long startTime = 0;

unsigned long timer = 0;
int goal = 0;
int mode = 0;
int pulse = 0;
unsigned long failtime = 0;

//Bool will be 1 when the case is shut
bool latched = 0;

/* This method reads in button presses to swap between 4 modes
 * each 4 modes corrispond to how long the timer will go
*/
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
    startTime = 0;

    //play failure animation

  }
  //timer is completed
  if(s == CLOSED && (now - startTime) >= (unsigned long)goal)
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
    //play idle animation
    //or no animation whatever happens when the case is open
    //perhaps it can display what the goal time is
    //to get that number use goal variable and divide it by 1000; 

  }

}

void loop() {

  latched = digitalRead(latchpin);

  if(s == OPEN) timeSelection();

  stateMachine();

  //timer < goal just to help prevent errors if i made a mistake
  if(s == CLOSED) timer = millis() - startTime;

}


