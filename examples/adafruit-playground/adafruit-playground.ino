#include <Adafruit_CircuitPlayground.h>
#include <FrameStream.h>
#include <Frameiterator.h>

#define OUTPUT__BAUD_RATE 9600
#define WINDOW_SIZE 30

FrameStream frm(Serial);

declarerunnerlist(GUI);
// Hierarchical runner list
beginrunnerlist();
fwdrunner(!g, GUIrunnerlist); // Reference to the second level (GUI)
callrunner(!!, InitGUI);
endrunnerlist();
// Second level
beginrunnerlist(GUI);
fwdrunner(EX, experimentCallback);
fwdrunner(RB, runCallback);
fwdrunner(IS, soundCallback);
fwdrunner(IL, lightCallback);
fwdrunner(IC, pickerCallback);
endrunnerlist();

int value = 255;
int experiment = 1;

int soundValue = 0;

int redValue = 255;
int greenValue = 0;
int blueValue = 0;

int cycle = 0;
int x = 0;
float light = 0;
bool running = false;

double window[WINDOW_SIZE];
int index = 0;
bool initialized = true;

void initWindow()
{
  for (int i = 0; i < WINDOW_SIZE; i++)
  {
    window[i] = 0.0;
  }
  initialized = false;
  index = 0;
}

void addToWindow(float value)
{
  if (!initialized)
  {
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
      window[i] = value;
    }
    initialized = true;
  }
  if (index >= WINDOW_SIZE)
  {
    index = 0;
  }
  window[index] = value;
  index++;
}

double getWindowAverage()
{
  double sum = 0;

  for (int i = 0; i < WINDOW_SIZE; i++)
  {
    sum += window[i];
  }

  return sum / WINDOW_SIZE;
}

void experimentCallback(char *str, size_t lenght)
{
  int newExperiment = atoi(str);

  if (newExperiment != experiment)
  {
    experiment = newExperiment;
    InitGUI();
  }
}

void runCallback(char *str, size_t lenght)
{
  int val = atoi(str);

  if (val == 0)
  {
    running = true;
    CircuitPlayground.redLED(HIGH);
  }
  else
  {
    running = false;
    CircuitPlayground.redLED(LOW);
  }
}

void pickerCallback(char *str, size_t length)
{
  char redString[3] = {str[1], str[2], '\0'};   // Extract red component
  char greenString[3] = {str[3], str[4], '\0'}; // Extract green component
  char blueString[3] = {str[5], str[6], '\0'};  // Extract blue component

  redValue = strtol(redString, NULL, 16);
  greenValue = strtol(greenString, NULL, 16);
  blueValue = strtol(blueString, NULL, 16);
}

void InitGUI()
{
  frm.print("!h");
  frm.print("<h1>Adafruit Circuit Laboratory</h1>");
  frm.print("<p>This Adafruit Circuit can execute 3 experiments, select one:</p>");
  frm.print("<select id='experiment'>");
  frm.print("<option value='1'>Temperature</option>");
  frm.print("<option value='2'>Make some Noise</option>");
  frm.print("<option value='3'>Lights on</option>");
  frm.print("</select>");
  frm.print("<input type='button' id='run' ");
  if (running)
  {
    frm.print("value='Stop' >");
  }
  else
  {
    frm.print("value='Start' >");
  }
  frm.end();

  frm.print("!j");
  frm.print(
      "const select = document.getElementById('experiment')\n"
      "select.onchange = () => {\n"
      "  sendframe('!gEX' + select.value)\n"
      "}\n"
      "const run = document.getElementById('run')\n"
      "run.onclick = () => {\n"
      "  if (run.value=='Start') {\n"
      "    run.value='Stop'\n"
      "    sendframe('!gRB0')\n"
      "  } else {\n"
      "    run.value='Start'\n"
      "    sendframe('!gRB1')\n"
      "  }\n"
      "}\n");
  frm.print("select.value = ");
  frm.print(experiment);
  frm.end();

  x = 0;
  cycle = 0;
  initWindow();

  running = false;

  switch (experiment)
  {
  case 1:
  {
    frm.print("!H<h2>Temperature</h2>");
    frm.end();

    frm.print("!SGgTv1000");
    frm.end();
    break;
  }

  case 2:
  {
    frm.print("!H<h2>Sound Experiment</h2>");
    frm.print("<p>Move the slider to control the sound level</p>");
    frm.end();

    frm.print("!SsISv0");
    frm.end();

    frm.print("!SGgSv50");
    frm.end();

    break;
  }

  case 3:
  {
    frm.print("!H<h2>Lights on</h2>");
    frm.print("<p>Move the slider to control the switch on the LEDs or change their color ...</p>");
    frm.end();

    frm.print("!SsILv255");
    frm.end();

    frm.print("!SpICv0");
    frm.end();

    frm.print("!SGgLv1000");
    frm.end();
    break;
  }
  }

  /*
    // Generation of the button
    // !S<E><ID><DATA>
    // Start      Element     ID         v=Value
    // indicator  s=Slider    [2 char]   initial value
    // !S         s           VA         v255
    frm.print("!SsVAv255");
    frm.end();
    frm.print("!SpVB");
    frm.end();

    frm.print("!j");
    frm.print("runner['vt']=function(x){\n"
      "document.getElementById('x').innerText=x\n"
      "}\n");
    frm.end();

    frm.print("!SGgrv50");
    frm.end();
    */
}

void soundCallback(char *str, size_t length)
{
  soundValue = atoi(str);
}

void soundExperiment()
{
  if (soundValue)
  {
    CircuitPlayground.playTone(soundValue * 10, 10, false);
  }

  addToWindow(CircuitPlayground.mic.soundPressureLevel(10));

  if (cycle == 0)
  {
    frm.print("!dgS1000,");
    frm.print(x++);
    frm.print(",");
    frm.print(getWindowAverage());
    frm.end();
    cycle = 10;
  }

  cycle--;
}

void temperatureExperiment()
{
  addToWindow(CircuitPlayground.temperature());

  if (cycle == 0)
  {
    frm.print("!dgT1000,");
    frm.print(x++);
    frm.print(",");
    frm.print(getWindowAverage());
    frm.end();
    cycle = 1000;
  }

  cycle--;
}

void lightCallback(char *str, size_t length)
{
  // "Transformation to the time domain (0-25 ms)"
  value = atoi(str);
}

void lightExperiment()
{
  for (int i = 0; i < 10; i++)
  {
    if (value > i * 25)
    {
      CircuitPlayground.setPixelColor(i, redValue + i, greenValue + 2 * i, blueValue + 3 * i);
    }
    else
    {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }

  addToWindow(CircuitPlayground.lightSensor());
  if (cycle == 0)
  {
    frm.print("!dgL1000,");
    frm.print(x++);
    frm.print(",");
    frm.print(getWindowAverage());
    frm.end();
    cycle = 100;
  }

  cycle--;
}

void setup()
{
  Serial.begin(OUTPUT__BAUD_RATE);

  // request reset of gui
  frm.print("!!");
  frm.end();

  delay(500);

  CircuitPlayground.begin();
}

void loop()
{
  frm.run();

  if (!running)
  {
    return;
  }

  switch (experiment)
  {
  case 1:
  {
    temperatureExperiment();
    break;
  }
  case 2:
  {
    soundExperiment();
    break;
  }
  case 3:
  {
    lightExperiment();
    break;
  }
  }
}
