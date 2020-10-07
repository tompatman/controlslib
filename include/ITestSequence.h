//
// Created by service on 4/21/17.
//

#pragma once

#include <string>
#include <Poco/Types.h>
#include <Poco/Exception.h>
#include <Poco/NumberFormatter.h>

namespace Process {

class ITestSequence {

public:

    virtual ~ITestSequence() = default;


    enum TestStatus {
        Idle = 0, Running = 0x1, Complete = 0x10
        , Failed = 0x100
        , Warning = 0x1000
    };

    virtual void setup() = 0;

    /**
     * Set a different state to jump to upon a test completion instead of the next test in the sequence.
     * @tparam eStateType
     * @param resumeState
     */
    template<typename eStateType>
    void setStateJump(eStateType resumeState)
    {
      _bTestJump = true;
      _eResumeState = resumeState;
    }

    /**
     * If using this with a state machine, process the test sequence and make a state change decision.
     * @tparam MyStateMachine The state machine template class.
     * @tparam eStateType The state machine template enumeration.
     * @param seqStateMach The state machine object.
     * @param totalStates The total number of states in the state machine.
     */
    template<class MyStateMachine, typename eStateType>
    void processSequence(MyStateMachine &seqStateMach, eStateType totalStates)
    {

      seqStateMach.template processState();
      _strSeqSubState = seqStateMach.template getStateTextStr((eStateType) seqStateMach.template getCurrState());

        /**
         * Do nothing if the test has passed or failed or completed with warning
         */
        if (_testStatus == ITestSequence::Complete || _testStatus == ITestSequence::Failed ||
            _testStatus == ITestSequence::Warning)
      {
        return;
      }


      switch (_currSeqStatus)
      {
        case ITestSequence::Failed:
          _testStatus = _currSeqStatus;
          faultedAction();
          break;

          case ITestSequence::Complete:
        case ITestSequence::Warning:
          if (_bTestJump)
          {
            /**
             * Need to jump to a previously stored state instead of the next test in the sequence
             */
            _statusCode = 0;
            seqStateMach.template stateChange((eStateType) (_eResumeState));
            _bTestJump = false;
            _eResumeState = 0;
          }
          if ((eStateType) ((Poco::Int16) seqStateMach.template getCurrState() + 1) == totalStates)
          {
            /**
             * All tests completed. Change to offline state, which is always 0.
             */
            _statusCode = 0;
            seqStateMach.template stateChange((eStateType) (0));
              _testStatus = _currSeqStatus;
            _strDetailedInfo = "Complete";
          }
          else
          {
            _statusCode = 0;
              _currSeqStatus = ITestSequence::Running;
            seqStateMach.template stateChange((eStateType) ((Poco::Int16) seqStateMach.template getCurrState() + 1));
            _strDetailedInfo = "No Data";
          }
          break;

        case ITestSequence::Idle:
          _testStatus = ITestSequence::Idle;
          break;

        case ITestSequence::Running:
          break;

        default:
          _testStatus = ITestSequence::Failed;
          _strSeqSubState = "Unknown State";
          break;
      }

    };

    /**
     * Run the test sequence directly or if used in concert with a state machine, call processState followed by
     * processSequence.
     */
    virtual void run() = 0;

    virtual std::string getName() = 0;

    TestStatus getTestStatus()
    { return (_testStatus); };

    std::string getTestStatusText()
    {
      switch (_testStatus)
      {
        case Idle:
          return ("Idle");
        case Running:
          return ("Running");
          case Complete:
              return ("Complete");
        case Failed:
          return ("Failed");
        case Warning:
          return ("Warning");
      }

      return ("Unknown " + Poco::NumberFormatter::formatHex(_testStatus));
    }

    Poco::Int16 getFaultCode()
    { return (_code); };

    std::string getTestSubState()
    {
      return (_strSeqSubState);
    };

    std::string getTestDetail()
    {
      return (_strDetailedInfo);
    };

    std::string getWarnings()
    {
      return (_strWarnings);
    }

    /**
     * Pass test methods into this, if they return a failure, the test sequence becomes a failed mode
     * and an exception is thrown to stop the test from continuing.
     * @param bFailed indicates a test failed
     * @return true on failure.
     */
    void FailCheck(const bool bFailed)
    {
      if (!bFailed)
      {
        return;
      }

      _currSeqStatus = Failed;
      _testStatus = Failed;

      throw Poco::IOException("Test Failure has occured!");


    }

    /**
 * Pass test methods into this, if they return a failure, the test sequence becomes a warning mode
 * but the test can continue.
 * @param bFailed indicates a test failed
 * @return true on failure.
 */
    bool WarningCheck(const bool bFailed)
    {
      if (!bFailed)
      {
        return (false);
      }

      _currSeqStatus = Warning;

      return (true);

    }

    /**
    * Function which does nothing, used for idle test state when finished
    */
    virtual ITestSequence::TestStatus idle()
    {
      _strSeqSubState = "Idle";
      return (TestStatus::Idle);
    };


    virtual TestStatus testOn(const bool bValue, const Poco::UInt16 msPassed, const Poco::UInt16 uiDelayMs = STD_TEST_IO_DELAY)
    {
      try
      {
        if (msPassed > uiDelayMs)
        {
          FailCheck(!(bValue));
            _currSeqStatus = ITestSequence::Complete;
        }
        else
        {
          _currSeqStatus = ITestSequence::Running;
        }

      }
      catch (Poco::Exception &ex)
      {

      }

      return (_currSeqStatus);

    }

    virtual TestStatus testOff(const bool bValue, const Poco::UInt16 msPassed, const Poco::UInt16 uiDelayMs = STD_TEST_IO_DELAY)
    {
      try
      {
        if (msPassed > uiDelayMs)
        {
          FailCheck(bValue);
            _currSeqStatus = Complete;
        }
        else
        {
          _currSeqStatus = Running;
        }

      }
      catch (Poco::Exception &ex)
      {

      }


      return (_currSeqStatus);

    }

    virtual TestStatus testOnWarn(const bool bValue, const Poco::UInt16 msPassed, const Poco::UInt16 uiDelayMs = STD_TEST_IO_DELAY)
    {
      try
      {
        if (msPassed > uiDelayMs)
        {
          if (!WarningCheck(!(bValue)))
          {
              _currSeqStatus = ITestSequence::Complete;
          }
          else
          {
            _currSeqStatus = ITestSequence::Warning;
          }

        }
        else
        {
          _currSeqStatus = ITestSequence::Running;
        }

      }
      catch (Poco::Exception &ex)
      {

      }

      return (_currSeqStatus);

    }

    virtual TestStatus testOffWarn(const bool bValue, const Poco::UInt16 msPassed, const Poco::UInt16 uiDelayMs = STD_TEST_IO_DELAY)
    {
      try
      {
        if (msPassed > uiDelayMs)
        {
          if (!WarningCheck(bValue))
          {
              _currSeqStatus = Complete;
          }
          else
          {
            _currSeqStatus = Warning;
          }
        }
        else
        {
          _currSeqStatus = Running;
        }

      }
      catch (Poco::Exception &ex)
      {

      }


      return (_currSeqStatus);

    }

    /**
     * Test that the value of two signals match
     * @param bValue1 Value 1
     * @param bValue2 Value 2
     * @param msPassed The number of mS passed in state
     * @param uiDelayMs The amount of mS to wait before testing for a match
     * @return Test status
     */
    virtual TestStatus testMatch(const bool bValue1, const bool bValue2, const Poco::UInt16 msPassed, const Poco::UInt16 uiDelayMs = STD_TEST_IO_DELAY)
    {
      try
      {
        if (msPassed > uiDelayMs)
        {
          FailCheck((bValue1 != bValue2));
            _currSeqStatus = Complete;
        }
        else
        {
          _currSeqStatus = Running;
        }

      }
      catch (Poco::Exception &ex)
      {

      }


      return (_currSeqStatus);

    }

    /**
     * If working with the status code, use the code to update the current test sequence result
     */
    void evaluateStatusCode()
    {
      if ((_statusCode & Failed) > 0)
      {
        _currSeqStatus = Failed;
      }
      else if ((_statusCode & Running) > 0)
      {
        _currSeqStatus = Running;
      }


      _statusCode = 0;
    }

    /**
     * This is called when either the test sequence fails
     * @return
     */
    virtual void faultedAction()
    {};

    static const Poco::UInt16 STD_TEST_IO_DELAY = 100;
    /**
     * Hold the status of this overall test
     */
    TestStatus _testStatus = Idle;

    /**
     * Hold the status of the specific sequence being run
     */
    TestStatus _currSeqStatus = Idle;

    std::string _strSeqSubState = "Unknown";

    std::string _strDetailedInfo = "Unknown";

    std::string _strWarnings = "";

    Poco::Int16 _code;

    /**
     * Used to store return codes from a series of on off tests by logical oring
     */
    Poco::UInt16 _statusCode = 0;

    /**
    * When tests are repeated, this indicator is set to indicate that on completion of the test, the next state to jump to is
    * stored, not just the next test in state list
    */
    bool _bTestJump = false;
    Poco::Int16 _eResumeState = 0;

};

}
