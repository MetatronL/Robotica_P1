
#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Arduino.h>

int pinX = A3 , pinY = A4, pinCenter = 0;

const int waitForInputCounts = 4;
const int _up = 1, _down = -1, _right = 2, _left = -2;
const int _justStarted = 0 , _snakePreGame = 101,_snakeReadyToGo = 102, _gameOver = -1;
const int _dotSetAndWaiting = 1, _dotNotSet = 0;

LedControl lc(10,13,11,1);

const int rs = 6, en = 7, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


int gamePause = 0 ;
int     currentStateOfGame, timeOfDelay = 100;
int snakeScore = 0;

int randomValue,randomX,randomY;
int lastDirection, newDirection ; 

int dotState;
int buttonState;

int currentTimeForWait = 0;
int movementProcessed = 1;


//int queue[130] , queueTop  , queueBottom;
int matrix[8][8];


struct coord{
  int x,y;
  coord() {x=0;y=0;}
  coord(int newX,int newY){ x = newX; y = newY; }
  int operator==(const coord &ref){
    return x == ref.x && y == ref.y ;
  }
  
};

class queue{
private: 
  static const int capacity = 120;
  int bot,top;
  int botTimes, topTimes;
  int advanceTop(){
        int old_top = top;
        (++top);
        if(top == capacity){
          ++topTimes;
          top = 0; 
        }
        return old_top;
    }
    int advanceBot(){
        int old_bot = bot;
        (++bot);
        if(bot == capacity){
          ++botTimes;
          bot = 0; 
        }
        return old_bot;
    }
    const int _emptyQueue = -1;

 public:
 
  
  coord holder[capacity];
  queue() {bot=0; top=0; botTimes = 0; topTimes = 0;}

  

  void push_back(int x,int y){
      coord temp(x,y);
      holder[advanceTop()] = temp;
  }
  
  void push_back(coord aux){
      holder[advanceTop()] = aux;
       lc.setLed(0,aux.x,aux.y,true);    
       matrix[aux.x][aux.y] =1 ; 
  }

  coord pop(){
    if( bot > top && botTimes >= topTimes )
      Serial.print("Error bot > top !\n");

     lc.setLed(0,holder[bot].x,holder[bot].y,false);    
       matrix[holder[bot].x][holder[bot].y] =0 ; 
    return holder[advanceBot()];
    
  }
  coord getBot(){
    if( bot > top && botTimes >= topTimes )
      Serial.print("Error bot > top !\n");
    return holder[bot];
    
  }

  coord getTop(){
      if(  top == 0)
        return holder[capacity - 1];
      return holder[top-1];
    
  }

  void turnOnAllStartup(){
      //only use if you are sure the queue has not started from 0 due to usage ( circular queue )
      int index = bot;
      for(; index < top; ++index)
         lc.setLed(0,holder[index].x,holder[index].y,true);    
  }


  
};

queue positionQueue;








 void getNewRandomValuesXY(){
      while(1)
      {
          randomValue = random(0,1023);
          randomX = randomValue%8;
          randomValue = random(0,1023);
          randomY = randomValue%8;
          if( matrix[randomX][randomY] == 0 )
            break;
      }
      matrix[randomX][randomY] = 2;
 }
void writeDebugRandomValuesXY(){
      Serial.print("[");
      Serial.print(randomX,DEC);
      Serial.print(",");
      Serial.print(randomY,DEC);
      Serial.print("]\n");

  
}


/*
void setLedOn(coord positionOfLed){
      // matrix[positionOfLed.x][positionOfLed.y] = 1;
      // lc.setLed(0,positionOfLed.x,positionOfLed.y,true);
       positionQueue.push_back(positionOfLed);

}

void setLedOff(coord oldestPosition){
   // matrix[oldestPosition.x][oldestPosition.y] = 0;
    //lc.setLed(0,oldestPosition.x,oldestPosition.y,false);
   // matrix[oldestPosition.x][oldestPosition.y] = 0; 
}
*/

void setGameOver(){
  lc.clearDisplay(0);     
  for(int i=0; i <= 7; ++i){
      lc.setLed(0,i,i,true);
      lc.setLed(0,i,7-i,true);
  }
  
}


void moveSnake(int currentDirection){
  
    
    
    coord newPosition = positionQueue.getTop(); 
    
    if( currentDirection == _right)
        newPosition.y = (newPosition.y+1)%8;
     else if( currentDirection == _left)
          newPosition.y = (newPosition.y > 0) ? newPosition.y - 1 : 7;
    else if( currentDirection == _up)
          newPosition.x = (newPosition.x < 7 ) ? newPosition.x + 1 : 0;
    else if( currentDirection == _down)
          newPosition.x = (newPosition.x > 0) ? newPosition.x - 1 : 7;

     coord oldestPosition = positionQueue.getBot();
     if(matrix[newPosition.x][newPosition.y] == 2){
        dotState = _dotNotSet ; 
        snakeScore++;
       
    }else if( matrix[newPosition.x][newPosition.y] == 1 &&  !(newPosition == oldestPosition) ){
        currentStateOfGame = _gameOver;
        //setGameOver();
    }else{
          positionQueue.pop();
    }
    //setLedOn(newPosition);
    positionQueue.push_back(newPosition);
}
int isFull(int input){
    return input > 5 || input < 2;
  
}

int readDirectionInput(int lastDirection){
    int xValue = analogRead(pinX);  
    int yValue = analogRead(pinY);

    xValue /= 128;
    yValue /= 128;
    if( isFull(xValue) == 1 &&  isFull(yValue) == 0 ){
          if( xValue > 5)
              return _up;
           else 
              return _down;
    }
    if( isFull(xValue) == 0 &&  isFull(yValue) == 1 ){
          if( yValue > 5)
              return _left;
           else 
              return _right;
    }
    return lastDirection;
}




void setup(){

  
  lc.shutdown(0,false);
  lc.setIntensity(0,10);

  currentStateOfGame = _justStarted;
  randomSeed(analogRead(A0));   
  Serial.begin(9600); 
  dotState = _dotNotSet;

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  
  pinMode(pinCenter, INPUT_PULLUP); 
}


void updateScore(const int &snakeScore){
    lcd.setCursor(0, 1);
    lcd.print(snakeScore);
}

void loop()

{

    // lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  //lcd.scrollDisplayRight();
    updateScore(snakeScore);
    
    buttonState = digitalRead(pinCenter);
    if( buttonState )
      gamePause = 1 - gamePause;
      
      if( gamePause == 0 ) 
      switch( currentStateOfGame  ){
        
            case _justStarted :
                 lc.clearDisplay(1);     
                 currentStateOfGame = _snakePreGame;
                 break;
                 
             case _snakePreGame :

                 positionQueue.push_back(coord(3,1) );
                 positionQueue.push_back(coord(3,2) );
                 positionQueue.push_back(coord(3,3) );
                 //positionQueue.turnOnAllStartup();

                 lastDirection = _right;
                 currentStateOfGame = _snakeReadyToGo;
                 currentTimeForWait = 0;
                 
                 break;
            case _snakeReadyToGo:
                  if( dotState == _dotNotSet ){
                      getNewRandomValuesXY();
                      lc.setLed(0,randomX,randomY,true);
                      writeDebugRandomValuesXY();
                      dotState = _dotSetAndWaiting;
                  }

                   newDirection = readDirectionInput(lastDirection);
                      if(  newDirection != -lastDirection)
                           lastDirection= newDirection ;
                   
                  if( ++currentTimeForWait == waitForInputCounts ){
                      currentTimeForWait = 0;
                     
                      moveSnake(lastDirection);
                  }
                  
                 break;
              case _gameOver:
                 setGameOver();
                  break;
            default:


                 break;
        
      }
  
      

  

     
    delay(timeOfDelay);
}






