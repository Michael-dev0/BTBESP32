enum State
{
  OPEN,
  CLOSED,
  DONE
};

void setup() {
  // put your setup code here, to run once:
    pinMode(2, OUTPUT); // start timer proccess
    pinMode(3, OUTPUT); // timer done
    pinMode(4, INPUT); //latch closed
    
    //making changes for github
}

int timer = 0; 
int goal = 60;
State s = OPEN;

void loop() {
  // put your main code here, to run repeatedly:
  timer = 1;


  if(digitalRead(4) && s == OPEN)
  {
    s = CLOSED;
    //output display command to begin timer proccess

  }

  if(!digitalRead(4) && s == CLOSED)
  {
    s = OPEN;
    //output display command to triggure failure
  }

  if(timer == goal)
  {
    s = DONE;
    //output display command to conclude animation proccess
  }

  if(s == DONE)
  {
    //closed again
    if(digitalRead(4))
    {
      //do something
    } 

  }

  //connection testing
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, HIGH);
  delay(1000);
}


