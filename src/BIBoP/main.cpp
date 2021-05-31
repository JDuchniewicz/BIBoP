#include <Arduino.h>
#include <Wire.h>
#include "wiring_private.h"

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
#include <ArduinoMqttClient.h>
#include <ACROBOTIC_SSD1306.h>

// buffer for logging
char PRINT_BUFFER[PRINTBUF_SIZE];

Batch batch; // for now this is a static container (could be a ring of data)
Config config(ssid, pass, certificate, broker, incomingTopic, outgoingTopic);

Collector collector;
Display display;
WiFiClient lambda;
BearSSLClient sslLambda(lambda);
MqttClient mqttClient(sslLambda);
NetworkManager networkManager(sslLambda, mqttClient, config);

// add a new serial because original one (pins A4 and A5) is hogged by the ECC
// D5 - SCL - pad 1
// D6 - SDA - pad 0
TwoWire sensorI2c(&sercom0, 5, 6);

volatile uint32_t wakeUpMillis = 0;
volatile uint32_t debounceMillis = 0;
volatile bool activelyUsing = false;
volatile bool pressAcknowledged = false;
volatile bool ignoreIRQ = false;

bool triggerTimeout = false;

uint32_t oledMillis = 0;
uint32_t dataMillis = 0;
uint32_t wifiPostMillis = 0;
uint32_t wifiPollMillis = 0;

constexpr auto ACTIVITY_INTERVAL = 1000; //1 s for now
constexpr auto WAKEUP_INTERVAL = 30000; //30 s for now
constexpr auto OLED_INTERVAL = 200;
constexpr auto DATA_INTERVAL = 1000 / SAMPLING_HZ;
constexpr auto WIFI_PUBLISH_INTERVAL = 60000; // 60 seconds when using the device
constexpr auto WIFI_POLL_INTERVAL = 1000;
constexpr auto BUTTON_PRESS_DELAY = 200;
constexpr auto SECOND = 1000000;
constexpr auto SLEEP_TIME = 60 * SECOND; // 90 seconds? TODO: 10 seconds for now

void buttonIrq();
void dataTask();
void wifiActivelyUsingTask();
void oledTask();
void wifiSendOnce();
void wifiPoll();
void disableIRQs();
void enableIRQs();

// move to a class
void usleep_init()
{
    // create a generic clock generator for the RTC peripheral system with ID 2
    // set up the 32 kHz oscillator as the input clock
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) |
                       GCLK_GENDIV_DIV(0);
    while (GCLK->STATUS.bit.SYNCBUSY);

    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
                        GCLK_GENCTRL_SRC_OSCULP32K |
                        GCLK_GENCTRL_ID(2);
    while (GCLK->STATUS.bit.SYNCBUSY);

    GCLK->CLKCTRL.reg = (uint32_t) (GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK2 |
                        (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos));
    while (GCLK->STATUS.bit.SYNCBUSY); // TODO: can remove cast?
}

void peripheralsOff()
{
    display.displayOff();
    collector.collectorOff();

    // turns off some peripherals for deep sleep mode
    //PM->APBCMASK.reg &= ~PM_APBCMASK_SERCOM0; // turn off SERCOM0
    // this prevents the MCU from waking up
}

void peripheralsOn()
{
    display.displayOn();
    collector.collectorOn();

    //PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;
}

void disableIRQs()
{
    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_DisableIRQ(EIC_IRQn);
    ignoreIRQ = true;
}

void enableIRQs()
{
    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_EnableIRQ(EIC_IRQn); // somehow the interrupt is acknowledged for a short while before going to sleep
    ignoreIRQ = false;
}

void usleepz(uint32_t usecs)
{
    // CARE FOR OVERFLOWS
    uint32_t limit = (usecs * 10) / 305; // the lowest timestep is 30,5 usec for 32 kHz input

    // disable RTC
    RTC->MODE0.CTRL.reg &= ~RTC_MODE0_CTRL_ENABLE;
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_SWRST;

    // configure RTC in mode 0 (32-bit counter)
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_PRESCALER_DIV1 | RTC_MODE0_CTRL_MODE_COUNT32;
    while (RTC->MODE0.STATUS.bit.SYNCBUSY);

    //// Initialize counter values
    RTC->MODE0.COUNT.reg = 0;
    RTC->MODE0.COMP[0].reg = limit;

    // enable CMP0 interrupt in the RTC
    RTC->MODE0.INTENSET.reg |= RTC_MODE0_INTENSET_CMP0;

    // enable RTC
    RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;
    while (RTC->MODE0.STATUS.bit.SYNCBUSY);

    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;           // Disable SysTick interrupts
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
    __WFI();
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;           // Enable SysTick interrupts
    NVIC_ClearPendingIRQ(EIC_IRQn); // clear any remaining IRQs
}

void setup()
{
    if constexpr(DEBUG)
    {
        Serial.begin(115200); // this causes the stall of the program if there is no Serial connected
        while (!Serial);
    }
    delay(200);

    // setup the second I2C bus
    sensorI2c.begin();
    pinPeripheral(5, PIO_SERCOM_ALT);
    pinPeripheral(6, PIO_SERCOM_ALT);

    if (collector.init(sensorI2c) != 0)
        while(1);

    if (networkManager.init() != 0)
        while(1);

    display.init(sensorI2c);

    // attach the wake-up interrupt from a button
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(9, INPUT_PULLUP);
    attachInterrupt(9, buttonIrq, LOW);
    // FAILSAFE wait 12 seconds before going to sleep
    delay(12000);

    usleep_init();

    triggerTimeout = true;
}

// cannot use I2C stuff in IRQ's as it uses clock
void buttonIrq()
{
    if (ignoreIRQ)
        return;
    // needs debouncing in the button case
    if (millis() - debounceMillis > BUTTON_PRESS_DELAY)
    {
        debounceMillis = millis();
        activelyUsing = true;
        //pressAcknowledged = true; // needs handling for OLED display changes
        print("yes, yes, yes \n");
    }
}

// TODO: add sleep enable button
void gotoSleep()
{
    // send data over wifi before sleeping
    peripheralsOff();

    wifiSendOnce();
    digitalWrite(LED_BUILTIN, 0);
    triggerTimeout = true;

    // prepare other peripherals to sleep here
    usleepz(SLEEP_TIME);
    collector.collectorOn(); // turn on only the collector (no need to wake up the screen for just data collection)
    // after wakeup wrong values are in the batch registers? // TODO: why
    batch.reset();
}

void loop()
{
    //print("looping\n");
    // in the main loop
    // if the loop triggered by a timeout -> start the timer
    if (triggerTimeout)
    {
        wakeUpMillis = millis();
        triggerTimeout = false;
    }

    // check the conditions for going back to sleep -> trigger sleeping
    if (millis() - wakeUpMillis > ACTIVITY_INTERVAL && !activelyUsing)
    {
        print("ActivelyUsing: %d\n", activelyUsing);
        print("Time to sleep ZZZ - now for realz\n");
        gotoSleep();
    }
    digitalWrite(LED_BUILTIN, 1);

    // if user using the band: (read user input via the button)
    if (activelyUsing) // TODO: why the hell this is true????
    {
        peripheralsOn();
        // different intervals for data and wifi?
        dataTask();

        // in a fixed interval
        // update the lcd
        oledTask();

        // send data in a fixed interval, poll for new packets
        wifiActivelyUsingTask();

        // refresh timeout timer
        wakeUpMillis = millis();

        // check if time has passed since last button press and go to sleep
        if (millis() - debounceMillis > WAKEUP_INTERVAL)
        {
            print("Currentmilis %ld\n", millis());
            print("Buttonpress %ld\n", debounceMillis);
            print("Time to sleep ZZZ\n");
            print("ActivelyUsing: %d\n", activelyUsing);
            activelyUsing = false;
            //display.displayOff();
            gotoSleep();
            // reset the wifi times
            wifiPostMillis = millis();
            wifiPollMillis = millis();
        }
    }
    else
    {
        // in a fixed interval: 125 Hz
        // collect data
        dataTask();

        // in a fixed interval
        // poll for incoming packets
        wifiPoll();
    }
}

void dataTask()
{
    if (millis() - dataMillis > DATA_INTERVAL)
    {
        collector.getData();
        // obtains latest data and fills the batch for the OLED
        collector.getLastData(batch);
        dataMillis = millis();
    }
}

// TODO: MQTT connection or data receival wrecks the Batch buffer data...????
void wifiActivelyUsingTask()
{
    // set up conditions for posting
    // publish only after some time passed from last check when using actively OR just woke up to collect data (handled separately)
    if (millis() - wifiPostMillis > WIFI_PUBLISH_INTERVAL)
    {
        print("Actively using wifi push\n");
        // disable interrupts
        disableIRQs();
        networkManager.reconnectWiFi();
        collector.readData(batch);
        networkManager.postWiFi(batch);
        wifiPostMillis = millis();
        //enable interrupts
        enableIRQs();
    }

    if (millis() - wifiPollMillis > WIFI_POLL_INTERVAL)
    {
        print("Actively using wifi poll\n");
        networkManager.reconnectWiFi();
        networkManager.readWiFi();
        wifiPollMillis = millis();
    }
}

void wifiSendOnce()
{
    print("Oneshot wifi posting because going to sleep\n");
    // disable interrupts
    disableIRQs();
    networkManager.reconnectWiFi();
    collector.readData(batch);
    networkManager.postWiFi(batch);
    //enable interrupts
    enableIRQs();
}

void wifiPoll()
{
    if (millis() - wifiPollMillis > WIFI_POLL_INTERVAL)
    {
        print("Actively using wifi poll\n");
        networkManager.reconnectWiFi();
        networkManager.readWiFi();
        wifiPollMillis = millis();
    }
}

void oledTask()
{
    //if (pressAcknowledged)
    //{
    //    print("TODO: change screen\n");
    //    pressAcknowledged = false;
    //}

    if (millis() - oledMillis > OLED_INTERVAL) // TODO: greater equal? or consider changing comparison conditions
    {
        display.update(batch);
        oledMillis = millis();
    }
}
