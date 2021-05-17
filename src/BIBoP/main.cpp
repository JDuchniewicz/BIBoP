#include <Arduino.h>
#include <Wire.h>
#include <Scheduler.h>

#include "Collector.h"
#include "NetworkManager.h"
#include "Display.h"
// because of how arduino makefile builds dependencies, this finds proper files in the libs/ dir - they are otherwise unused here
#include "Defines.h"
#include "Secrets.h"
#include <MAX30105.h>
#include <WiFiNINA.h>
// in order to make WiFiNINA build, I had to remove one UDP.h in cores/arduino and move the IPAdress raw_addr to public
#include <SPI.h> // required to build wifinina
#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h>
#include <ACROBOTIC_SSD1306.h>

Collector collector;
Display display;
WiFiClient lambda;
BearSSLClient sslLambda(lambda);
NetworkManager networkManager(sslLambda);

Batch batch; // for now this is a static container (could be a ring of data)
//long lastTime = 0;

//bool readyToDim = true;

volatile uint32_t wakeUpMillis = 0;
volatile uint32_t buttonPressMillis = 0;
volatile bool activelyUsing = false;
bool triggerTimeout = false;
constexpr auto ACTIVITY_INTERVAL = 1000; //1 s for now
constexpr auto WAKEUP_INTERVAL = 5000; //5 s for now

// C, eh?
void displayLoop();
void buttonIrq();
void dataTask();
void wifiTask();
void oledTask();

void printLastData()
{
    Serial.print(" R: ");
    Serial.print(batch.ppg_red);
    Serial.print(" IR: ");
    Serial.print(batch.ppg_ir);
    Serial.println();
}

// move to a class
void usleep_init()
{
    // set up the 32 kHz oscillator as the input clock
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(0);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(2);
    GCLK->CLKCTRL.reg = (uint32_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos));
}

void usleepz(uint32_t usecs)
{
    uint32_t limit = (usecs * 1000) / 30500; // the lowest timestep is 30,5 usec for 32 kHz input

    // disable RTC
    RTC->MODE0.CTRL.reg &= ~RTC_MODE0_CTRL_ENABLE;
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_SWRST;

    // configure RTC in mode 0 (32-bit counter)
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_PRESCALER_DIV1 | RTC_MODE0_CTRL_MODE_COUNT32;

    // Initialize counter values
    RTC->MODE0.COUNT.reg = 0;
    RTC->MODE0.COMP[0].reg = limit;

    // enable CMP0 interrupt in the RTC
    RTC->MODE0.INTENSET.reg |= RTC_MODE0_INTENSET_CMP0;

    // enable RTC
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
    __WFI();
}

void setup()
{
    Serial.begin(115200); // this causes the stall of the program if there is no Serial connected
    while (!Serial);
    delay(200);

    if (collector.init() != 0)
        while(1);

    //if (networkManager.init(ssid, pass, lambda_serv, certificate) != 0)
    //    while(1);

    // TODO add SSL/TLS and retry posting this to the endpoint
    /*
    int status = -1;
    while (status != 0)
    {

    }
    */
    //Serial.println("Attempt sending the payload.");
    //networkManager.postWiFi(request_body);

    if (display.init() != 0)
        while(1);

    // Start screen update loop
    //Scheduler.startLoop(displayLoop);

    // attach the wake-up interrupt from a button
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(9, INPUT_PULLUP);
    attachInterrupt(9, buttonIrq, LOW);

    // FAILSAFE wait 20 seconds before going to sleep
    delay(20000);

    usleep_init();

    triggerTimeout = true;
}

void displayLoop()
{
    display.update(batch);
    delay(500);
}

void buttonIrq()
{
    //// TODO: this does not work properly with the deepsleep mode and requires special handling.
    //// Need to check if it will wake up the MCU instantaneously without any further dancing
    //// disable this interrupt in the normal operating mode

    //// create an interrupt which will work with the deepsleep mode

    //// poor man's debouncing
    //current_time = millis();
    //dupa = 0;
    //if ((current_time - debounce_time) > IRQ_DELAY)
    //{
    //    dupa = 2000;
    //    // TODO: waking up from sleep with a pushbutton will effectively wake up the screen and
    //    // make the microcontroller's peripherals run for some predefined time (after the user stopped using the device)
    //    //Serial.println("IRQ!");
    //}
    //debounce_time = current_time;


    // set up activelyUsing variable
    wakeUpMillis = millis();
    buttonPressMillis = wakeUpMillis;

    activelyUsing = true; // probably needs debouncing in the button case
}

void loop()
{
    // in the main loop

    // if the loop triggered by a timeout -> start the timer
    if (triggerTimeout)
    {
        wakeUpMillis = millis();
        triggerTimeout = false;
    }

    // check the conditions for going back to sleep -> trigger sleeping
    uint32_t currentMillis = millis();
    if (currentMillis - wakeUpMillis > ACTIVITY_INTERVAL && !activelyUsing)
    {
        digitalWrite(LED_BUILTIN, 0);
        triggerTimeout = true;

        // prepare other peripherals to sleep here
        usleepz(500000000);
    }
    digitalWrite(LED_BUILTIN, 1);

    // if user using the band: (read user input via the button)
    if (activelyUsing) // TODO: for now not testing this
    {
        // refresh timeout timer
        wakeUpMillis = millis();

        // check if time has passed since last button press and go to sleep
        if (currentMillis - buttonPressMillis > WAKEUP_INTERVAL)
        {
            Serial.println("Time to sleep ZZZ");
            activelyUsing = false;
        }

        // different intervals for data and wifi?

        // in a fixed interval
        // update the lcd
        oledTask();

        Serial.println("Actively using");
    }
    else
    {
        // in a fixed interval: 125 Hz
        // collect data
        dataTask();

        // in a fixed interval
        // send package over wifi/ble
        wifiTask();
    }


    /*
    // TODO: somehow this needs unit 10 times greater than it sleeps?
    usleepz(50000000);

    digitalWrite(LED_BUILTIN, status);
    status = !status;
    ++dupa;
    // simulate a busy loop
    //while (!readyToDim)
    //{
    //    // turn off the buton IRQ and handle it as a regular input, while last input was not handled long ago
    //    //
    //    // need to offload wifi handling and data collection in a separate task
    //    // if this task in progress, wait for completion and only then exit the loop and go to sleep
    //    // yield after checking the input etc - so other tasks
    //    need a button handling code here
    //}
    delay(10);
    // TODO: after adding the clock management code, need to structure the data collection and display loops and test it!!
    //
    //
    //Serial.println("REGULAR!");
    //long d = millis() - lastTime;
    //lastTime = millis();
    //Serial.print("DELAY ");
    //Serial.println(d);
    //Serial.println("looping...");
    //networkManager.readWiFi();
    //if (networkManager.serverDisconnectedWiFi())
    //{
    //    Serial.println("Disconnected from the server. Client stopped.");
    //    while (1);
    //}
    // collect the data
    collector.getData();
    collector.getLastData(batch);
    // perform the inference if needed
    printLastData();
    //networkManager.postWiFi()
    yield();
    */
}

void dataTask()
{
    collector.getData();
    collector.getLastData(batch);
    printLastData();
}

void wifiTask()
{

}

void oledTask()
{

}
