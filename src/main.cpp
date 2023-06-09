
#include <Arduino.h>
#include <bluefairy.h>

#include "const.h"
#include "logger.hpp"
#include "app-state.hpp"
#include "led-controller.hpp"
#include "states/booting/booting.state.hpp"
#include "states/ota/ota.state.hpp"
#include "states/animate/animate.state.hpp"
#include "rx.hpp"

bluefairy::Scheduler logicScheduler;
bluefairy::Scheduler ledScheduler;
AppStateMachine stateMachine;
LedController ledController;

OTAState otaState(&stateMachine, &logicScheduler, &ledController);
BootingState bootingState(&stateMachine, &logicScheduler, &ledController);
AnimateState animateState(&stateMachine, &logicScheduler, &ledController);

inline void ledTaskHandlerSetup() {
  Logger::getInstance().logLn("ledTaskHandlerSetup() - starting");
  int frameDuration = 1000 / LED_FPS;

  ledController.begin();

  Logger::getInstance().logLn("ledTaskHandlerSetup() - scheduling ledController.tick() every " + String(frameDuration) + "ms");
  ledScheduler.every(
      frameDuration,
      []() {
          ledController.tick();
          ledController.setBrightness(RX::ledBrightness);
      }
  );

  Logger::getInstance().logLn("ledTaskHandlerSetup() - done");
}

inline void ledTaskHandlerLoop() {
  ledScheduler.loop();
}

inline void setupStateMachine() {
  Logger::getInstance().logLn("logicTaskHandlerSetup() - preparing state machine");
  stateMachine[AppState::APP_STATE_BOOTING] = bootingState;
  stateMachine[AppState::APP_STATE_OTA] = otaState;
  stateMachine[AppState::APP_STATE_ANIMATE] = animateState;

  Logger::getInstance().logLn("logicTaskHandlerSetup() - starting state machine");
  stateMachine.toState(AppState::APP_STATE_BOOTING);

  Logger::getInstance().logLn("logicTaskHandlerSetup() - done");
}

#ifdef ESP32
  TaskHandle_t LedTask;
  void LedTaskHandler(void * parameter) {
    ledTaskHandlerSetup();

    for (;;) {
      ledTaskHandlerLoop();
    }
  }

#endif

void setup()
{
  Logger::getInstance().begin();
  Logger::getInstance().logLn("Welcome to CRSF Visualizer");

  setupStateMachine();
  RX::begin();
  logicScheduler.every(1, []() { RX::loop(); });

  #ifdef ESP32
    Logger::getInstance().logLn("ESP32 detected, starting led task");
    xTaskCreatePinnedToCore(
        LedTaskHandler,
        "LED",
        10000,
        NULL,
        0,
        &LedTask,
        0
    );
  #else
    Logger::getInstance().logLn("ESP32 not detected, running setup() in main thread");
    ledTaskHandlerSetup();
  #endif

  Logger::getInstance().logLn("setup() done, starting loop()");
}

void loop() {
  #ifndef ESP32
    ledTaskHandlerLoop();
  #endif
  logicScheduler.loop();
}