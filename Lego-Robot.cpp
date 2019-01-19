#include "PC_FileIO.c"

//constants
const int ROW_GRID = 26; //number of rows in grid
const int COL_GRID = 26; //number of columns in grid
const float GEAR_ROW = 0.44; //gear radius for row
const float GEAR_COLUMN = 1.75; //gear radius for column 
const int SPEED = 25;
const tMotor MARKER_MOTOR = motorB; //motor controlling marker
//motor controlling bar moving sensor and marker
const tMotor FRAME_MOTOR = motorA; 
//motor controlling sensor and marker along bar
const tMotor BAR_MOTOR = motorD; 
// distance to move to next value to scan or draw 0.145
const float DIST_INCREMENT = 0.25; 
//distance to lift and drop marker to paper
const float MARKER_INCREMENT = 0.25; 
const float MARKER_RAD = 1.545;

int gridValues[ROW_GRID][COL_GRID];

//function prototypes
void createPreset (TFileHandle & fout);
void moveDist(tMotor motorMove, float distance, int back_forward, 
float gearSize);
void scanRow(tMotor rowMotor, int direction, int rowIndex);
void initializeArray();
int scanTotal();
bool fileInput(TFileHandle & fin);
float drawAll();
void drawRow(tMotor rowMotor, int direction, int rowIndex);

task main() {

  motor[motorD]=0;
  motor[motorA]=0;

  //variables
  float timeScan = 0;
  float timeDraw = 0;

  //initialize sensors
  SensorType[S1] = sensorEV3_Color;
  wait1Msec(50);
  SensorMode[S1] = modeEV3Color_Reflected;
  wait1Msec(50);

  //file opening
  TFileHandle creature;
  bool creatureCheck = openReadPC(creature, "creature.txt");
  TFileHandle pumpkin;
  bool pumpkinCheck = openReadPC(pumpkin, "pumpkin.txt");
  TFileHandle hi;
  bool hiCheck = openReadPC(hi, "hi.txt");
  TFileHandle tool;
  bool toolCheck = openReadPC(tool, "tool.txt");
  TFileHandle newPreset;
  bool newPresetCheck = openWritePC(newPreset, "newPreset.txt");

  // Ensures file works
  if(!(toolCheck && hiCheck && pumpkinCheck && creatureCheck &&      
     newPresetCheck))
     displayString(0, "One or more file(s) could not be opened.");

  initializeArray();

  // Wait for user input
  do {

     displayString(1, "Paint.Goose");
     displayString(2, "Press the left button to scan");
     displayString(4, "Press the right button to");
     displayString(5, "print a preset"); 

     while(!getButtonPress(buttonAny))
     {}

     if(getButtonPress(buttonLeft)){ //scanning
        while(getButtonPress(buttonLeft))
           {}
        timeScan = scanTotal();
        timeScan /= 60000.0;

        createPreset(newPreset);

        displayTextLine(1, "It took %0.2f minutes to scan.", timeScan);
        displayString(2, "It will take about %.2f minutes to print", 		  	                timeScan); 
        displayString(3, "Please change the sheet");
        displayString(4, "Once the paper has been");
        displayString(5, "changed, press any button");
        displayString(6, "to start printing");

        while(!getButtonPress(buttonAny))
        {}

        while(getButtonPress(buttonAny))
        {}
     }

     else if(getButtonPress(buttonRight)) { //printing a preset

        while(getButtonPress(buttonRight))
           {}

        displayTextLine(0, "What image would you like?");
        displayString(2, "2. Spoopy pumpkin -- Down");
        displayString(3, "   button");
        displayString(4, "3. Hi -- Up button");
        displayString(5, "4. Creature -- Right button");
        displayString(6, "5. The Tool -- Left button");
   
        while (!(getButtonPress(buttonDown)||getButtonPress(buttonUp)||
               getButtonPress(buttonLeft)||getButtonPress(buttonRight)))
        {}

        if (getButtonPress(buttonDown))
           fileInput(pumpkin);
        else if (getButtonPress(buttonUp))
           fileInput(hi);
        else if (getButtonPress(buttonRight))
           fileInput(creature);
        else if (getButtonPress(buttonLeft))
           fileInput(tool);
     }

     timeDraw = drawAll();
     timeDraw /= 60000;
     eraseDisplay();
     displayTextLine(0, "It took %.2f minutes", timeDraw);
     displayTextLine(1, "to print this image.");

     displayString(1, "Press centre button to");
     displayString(2, "do it again.");
     displayString(3, "Press any other button");
     displayString(4, "to exit.");

     while(!getButtonPress(buttonAny))
     {}

  } while(getButtonPress(buttonEnter));

  eraseDisplay();
  displayTextLine(1, "Thank you for using");
  displayTextLine(2, "Paint.Goose.");
  wait1Msec(2000);

  closeFilePC(creature);
  closeFilePC(hi);
  closeFilePC(pumpkin);
  closeFilePC(tool);
  closeFilePC(newPreset);

}

void moveDist(tMotor motorMove, float distance, int back_forward, float gearSize) { 

  nMotorEncoder[motorMove] = 0;

  motor[motorMove] = SPEED * back_forward;

  if(back_forward==1)
  {
     while(((nMotorEncoder[motorMove] * PI * gearSize) / 180) < distance * back_forward) 
        {}
  }
  else
  {
     while(((nMotorEncoder[motorMove] * PI * gearSize) / 180) > distance *  
           back_forward) 
        {}
  }

  motor[motorMove] = 0; //turn motors off
}

void scanRow(tMotor rowMotor, int direction, int rowIndex) {

  if(direction == 1) {

     int col = 0;

     while(col < COL_GRID) {

        if(SensorValue[S1]<50)
        {
           gridValues[rowIndex][col] = 1;
        }
        wait1Msec(50);

        col++;

        moveDist(rowMotor, DIST_INCREMENT, direction, GEAR_ROW);
     }
  }


  else {

     int col = COL_GRID - 1;

     while(col >= 0) {

        if(SensorValue(S1)<50)
           gridValues[rowIndex][col] = 1;
        wait1Msec(50);

        col--;

        moveDist(rowMotor, DIST_INCREMENT, direction, GEAR_ROW);
     }
  }
}

void initializeArray()
{
  for(int row = 0; row < ROW_GRID; row++)
  {
     for(int col = 0;col< COL_GRID; col++)
     {
        gridValues[row][col] = 0;
     }
  }
}

int scanTotal()
{
  clearTimer(T1);
  int direction = 1;
  int scanTime = 0;
  for (int row = 0;row < ROW_GRID; row++)
  {
     scanRow(BAR_MOTOR, direction, row);
     moveDist(FRAME_MOTOR, DIST_INCREMENT, 1, GEAR_COLUMN);
     direction *= -1;
  }
  scanTime = time1(T1);
  moveDist(FRAME_MOTOR, DIST_INCREMENT * ROW_GRID, -1, GEAR_COLUMN);
  return scanTime;
}

bool fileInput(TFileHandle & fin)
{
  int currValue=0;

  for(int row=0;row<ROW_GRID;row++)
  {
     for(int col=0;col<COL_GRID;col++)
     {
        readIntPC(fin, currValue);
        gridValues[row][col]= currValue;
     }
  }
  return true;
}

void createPreset(TFileHandle & fout)
{
  for(int row=0;row<ROW_GRID;row++)
  {
     for(int col=0;col<COL_GRID;col++)
     {
        writeLongPC(fout, gridValues[row][col]);
        writeCharPC(fout, ' ');
     }
     writeEndlPC(fout);
  }
}

void drawRow(tMotor rowMotor, int direction, int rowIndex)
{
  if(direction == 1) {

     int colIndex = 0;

     while(colIndex < ROW_GRID) {

        if(gridValues[rowIndex][colIndex] == 1) //not white
        {
           moveDist(MARKER_MOTOR, MARKER_INCREMENT, 1, MARKER_RAD);
           wait1Msec(50);

           moveDist(MARKER_MOTOR, MARKER_INCREMENT, -1, MARKER_RAD);
        }
        moveDist(rowMotor, DIST_INCREMENT, direction, GEAR_ROW);
        colIndex++;
     }
  }
  else {

     int colIndex = ROW_GRID - 1;

     while(colIndex >= 0) {

        if(gridValues[rowIndex][colIndex] == 1){ //not white

           moveDist(MARKER_MOTOR, MARKER_INCREMENT, 1, MARKER_RAD);
           wait1Msec(50);

           moveDist(MARKER_MOTOR, MARKER_INCREMENT, -1, MARKER_RAD);
        }
        colIndex--;

        moveDist(rowMotor, DIST_INCREMENT, direction, GEAR_ROW);
     }
  }
}

float drawAll()
{
  clearTimer(T1);
  int direction = 1;
  int drawTime = 0;
  for (int row=0;row<ROW_GRID;row++)
  {
     drawRow(BAR_MOTOR, direction, row);
     moveDist(FRAME_MOTOR, DIST_INCREMENT, 1, GEAR_COLUMN);
     direction *=-1;
  }
  drawTime=time1(T1);
  moveDist(FRAME_MOTOR, DIST_INCREMENT * ROW_GRID, -1, GEAR_COLUMN);
  return drawTime;
}
