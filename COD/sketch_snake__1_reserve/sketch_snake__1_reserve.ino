
#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Arduino.h>

int pinX = A3 , pinY = A4, pinCenter = 0;

const int waitForInputCounts = 4;

const int _justStarted = 0 , _snakePreGame = 101,_snakeReadyToGo = 102, _gameOver = -1 , _snake = 201;
const int _dotSetAndWaiting = 1, _dotNotSet = 0;

LedControl lc(10,13,11,1);

const int rs = 6, en = 7, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


int gamePause = 0 ;
int     currentStateOfGame, defaultDelay = 100;



int randomValue,randomX,randomY;
//int lastDirection, newDirection ; 

int dotState;
int centerButtonState = 0, xValue = 0, yValue = 0;

int currentTimeForWait = 0;
int movementProcessed = 1;


//int queue[130] , queueTop  , queueBottom;
int matrix[8][8];

int isFull(int input){
        return input > 5 || input < 2;
      
    }

void setGameOver(){
  lc.clearDisplay(0);     
  for(int i=0; i <= 7; ++i){
      lc.setLed(0,i,i,true);
      lc.setLed(0,i,7-i,true);
  }
  
}

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
  coord holder[capacity];
  int bot,top;
  int botTimes, topTimes;
  const int _emptyQueue = -1;
  int advanceTop(){
        int old_top = top++;
        if(top == capacity){
          ++topTimes;
          top = 0; 
        }
        return old_top;
    }
    int advanceBot(){
        int old_bot = bot++;
        if(bot == capacity){
          ++botTimes;
          bot = 0; 
        }
        return old_bot;
    }
    

 public:
 
  
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



class game{
    int score;
    int machineDelay , loopWaitCount ;
    int preferredDelay; int currentLoop;

public:
    game():score(0){}
    virtual void move();
    //virtual void printScore();
    virtual void start();
    virtual void Continue();

    game(int inputMachineDelay, int inputPreferredDelay){
        machineDelay = inputMachineDelay;
        preferredDelay = inputPreferredDelay;
        loopWaitCount = preferredDelay / machineDelay;
        if(  loopWaitCount < 1)
            loopWaitCount = 1;
        currentLoop = 0;
        
    }

    void incrementLoop(){
      ++currentLoop;
      currentLoop %= loopWaitCount;
    }

    void isTimeToUpdate(){
      return currentLoop  == 0 ;
    }
    
    void incrementScore(){
          ++score; 
    }
    void resetScore(){
          score = 0; 
    }
    int& getScore(){
          return score;
    }

  
};

class snakeClass : public game,public queue{
    int lastDirection, currentDirection;
    const int _up = 1, _down = -1, _right = 2, _left = -2;
    
    
    int interpretNewDirection(){
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

    int getValidDirection(){
        if(  currentDirection == -lastDirection)
            return lastDirection;
        return currentDirection;
    }

    



      void createFood(){
          while( 1 )
          {
              randomValue = random(0,1023);
              randomX = randomValue%8;
              randomValue = random(0,1023);
              randomY = randomValue%8;
              if( matrix[randomX][randomY] == 0 )
                break;
          }
          matrix[randomX][randomY] = 2;
          lc.setLed(0,randomX,randomY,true);
     }
    
public:
    //snake(int time

    void move(){
        currentDirection = interpretNewDirection();
        currentDirection = getValidDirection();
        lastDirection = currentDirection;
        coord newPosition = getTop(); 
        
        if( currentDirection == _right)
            newPosition.y = (newPosition.y+1)%8;
         else if( currentDirection == _left)
              newPosition.y = (newPosition.y > 0) ? newPosition.y - 1 : 7;
        else if( currentDirection == _up)
              newPosition.x = (newPosition.x < 7 ) ? newPosition.x + 1 : 0;
        else if( currentDirection == _down)
              newPosition.x = (newPosition.x > 0) ? newPosition.x - 1 : 7;
    
         coord oldestPosition = getBot();
         if(matrix[newPosition.x][newPosition.y] == 2){
            matrix[newPosition.x][newPosition.y] = 1;
            incrementScore();
            createFood();
        }else if( matrix[newPosition.x][newPosition.y] == 1 ){
            setGameOver();
            currentStateOfGame = _gameOver;
//reminder
//add game over            
        }else{
              pop();
        }
        
        push_back(newPosition);
    }

    void start(){
       push_back(coord(3,1) );
       push_back(coord(3,2) );
       push_back(coord(3,3) );
       lastDirection = _right;
       createFood();
    }

    void Continue(){
        move();
      
    }
    
};


snakeClass *snake;



 
void writeDebugRandomValuesXY(){
      Serial.print("[");
      Serial.print(randomX,DEC);
      Serial.print(",");
      Serial.print(randomY,DEC);
      Serial.print("]\n");

  
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
  lcd.print("Press anything");
  lcd.setCursor(0, 1);
  lcd.print("to start..");
  pinMode(pinCenter, INPUT  ); 
}


void updateScore(const int &snakeScore){
    lcd.setCursor(0, 1);
    lcd.print(snakeScore);
}

void updateInputValues(){
  centerButtonState = digitalRead(pinCenter);
  xValue = analogRead(pinX);  
  yValue = analogRead(pinY);
  
}

void waitForInput(){
    while( 1 == 1){
       updateInputValues();
       if( centerButtonState == LOW || isFull(xValue/128) || isFull(yValue/128) )
          break;
       delay(50);
      
    }
  
}

void loop()

{

    // lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  //lcd.scrollDisplayRight();
    //updateScore(snakeScore);
    
      updateInputValues();
    
      if( centerButtonState )
          gamePause = 1 - gamePause;
      if( gamePause)
          return;

      switch( currentStateOfGame ){
          case _justStarted:

          
              //Reminder
              //Add Game Menu
              waitForInput();
              lcd.clear();  
              currentStateOfGame = _snake;
              snake = new snakeClass;
              snake->start();
              break;
           case _snake:
              snake->Continue();
              break;
      }
      
     
  
      

  

     
    delay(defaultDelay);
    
}






