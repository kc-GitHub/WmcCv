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
#include "Z21Slave.h"
#include "wmc_event.h"
#include <tinyfsm.hpp>

/***********************************************************************************************************************
 * T Y P E D  E F S  /  E N U M
 **********************************************************************************************************************/

/**
 * Wmc CV specific events.
 */
enum cvEventData
{
    start = 0,
    stop,
    cvNack,
    cvData,
    update
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
    static uint16_t m_cvNumber;    /* Cv number to be changed. */
    static uint16_t m_cvValue;     /* Value of cv number. */
    static uint8_t m_timeOutCount; /* Counter for timeout handling. */
};

#endif
