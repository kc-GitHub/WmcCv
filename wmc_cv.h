/**
 **********************************************************************************************************************
 * @file  wmc_cv.h
 * @brief Cv programming with the MWC.
 ***********************************************************************************************************************
 */
#ifndef WMC_CV_H
#define WMC_CV_H

/***********************************************************************************************************************
 * I N C L U D E S
 **********************************************************************************************************************/
#include "WmcTft.h"
#include "app_cfg.h"
#include <event.h>
#include <tinyfsm.hpp>

/***********************************************************************************************************************
 * T Y P E D  E F S  /  E N U M
 **********************************************************************************************************************/

/**
 * Wmc CV specific events.
 */
enum cvEventData
{
    startCv = 0,
    startPom,
    cvNack,
    cvData,
    update,
    responseNok,
    responseBusy,
    responseReady,
};

/**
 * Forwarded pul;se switch event.
 */
struct cvpulseSwitchEvent : tinyfsm::Event
{
    pulseSwitchEvent EventData;
};

/**
 * Forwarded push button event.
 */
struct cvpushButtonEvent : tinyfsm::Event
{
    pushButtonsEvent EventData;
};

/**
 * Cv module event.
 */
struct cvEvent : tinyfsm::Event
{
    cvEventData EventData;
    uint16_t cvNumber;
    uint8_t cvValue;
};

/***********************************************************************************************************************
 * C L A S S E S
 **********************************************************************************************************************/

class wmcCv : public tinyfsm::Fsm<wmcCv>
{
public:
    /* default reaction for unhandled events */
    void react(tinyfsm::Event const&){};

    virtual void react(cvEvent const&);
    virtual void react(cvpushButtonEvent const&);
    virtual void react(cvpulseSwitchEvent const&);

    virtual void entry(void){}; /* entry actions in some states */
    virtual void exit(void){};  /* no exit actions at all */

    cvProgEvent EventCvProg; /* Cv module event to other module. */

protected:
    static WmcTft m_wmcCvTft;      /* Display. */
    static uint16_t m_PomAddress;  /* Address of loc to be changed with POM. */
    static uint16_t m_cvNumber;    /* CV number to be changed. */
    static uint16_t m_cvValue;     /* Value of CV number. */
    static uint8_t m_timeOutCount; /* Counter for timeout handling. */
    static bool m_PomActive;       /* POM mode programming. */

    static const uint16_t STEP_1              = 1;    /* In - decrease by 1 */
    static const uint16_t STEP_10             = 10;   /* Increase by 10 */
    static const uint16_t STEP_100            = 100;  /* Increase by 100 */
    static const uint16_t STEP_1000           = 1000; /* Increase by 1000 */
    static const uint16_t CV_DEFAULT_NUMBER   = 1;    /* Default CV number */
    static const uint16_t CV_DEFAULT_VALUE    = 0;    /* Default CV value */
    static const uint16_t POM_DEFAULT_ADDRESS = 1;    /* Default CV value */
    static const uint16_t CV_MAX_NUMBER       = 1024; /* Maximum CV number. */
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    static const uint16_t CV_MAX_NUMBER_CV_MODE = 1024; /* Maximum CV number. */
#else
    static const uint16_t CV_MAX_NUMBER_CV_MODE = 255; /* Maximum CV number. */
#endif
    static const uint16_t CV_MAX_VALUE    = 255;  /* Maximum CV value. */
    static const uint16_t POM_MAX_ADDRESS = 9999; /* Maximum CV value. */
    static const uint8_t TIME_OUT_20_SEC  = 40;   /* Timeout counter max value based on 0.5sec update. */
    static const uint8_t TIME_OUT_10_SEC  = 20;   /* Timeout counter max value based on 0.5sec update. */
};

#endif
