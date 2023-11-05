#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//used for setting backgroud colours.
#define GREEN 0x2
#define WHITE 0x7 

//global variables.
//this is the initial state of the FSM in the main loop() function.
uint8_t state = 1;
//initial value of N (sequence length).
int N = 4;
//initial value of M (no. of possible keys in sequences).
int M = 2;
//initial value of T (time limit to enter next key).
float T = 5.0;

void setup() {
  //set up code
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  //allows in-built random() function to generate random values.
  randomSeed(analogRead(0));
  //scrolls once when arduino is started up
  scrollMessage("Use up and down to scroll, then press select.");
} 

//This menu function runs after every state in the FSM and at the start.
void menu(){
  lcd.clear();
  lcd.setCursor(0,0);  
  //array of all menu options
  String options[] = {"1: Play practice mode.", "2: Play story mode.", 
                      "3: Change sequence length (N).", "4: Change no. of keys (M).",
                      "5: Change time limit (T)."};
  //current option set to 1- represents "1: Play practice mode."
  int current_option = 1;
  bool not_selected = true;
  //loop runs, displaying current menu option until one is selected.
  while(not_selected){  
    //the initial state of buttons is stored in int_btn
    uint8_t int_btn = lcd.readButtons();
    //display current menu option
    scrollMessage(options[current_option - 1]);
    //uses function pressedButton() to wait for and get user's input.
    char user_input = pressedButton(int_btn);
    //for button press select, uses the value of current_option to change to correct state,
    //each menu option has its own state.
    if(user_input == 's'){
      if(current_option == 1){
        state = 2;
        break;
      }else if(current_option == 2){
        state = 5;
        break;
      }else if(current_option == 3){
        state = 3;
        break;
      }else if(current_option == 4){
        state = 4;
        break;
      }else if(current_option == 5){
        state = 6;
        break;
      }
    //if up or down are pressed, the menu_option changes and the new menu option
    //is displayed.
    }else if(user_input == 'u'){
      if(current_option == 1){
        current_option = 5;  
      }else{
        current_option -= 1;
      }
    }else if(user_input == 'd'){
      if(current_option == 5){
        current_option = 1;  
      }else{
        current_option += 1;
      }
    }
  }
}

//Function for story mode runs the singleStage() function with chnaging N,M,D and T values 
//until the user enters a wrong sequence and loses.
void storyMode(){
  bool correct = true;
  //counter keeps track of which stage is being run.
  int counter = 1;
  //intial values for stage 1.
  int n = 4;
  int m = 2;
  int d = 3;
  int t = 5.00;
  //loop runs while the user enters correct sequences
  while(correct == true){
    //singleStage() returns true if the stage is passed and false if not.
    correct = singleStage(n, m, d, t);
    //M increases by 1 in stages 3 and 5.
    if(counter == 2 or counter == 4){
        m++;
    //D is reduced by 1 second in stages 4 and 6
    }else if(counter == 3 || counter == 5){
        d--;
    }
    //N increases by 2 and T is reduced by 1/4 of a second every stage.
    n += 2;
    t = t - 0.25;
    //The counter isn't needed after stage 6.
    if(counter < 6){
      counter++;
    }
  }
}

//This fucntion is called once in practice mode straight from the FSM and is called multiple times
//from the storyMode() fucntion to run a stage.
//n is N, m is M, d is D and t is T.
bool singleStage(int n, int m, int d, float t){
  char key;
  char seq[n];

  //Generates the sequence
  for(int i = 0; i < n; i++){
    //generateRand generates a random character from 'r', 'l', 'd' and 'u' for each button depending on the
    //value of m. 
    key = generateRand(m);
    seq[i] = key;
  }

  Serial.println("--New pattern--");
  char user_input;
  //for loop displays sequence keys one by one both on LCD display for user and 
  //in Serial monitor for testing
  for(int j = 0; j < n; j++){
    String output;
    if(seq[j] == 'u'){
      output = "UP";  
    }else if(seq[j] == 'd'){
      output = "DOWN";  
    }else if(seq[j] == 'l'){
      output = "LEFT";  
    }else if(seq[j] == 'r'){
      output = "RIGHT";  
    }

    Serial.println(output);
    lcd.setCursor(6, 0);
    lcd.print(output);
    //each key is displayed for d seconds depending on the value of d
    delay(d*1000);
    lcd.clear();
    delay(500);
  }

  //outer for loop runs for each key in the sequence, displaying and running the timer, 
  //and checking the button pressed is correct or wrong. 
  bool correct = true;
  char input;
  uint8_t int_btn;
  for(int k = 0; k < n; k++){
    float time_left = t;
    input = 'x';
    //while loop displays timer and checks for user input.
    //it re-runs every 0.1 seconds as timer goes down,
    //each time calling pressedButtonT() to check if any buttons have been pressed 
    //and if so which one.
    while(input == 'x' && time_left > 0){
      int_btn = lcd.readButtons();
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("time left: "); 
      lcd.setCursor(12, 1); 
      lcd.print(time_left); 
      //pressedButtonT() is similar but works differently to pressedButton(), as rather than waiting for
      //an input it runs for an amount of time (second parameter) and returns either the character for the button 
      //pressed or 'x' for no button presses.
      input = pressedButtonT(int_btn, 10);
      time_left = time_left - 0.1;
    }
    //if timer runs out and input remains 'x' indicating no button presses, they fail this stage.
    if(input == 'x'){
      correct = false;
      break;
    }
    //or if the input was given in time but wasn't correct they again fail this stage.
    if(input != seq[k]){
      correct = false;  
    }
  }

  //if correct stayed true as user enetered the sequence correctly and in time,
  //the backlight flashes green.
  if(correct == true){
    lcd.clear();
    lcd.setBacklight(GREEN);
    delay(200);
    lcd.setBacklight(WHITE);
  //if the user got the sequence wrong or wasn't in time, the game over display runs.
  }else{
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("OOPs  :("); 
    lcd.setCursor(4, 1); 
    lcd.print("Game over!"); 
    delay(1000);
    lcd.clear();
  }
  //return the value of correct from the function. Used in storyMode() to check if stage passed. 
  return correct;
}

//this fucntion allows user to change the timer T
void changeT(){
  bool not_selected = true;
  //loop runs, allowing user to change the value of T using up and down until they press select.
  while(not_selected){  
    lcd.clear();
    uint8_t int_btn = lcd.readButtons();
    lcd.setCursor(4, 0);
    lcd.print("T: ");
    lcd.setCursor(7, 0);
    lcd.print(T);
    char user_input = pressedButton(int_btn);
    //leaves loop when select is pressed
    if(user_input == 's'){
      break;
    //doesn't allow user to go higher than T=5.00
    }else if(user_input == 'u' && T <= 4.9){
      T += 0.1;
    //doesn't allow user to go lower than T=1.50
    }else if(user_input == 'd' && T >= 1.6){
      T -= 0.1;
    }
  } 
  
}

//allows user to change N, works in the same way as changeT
void changeN(){
  bool not_selected = true;
  while(not_selected){  
    lcd.clear();
    uint8_t int_btn = lcd.readButtons();
    lcd.setCursor(7, 0);
    lcd.print("N: ");
    lcd.setCursor(10, 0);
    lcd.print(N);
    char user_input = pressedButton(int_btn);
    if(user_input == 's'){
      break;
    //upper limit is N=12
    }else if(user_input == 'u' && N < 12){
      N += 1;
    //lower limit is N=4
    }else if(user_input == 'd' && N > 4){
      N -= 1;
    }
  } 
}

//allows user to change M, again works in the same way as changeN and changeT
void changeM(){
  bool not_selected = true;
  while(not_selected){  
    lcd.clear();
    uint8_t int_btn = lcd.readButtons();
    lcd.setCursor(7, 0);
    lcd.print("M: ");
    lcd.setCursor(10, 0);
    lcd.print(M);
    char user_input = pressedButton(int_btn);
    if(user_input == 's'){
      break;
    //upper limit is M=4
    }else if(user_input == 'u' && M < 4){
      M += 1;
    //upper limit is M=2
    }else if(user_input == 'd' && M > 2){
      M -= 1;
    }
  }
}

//generates a random character to represent a button from up, down, left and right
//parameter m is M, so that for m=2 it only returns characters for left and right and
//for m=3 it only returns characters for left, right and up.
char generateRand(int m){
  char key = 'x';
  long randNumber;
  //loop runs until a charater is generated that is allowed for the m that is passed to the function
  while(key == 'x'){
    //generates random number between 1 and 4
    randNumber = random(1, 5);
    //there's a case for each number, so depending on the value of m, it is used and if it can't be used,
    //the loop runs again.
    switch(randNumber){
      case 1:
        if(m == 3 || m == 4){
          key = 'u';
        }
        break;
      case 2:
        if(m == 4){
          key = 'd';
        }
        break;
      case 3:
        key = 'l';
        break;
      case 4:
        key = 'r';
        break;
    }
    delay(3); 
  }
  //returns the character.
  return key; 
}

//returns character of the button that the user has pressed during a time limit, if no button
//pressed in that time, returns 'x'. initial_buttons is the initial state of the buttons used to 
//tell which button states have changed and m_secs is time limit.
char pressedButtonT(uint8_t initial_buttons, int m_secs){
  //int_time used to calculate time passed since fucntion started running
  unsigned long int_time = millis();
  //loop runs, constantly waiting for a button press until time runs out.
  while(true){    
    uint8_t buttons_state = lcd.readButtons();
    uint8_t button_changes = initial_buttons & (~buttons_state);
    //depending on which button is pressed, different character is returned
    if ((button_changes & BUTTON_UP)) {
      return('u');
    }else if ((button_changes & BUTTON_DOWN)) {
      return('d');
    }else if ((button_changes & BUTTON_RIGHT)) {
      return('r');
    }else if ((button_changes & BUTTON_LEFT)) {
      return('l');
    }else if ((button_changes & BUTTON_SELECT)) {
      return('s');
    //if no buttons are pressed and the time runs out, breaks out of loop and returns 'x' to indicate
    //no button presses.
    }else if(millis() - int_time >= m_secs){
      break;  
    }
    initial_buttons = buttons_state;
  }
  return('x');
}

//works similarly as pressedButtonT(), but without a timer, it simply runs until a button is pressed, and then returns a character 
//for that button.
char pressedButton(uint8_t initial_buttons){
  //loop runs until a button is pressed and a character is returned
  while(true){
    uint8_t buttons_state = lcd.readButtons();
    uint8_t button_changes = initial_buttons & (~buttons_state);
    if ((button_changes & BUTTON_UP)) {
      return('u');
    }else if ((button_changes & BUTTON_DOWN)) {
      return('d');
    }else if ((button_changes & BUTTON_RIGHT)) {
      return('r');
    }else if ((button_changes & BUTTON_LEFT)) {
      return('l');
    }else if ((button_changes & BUTTON_SELECT)) {
      return('s');
    }
    initial_buttons = buttons_state;
  }
}

//scrolls any message msg that is passed to it across the screen
void scrollMessage(String msg){
  //adds preceeding spaces to message 
  msg = "               " + msg;
  int pos = 0;
  //loop prints message, each time removing one character from the left, uses a delay of 150ms to
  //make scrolling motion
  for (int i = msg.length(); i > 0; i--) {
    lcd.clear();
    lcd.setCursor(0,0);
    String scrolledMessage = msg.substring(pos, pos+16);
    lcd.print(scrolledMessage);
    delay(150);
    pos += 1;
  } 
  lcd.clear();
}

//this is the Finite-State-Machine, it is set to state 1 (menu) at the start and always returns to this state after all other states.
void loop(){
  switch(state){
    //menu
    case 1:
      menu();
      break;
    //practice mode
    case 2:
      //singleStage() is called with the global variables N,M and T as the fucntions changeN(), changeM() 
      //and changeT() can change them. Delay is set to 1 second
      singleStage(N, M, 1, T);
      state = 1; 
      break;
    //change N
    case 3:
      changeN();
      state = 1; 
      break;
    //change M
    case 4:
      changeM();
      state = 1; 
      break;
    //Story mode
    case 5:
      storyMode();
      state = 1; 
      break;
    //Change T
    case 6:
      changeT();
      state = 1; 
      break;
    default:
      break;
  }
}
