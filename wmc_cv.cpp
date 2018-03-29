/***********************************************************************************************************************
   @file   wc_cv.cpp
   @brief  Main application of WifiManualControl (WMC).
 **********************************************************************************************************************/

/***********************************************************************************************************************
   I N C L U D E S
 **********************************************************************************************************************/
#include "wmc_cv.h"
#include "fsmlist.hpp"
#include "wmc_app.h"
#include <tinyfsm.hpp>

/***********************************************************************************************************************
   D E F I N E S
 **********************************************************************************************************************/

/***********************************************************************************************************************
   F O R W A R D  D E C L A R A T I O N S
 **********************************************************************************************************************/
class Idle;
class EnterCvNumber;
class EnterCvValueRead;
class EnterCvValueChange;
class EnterCvWrite;

/***********************************************************************************************************************
   D A T A   D E C L A R A T I O N S (exported, local)
 **********************************************************************************************************************/

/* Init variables. */
WmcTft wmcCv::m_wmcCvTft;
uint16_t wmcCv::m_cvNumber    = 1;
uint16_t wmcCv::m_cvValue     = 0;
uint8_t wmcCv::m_timeOutCount = 0;

/***********************************************************************************************************************
  F U N C T I O N S
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Show screen and handle entering cv number to be changed.
 */
class Idle : public wmcCv
{
    /**
     */
    void entry() override{};

    /**
     * Handle cv command events.
     */
    void react(cvEvent const& e) override
    {
        switch (e.EventData)
        {
        case start:
            m_cvValue  = 1;
            m_cvNumber = 1;
            transit<EnterCvNumber>();
            break;
        case stop:
        case cvNack:
        case cvData:
        case update: break;
        }
    }
};

/***********************************************************************************************************************
 * Show screen and handle entering cv number to be changed.
 */
class EnterCvNumber : public wmcCv
{
    /**
     */
    void entry() override
    {
        m_wmcCvTft.UpdateStatus("CV programming", true, WmcTft::color_green);
        m_wmcCvTft.ShowDccNumber(m_cvNumber, true);
    };

    /**
     * Handle forwarded pulse switch events.
     */
    void react(cvpulseSwitchEvent const& e)
    {
        bool DataChanged = false;

        switch (e.EventData.Status)
        {
        case turn:
            if (e.EventData.Delta > 0)
            {
                m_cvNumber++;
                DataChanged = true;
            }
            else if (e.EventData.Delta < 0)
            {
                if (m_cvNumber > 1)
                {
                    m_cvNumber--;
                    DataChanged = true;
                }
                else
                {
                    m_cvNumber  = 999;
                    DataChanged = true;
                }
            }
            break;
        case pushturn:
            if (e.EventData.Delta > 0)
            {
                m_cvNumber += 10;
                DataChanged = true;
            }
            else if (e.EventData.Delta < 0)
            {
                if (m_cvNumber > 10)
                {
                    m_cvNumber -= 10;
                    DataChanged = true;
                }
                else
                {
                    if (m_cvNumber > 1)
                    {
                        m_cvNumber -= 1;
                        DataChanged = true;
                    }
                    else
                    {
                        m_cvNumber  = 999;
                        DataChanged = true;
                    }
                }
            }
            break;
        case pushedShort:
            EventCvProg.Request = cvExit;
            send_event(EventCvProg);
            break;

            break;
        case pushedNormal:
        case pushedlong: transit<EnterCvValueRead>(); break;
        }

        if (DataChanged == true)
        {
            if (m_cvNumber > 999)
            {
                m_cvNumber = 1;
            }
            m_wmcCvTft.ShowDccNumber(m_cvNumber, false);
        }
    }

    /**
     * Handle forwarded push button events.
     */
    void react(cvpushButtonEvent const& e)
    {
        bool DataChanged = false;

        switch (e.EventData.Button)
        {
        case button_0:
            m_cvNumber  = 1;
            DataChanged = true;
            break;
        case button_1:
            m_cvNumber += 1;
            DataChanged = true;
            break;
        case button_2:
            m_cvNumber += 10;
            DataChanged = true;
            break;
        case button_3:
            m_cvNumber += 100;
            DataChanged = true;
            break;
        case button_4:
        case button_5:
        case button_none:
        case button_power: break;
        }

        if (DataChanged == true)
        {
            if (m_cvNumber > 999)
            {
                m_cvNumber = 999;
            }
            m_wmcCvTft.ShowDccNumber(m_cvNumber, false);
        }
    }

    /**
     * Handle cv command events.
     */
    void react(cvEvent const& e) override
    {
        switch (e.EventData)
        {
        case stop: transit<Idle>(); break;
        case start:
        case cvNack:
        case cvData:
        case update: break;
        }
    }
};

/***********************************************************************************************************************
 * Handle reading cv value
 */
class EnterCvValueRead : public wmcCv
{
    /**
     * Init modules and start connection to wifi network.
     */
    void entry() override
    {
        m_wmcCvTft.UpdateStatus("Reading CV", true, WmcTft::color_green);
        EventCvProg.Request  = cvRead;
        EventCvProg.CvNumber = m_cvNumber;
        send_event(EventCvProg);

        m_timeOutCount = 0;
        m_wmcCvTft.UpdateRunningWheel(m_timeOutCount);
    };

    /**
     * Handle cv command events.
     */
    void react(cvEvent const& e) override
    {
        switch (e.EventData)
        {
        case stop: transit<Idle>(); break;
        case start:
        case cvNack: transit<EnterCvValueChange>(); break;
        case cvData:
            m_cvValue = e.cvValue;
            transit<EnterCvValueChange>();
            break;
        case update:
            m_timeOutCount++;
            m_wmcCvTft.UpdateRunningWheel(m_timeOutCount);

            /* If after 20 seconds still no response continue.... */
            if (m_timeOutCount > 40)
            {
                transit<EnterCvValueChange>();
            }
            break;
        }
    }
};

/***********************************************************************************************************************
 * Handle changing the cv value.
 */
class EnterCvValueChange : public wmcCv
{
    /**
     */
    void entry() override
    {
        m_wmcCvTft.UpdateStatus("CV programming", true, WmcTft::color_green);
        m_wmcCvTft.ShowDccValue(m_cvValue, true);
    };

    /**
     * Handle forwarded pulse switch events.
     */
    void react(cvpulseSwitchEvent const& e)
    {
        bool DataChanged = false;

        switch (e.EventData.Status)
        {
        case turn:
            if (e.EventData.Delta > 0)
            {
                m_cvValue++;
                DataChanged = true;
            }
            else if (e.EventData.Delta < 0)
            {
                if (m_cvValue > 0)
                {
                    m_cvValue--;
                    DataChanged = true;
                }
                else
                {
                    m_cvValue   = 255;
                    DataChanged = true;
                }
            }
            break;
        case pushturn:
            if (e.EventData.Delta > 0)
            {
                m_cvValue += 10;
                DataChanged = true;
            }
            else if (e.EventData.Delta < 0)
            {
                if (m_cvValue > 10)
                {
                    m_cvValue -= 10;
                    DataChanged = true;
                }
                else
                {
                    if (m_cvValue > 0)
                    {
                        m_cvValue -= 1;
                        DataChanged = true;
                    }
                    else
                    {
                        m_cvValue   = 255;
                        DataChanged = true;
                    }
                }
            }
            break;
        case pushedShort:
            /* Back to entering cv number. */
            m_wmcCvTft.ShowDccValueRemove();
            transit<EnterCvNumber>();
            break;
        case pushedNormal:
        case pushedlong: transit<EnterCvWrite>(); break;
        }

        if (DataChanged == true)
        {
            if (m_cvValue > 255)
            {
                m_cvValue = 0;
            }
            m_wmcCvTft.ShowDccValue(m_cvValue, false);
        }
    }

    /**
     * Handle forwarded push button events.
     */
    void react(cvpushButtonEvent const& e)
    {
        bool DataChanged = false;

        switch (e.EventData.Button)
        {
        case button_0:
            m_cvValue   = 1;
            DataChanged = true;
            break;
        case button_1:
            m_cvValue += 1;
            DataChanged = true;
            break;
        case button_2:
            m_cvValue += 10;
            DataChanged = true;
            break;
        case button_3:
            m_cvValue += 100;
            DataChanged = true;
            break;
        case button_4:
        case button_5:
        case button_none:
        case button_power: break;
        }

        if (DataChanged == true)
        {
            if (m_cvValue > 255)
            {
                m_cvValue = 0;
            }
            m_wmcCvTft.ShowDccValue(m_cvValue, false);
        }
    }

    /**
     * Handle cv command events.
     */
    void react(cvEvent const& e) override
    {
        switch (e.EventData)
        {
        case stop: transit<Idle>(); break;
        case start:
        case cvNack:
        case cvData:
        case update: break;
        }
    }
};

/***********************************************************************************************************************
 * Write Cv value and wait for result of cv programming.
 */
class EnterCvWrite : public wmcCv
{
    /**
     * Init modules and start connection to wifi network.
     */
    void entry() override
    {
        m_wmcCvTft.UpdateStatus("Writing CV", true, WmcTft::color_green);
        EventCvProg.Request  = cvWrite;
        EventCvProg.CvNumber = m_cvNumber;
        EventCvProg.CvValue  = m_cvValue;
        send_event(EventCvProg);

        m_timeOutCount = 0;
        m_wmcCvTft.UpdateRunningWheel(m_timeOutCount);
    };

    /**
     * Handle cv command events.
     */
    void react(cvEvent const& e) override
    {
        switch (e.EventData)
        {
        case stop: transit<Idle>(); break;
        case start: break;
        case cvNack:
        case cvData:
            /* Programming ok, back to entering cv number for next CV. */
            m_wmcCvTft.ShowDccValueRemove();
            transit<EnterCvNumber>();
            break;
        case update:
            m_timeOutCount++;
            m_wmcCvTft.UpdateRunningWheel(m_timeOutCount);

            /* If after 10 seconds still no response continue.... */
            if (m_timeOutCount > 20)
            {
                transit<EnterCvValueChange>();
            }
            break;
        }
    }
};

/***********************************************************************************************************************
 * Default event handlers when not declared in states itself.
 */

void wmcCv::react(cvpulseSwitchEvent const&){};
void wmcCv::react(cvpushButtonEvent const&){};
void wmcCv::react(cvEvent const&){};

/***********************************************************************************************************************
 * Initial state.
 */
FSM_INITIAL_STATE(wmcCv, Idle)
