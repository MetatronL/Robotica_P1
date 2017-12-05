/*
    DEC 2017
    Snake
    by Bogdan-Gabriel Paun(233)
    Laborant : Ivona
*/





#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Arduino.h>

const int defaultDelay = 25;

const int pinX = A3 , pinY = A4, pinCenter = 0;
LedControl lc(10,13,11,1);

const int rs = 6, en = 7, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int _justStarted = 0 , _gameOver = -1 , _snake = 201;
const int _up = 1, _down = -1, _right = 2, _left = -2;


int gamePause = 0 ,currentStateOfGame;
int randomValue,randomX,randomY;
int centerButtonState = 0, xValue = 0, yValue = 0;


//int queue[130] , queueTop  , queueBottom;
int matrix[8][8];
// 0 = empty
// 1 = snake
// 2 = food




int isFull(int);
//return  1 if the joystick is up or rigth
//return -1 if the joystick is down or left
//return 0 if center

void setGameOver();

void updateScore( const int& );

void updateInputValues(void);
//analog reads from joystick's pins

void waitForInput(void);
//waits for a joystick movement

int waitForInputDifficulty(void);
// waits for input
//up = easy
//right = medium
//down = hard


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
  // queue's memory zone
  
  int bot,top ; 
  // queue position for first and last element

  int botTimes, topTimes;
  // quantum of times the queue has been completed to avoid errors
    
  static const int _emptyQueue = -1;
  
  int queue::advanceTop(void);
  //returns the next position in queue for writing
  
  int queue::advanceBot(void);
  //moves over the last element

 public:

 
  queue() {bot=0; top=0; botTimes = 0; topTimes = 0;}
  
  void queue::push_back(int x,int y);
  //push_back in queue for <int,int>
  
  void queue::push_back(coord aux);
  //push_back in queue for coord = <int,int>

  coord queue::pop(void);
  //returns the oldest element , and moves the left cursor over him in queue
  
  coord queue::getBot(void);
  //returns the oldest element

  coord queue::getTop(void);
  //return the newest element

  void queue::turnOnAllStartup(void);
  //sets up the initial snake

};










class game{

    int score;
    int machineDelay , loopWaitCount ;
    int preferredDelay; int currentLoop;
    

public:
    
    game():score(0){}
    virtual void move();
    //virtual void printScore();
    virtual void start();
    virtual void continueGame();
    virtual void waitForDifficulty();

    game(int inputMachineDelay, int inputPreferredDelay){
        machineDelay = inputMachineDelay;
        preferredDelay = inputPreferredDelay;
        loopWaitCount = preferredDelay / machineDelay;
        if(  loopWaitCount < 1)
            loopWaitCount = 1;
        currentLoop = 0;
        score = 0;
    }
    //calculate how many times it must wait to match the desired delay betwen updates

    //I use a count to know when its the time to update
    void incrementLoop(){
      ++currentLoop;
      currentLoop %= loopWaitCount;
    }

    int isTimeToUpdate(){
      return (currentLoop  == 0) ;
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

    //changes the preffered delay 
    void setSpeed( int inputPreferredDelay ){
        preferredDelay = inputPreferredDelay;
        loopWaitCount = preferredDelay / machineDelay;
        if(  loopWaitCount < 1)
            loopWaitCount = 1;
        currentLoop = 0;
        score = 0;
    }

  
};

class snakeClass : public game,public queue{
    
    const int _up = 1, _down = -1, _right = 2, _left = -2;
    const int difficultyEasyDelay = 350, difficultyMediumDelay = 250, difficultyHardDelay = 200;

    int lastDirection, currentDirection, foundWhileWaitingDirection;
    int useBorder ;
    // 0 = no border
    // 1 = border 
    // if the snake touches the border the game is over

    
    int interpretNewDirection(){
      
        //xValue = 0 , 1023
        xValue /= 128;
        yValue /= 128;
        //xValue = 0 , 8
        
        if( isFull(xValue) &&  !isFull(yValue)  ){
              if( xValue > 5)
                  return _up;
               else 
                  return _down;
        }
        if( !isFull(xValue) &&  isFull(yValue)  ){
              if( yValue > 5)
                  return _left;
               else 
                  return _right;
        }
        return lastDirection;

      
    }
    
    //checks if the input direction is wrong
    //if its wrong it keeps the last direction
    int getValidDirection(){
        if(  currentDirection == -lastDirection)
            return lastDirection;
        return currentDirection;
    }

    


      //creates food for snake
      //it tries to create a random position untill it finds a valid position( empty cell)
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


     //interpret the direction from the input
     //reminder the snake only moves after a given number of steps
     //I use a the variable foundWhileWaitingDirection to remember the last valid input found while waiting
     
     void updateDirection(){
          currentDirection = interpretNewDirection();
          currentDirection = getValidDirection();

          //IF the new direction is valid I save the direction for next update
          if( lastDirection !=  currentDirection  )
              foundWhileWaitingDirection = currentDirection;
          //if the input is invalid I use the last found valid direction(if it exists)
          else if( foundWhileWaitingDirection != 0 )
              currentDirection = foundWhileWaitingDirection ;  
     }

     
     void updateDifficulty(int choice){
        if( choice == 0)
            return;
        
        else if( choice == _up){
            // Ease 
            // Defualt Setting
            useBorder = 0;
        }
        else if( choice == _right){
            // Medium
            setSpeed( difficultyMediumDelay );
            useBorder = 1;
        }
        else if( choice == _down){
            // Hard
            setSpeed( difficultyHardDelay );
            useBorder = 1;
        }
          
     }
    
public:
     
    
    snakeClass(int inputMachineDelay) : game(inputMachineDelay, 400) {   }

    void move(){

        //reset the auxiliar direction used the memorize the input found while waiting
        foundWhileWaitingDirection = 0;
        //sets the new direction
        lastDirection = currentDirection;
        coord newPosition = getTop(); 

        //calculate the new position of the head of the snake
        if( currentDirection == _right)
            newPosition.y = newPosition.y + 1;
         else if( currentDirection == _left)
              newPosition.y = newPosition.y - 1;
        else if( currentDirection == _up)
              newPosition.x = newPosition.x  + 1 ;
        else if( currentDirection == _down)
              newPosition.x = newPosition.x  - 1 ;
        //if border is set 
        //verify if the snake touched the border
        if(useBorder && (newPosition.x == -1 ||  newPosition.x == 8 ||  newPosition.y == -1 || newPosition.y == 8 ) ){
            setGameOver();
            currentStateOfGame = _gameOver;
            return;
        }

        //correct the new position
        if(  newPosition.x == -1)
             newPosition.x = 7;
        if(  newPosition.y == -1)
             newPosition.y = 7;
        newPosition.x %= 8;
        newPosition.y %= 8;

         //if the new position is already used by snake -> game over
         //if the new position is food -> generate new food , increment score
         //if the new posiiton is empty -> start the led on the new position , deletes the oldest block of snake
         
         coord oldestPosition = getBot();
         if(matrix[newPosition.x][newPosition.y] == 2){
            matrix[newPosition.x][newPosition.y] = 1;
            incrementScore();
            createFood();
        }else if( matrix[newPosition.x][newPosition.y] == 1 ){
            setGameOver();
            currentStateOfGame = _gameOver;
            return;
          
        }else{
              pop();
        }
        
        push_back(newPosition);
        updateScore(getScore());
    }

    //initial snake
    void start(){
       push_back(coord(3,1) );
       push_back(coord(3,2) );
       push_back(coord(3,3) );
       lastDirection = _right;
       createFood();
    }

    //waits for difficulty
    //default is easy
    void waitForDifficulty(){
        int choice = 0;
        lcd.clear();
         
        lcd.setCursor(0, 0);
        lcd.print("Please select");
        lcd.setCursor(0, 1);
        lcd.print("your difficulty");
        choice = waitForInputDifficulty();
        if( choice != 0 ){
            updateDifficulty(choice);
            return ;
        }
        
        //Print details about Easy difficulty
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Easy(UP) ");
        lcd.setCursor(0, 1);
        lcd.print("No borders");
        choice = waitForInputDifficulty();
        if( choice != 0 ){
            updateDifficulty(choice);
            return ;
        }

        
        //Print details about Medium difficulty
        lcd.clear();
        lcd.setCursor( 0, 0 );
        lcd.print( "Medium(Right) " );
        lcd.setCursor( 0, 1 );
        lcd.print("With borders");
        choice = waitForInputDifficulty();
        if( choice != 0 ){
            updateDifficulty( choice );
            return ;
        }

        //Print details about Hard difficulty
        lcd.clear();
        lcd.setCursor( 0, 0 );
        lcd.print( "Hard(DOWN) " );
        lcd.setCursor(0, 1);
        lcd.print( "With borders" );
        //delay(2000);
        choice = waitForInputDifficulty();
        if( choice != 0 ){
            updateDifficulty( choice );
            return ;
        }
    }
    
    void continueGame(){
        // i update the snake only at given step calculated from the delays
        incrementLoop();

        //reads input + calculate the new direction
        updateDirection();
        
        if( isTimeToUpdate()  )
           move();
    }

    
};








//pointer to my game
game *currentGame;


void setup(){
  
  pinMode( pinCenter, INPUT_PULLUP  ); 
  
  lc.shutdown( 0,false );
  lc.setIntensity( 0,10 );
  
  currentStateOfGame = _justStarted;
  
  randomSeed( analogRead( A0 ) );   
  Serial.begin( 9600 ); 

  lcd.begin( 16, 2 );
  lcd.print( "Press anything" );
  lcd.setCursor( 0, 1 );
  lcd.print( "to start.." );
  
}


void loop(){

      updateInputValues();
    
      if( centerButtonState == 0){
          gamePause = 1 - gamePause;
          delay(300);
      }
          
      if( gamePause == 0 )
        switch( currentStateOfGame ){
            case _justStarted:
                waitForInput();
                lcd.clear();
                
                currentStateOfGame = _snake;
                currentGame = new snakeClass( defaultDelay );
                delay(500);
                currentGame->waitForDifficulty();
                currentGame->start();
                break;
             case _snake:
                currentGame->continueGame();
                break;
        }
    
    delay( defaultDelay );
    
}



int isFull(int input){
        if( input > 5 )
            return 1;
            
        if( input < 2)
            return -1;

        return 0;
    }

void setGameOver(){
  lc.clearDisplay(0);
  lcd.setCursor(0, 0);
  lcd.print("Game Over!"); 
  for(int i=0; i <= 7; ++i){
      lc.setLed(0,i,i,true);
      lc.setLed(0,i,7-i,true);
  }
  
}


void updateScore( const int &myScore ){
    lcd.clear();  
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.setCursor(7, 1);
    lcd.print(myScore);
}

void updateInputValues(void){
  centerButtonState = digitalRead( pinCenter );
  Serial.println(centerButtonState,DEC);
  xValue = analogRead( pinX );  
  yValue = analogRead( pinY );
  
}

void waitForInput(void){
    while( 1 == 1){
       updateInputValues();
       if( centerButtonState == LOW || isFull( xValue / 128 ) || isFull( yValue / 128 ) )
          break;
       delay( 50 );
      
    }
  
}

int waitForInputDifficulty(void){
    int count = 0 ;
    while( ++count < 20){
       updateInputValues();
       if(  isFull( xValue / 128) ==  1 && isFull( yValue / 128 ) == 0  )
            return _down;
       if(  isFull( xValue / 128) == -1 && isFull( yValue / 128 ) == 0  )
            return _up;
       if(  isFull( xValue / 128) == 0  && isFull( yValue / 128 ) == 1  )
            return _right;
       if(  isFull( xValue / 128) == 0  && isFull( yValue / 128 ) == -1 )
            return _left;
       delay(50);
    }
    return 0;
  
}



// Definiton zone for class queue 

//Public:
int queue::advanceTop(void){
        int old_top = top++;
        if(top == capacity){
          ++topTimes;
          top = 0; 
        }
        return old_top;
    }

    int queue::advanceBot(void){
        int old_bot = bot++;
        if(bot == capacity){
          ++botTimes;
          bot = 0; 
        }
        return old_bot;
    }
//Private

void queue::push_back(int x,int y){
      coord temp(x,y);
      holder[advanceTop()] = temp;
  }
  
  void queue::push_back(coord aux){
      holder[advanceTop()] = aux;
       lc.setLed(0,aux.x,aux.y,true);    
       matrix[aux.x][aux.y] =1 ; 
  }

  coord queue::pop(void){
    if( bot > top && botTimes >= topTimes )
      Serial.print("Error bot > top !\n");

     lc.setLed(0,holder[bot].x,holder[bot].y,false);    
       matrix[holder[bot].x][holder[bot].y] = 0 ; 
    return holder[advanceBot()];
    
  }
  
  coord queue::getBot(void){
    if( bot > top && botTimes >= topTimes )
      Serial.print("Error bot > top !\n");
    return holder[bot];
    
  }

  coord queue::getTop(void){
      if(  top == 0)
        return holder[capacity - 1];
      return holder[top-1];
    
  }

  void queue::turnOnAllStartup(void){
      //only use if you are sure the queue has not started from 0 due to usage ( circular queue )
      int index = bot;
      for(; index < top; ++index)
         lc.setLed(0,holder[index].x,holder[index].y,true);    
  }

// End of definiton zone for queue class





