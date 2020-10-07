//
// Created by service on 4/17/17.
//

#pragma once

#include <core/StateMach.h>
#include <zmq/DataSubscription.h>
#include "process/ITestSequence.h"
#include <eventDb/EventLogger.h>
#include <process/ITestSequence.h>
#include <Factory/IBoard.h>
#include "core/ppcRunnable.h"


namespace PhytecAM335X {

typedef enum PostState {
    POST_Offline
    , POST_Prestart
    , POST_Run_Sequences
    , POST_Complete, POST_Warning
    , POST_Test_Faulted
    , POST_Num
} ePOSTState;



static const char *postStatusText(PhytecAM335X::ePOSTState ps)
{
  using namespace PhytecAM335X;
  switch(ps)
  {
    case POST_Offline: return "Offline";
    case POST_Prestart: return "Prestart";
    case POST_Run_Sequences: return "Run";
    case POST_Complete: return "Complete";
    case POST_Warning: return "Warning";
    case POST_Test_Faulted: return "Faulted";
    case POST_Num:
    default: return "Unknown";
  }
}

template<typename TData>
class StandardPOSTManager
        : public StateMach<ePOSTState> {

public:
    enum TextData {
        State
        , SequenceDetail
        , TestSubState
        , LastTestSequence
        , LastTestSequenceResult, Exceptions
        , Num
    };


    /**
     * Data from the board
     */
    TData &_boardData;

    StandardPOSTManager(const string &strName, ITick &ticker, EventLogger &evt, PhytecAM335X::Factory::IBoard &board, TData &boardData,
                        const Poco::UInt16 threadRateMs)
            : StateMach<ePOSTState>(strName.c_str(), POST_Offline, POST_Num, ticker, LG_DEBUG)
              , _evt(evt)
              , _board(board)
              , _boardData(boardData)
    {


      for (auto element : _strStatus)
      {
        element = "";
      }

    }

    ~StandardPOSTManager() override
    {

      for (auto it = _testSeq.begin(); it != _testSeq.end(); it++)
      {
        delete (*it);
      }

    };

    const char *getStateText(ePOSTState state) override
    {
      return postStatusText(state);
    };

    /**
     * Provide the text of the current test sequence
     * @return
     */
    std::string getCurrentSequenceDetail()
    {
      if (getCurrState() >= POST_Test_Faulted || getCurrState() == POST_Offline)
      {
        //return ("-");
      }
      return (_testSeq[_currSequence]->getTestDetail());

    };

    /**
 * Provide the text of the last test sequence
 * @return The lat test sequence string
 */
    std::string getLastSequenceDetail()
    {
      return (_testSeq[_lastSequence]->getTestDetail());
    };

    std::string getStatusText(enum TextData textItem)
    {
      if (textItem >= TextData::Num)
      {
        throw Poco::InvalidArgumentException("Invalid text item.");
      }

      return (_strStatus[textItem]);
    }

    bool isTestFinished() {
        return (getCurrState() == POST_Test_Faulted || getCurrState() == POST_Warning ||
                getCurrState() == POST_Complete);
    }
    void process()
    {
        processState();

        updateData();

    };


protected:


    int okStateChange(ePOSTState new_state) override
    {
      return 1;
    };

    int setupState(const ePOSTState nextState) override
    {
      ZMQ::tCommand newCmd;
      switch (nextState)
      {

        case POST_Prestart:
            _strStatus[StandardPOSTManager::TextData::Exceptions] = "";
          _evt.logEvent(LG_INFO, "StandardPOSTManager Started");
          newCmd.cmd = BaseCommands::SET_INIT_CONDITION;
          newCmd.ioAddr = 0;
          _board.handleCommands(newCmd);
          /**
           * Reset the current sequence to the beginning of the sequence list
           */
          _currSequence = 0;
          _lastSequence = 0;
              _bWarningTripped = false;
          _testSeq[_currSequence]->setup();
          break;

        case POST_Run_Sequences:
          break;

        case POST_Complete:
        case POST_Test_Faulted:
        case POST_Warning:
            break;

        default:
          break;
      }

      // Figure out how to run cal record store at the end of the calibration

      /**
       * Log the test result status on either completion or failure
       */
      //TODO: The event logger resource name needs to be the hostname
        if (nextState == POST_Complete || nextState == POST_Warning)
      {
        _evt.logEvent(EVENT_INFO, "StandardPOSTManager Result: %s", getStateText(nextState));
      }

      if (nextState == POST_Test_Faulted)
      {
        _evt.logEvent(URGENT_SERVICE_REQUEST, "StandardPOSTManager Result: %s: Last test sequence: %s->%s", getStateText(nextState),
                      _boardData.template postData.
                              template LastTestSequence, getLastSequenceDetail().c_str());
      }

      return (1);

    };

    int implementState() override
    {
      switch (getCurrState())
      {
        case POST_Test_Faulted:
        case POST_Offline:
        case POST_Complete:
        case POST_Warning:
            break;

        case POST_Prestart:
          if (_msecs_elapsed > PRESTART_WAIT)
          {
            stateChange(POST_Run_Sequences);
          }
          break;

        case POST_Run_Sequences:
          executeTestSequence();
          break;
        default:
          break;

      }
      return 1;

    };



    Poco::UInt16 _currSequence = 0;
    Poco::UInt16 _lastSequence = 0;
    const Poco::UInt16 PRESTART_WAIT = 3000;


    PhytecAM335X::Factory::IBoard &_board;

    /**
     * Event logger for test results
     */
    EventLogger &_evt;

    bool _bHasRun = false;

    void updateData()
    {

      switch (getCurrState())
      {
        case POST_Offline:
            _strStatus[TextData::State] = getStateText(getCurrState());
          break;

        case POST_Test_Faulted:
        case POST_Complete:
        case POST_Warning:
            _strStatus[TextData::LastTestSequence] = _testSeq[_lastSequence]->getName();
          _strStatus[TextData::LastTestSequenceResult] = _testSeq[_lastSequence]->getTestStatusText();
          _strStatus[TextData::SequenceDetail] = _testSeq[_lastSequence]->getTestDetail();
          _strStatus[TextData::State] = getStateText(getCurrState());
          _strStatus[TextData::TestSubState] = _testSeq[_lastSequence]->getTestSubState();
          break;

        default:
            _strStatus[TextData::LastTestSequence] =
                    _lastSequence == _currSequence ? "None" : _testSeq[_lastSequence]->getName();
          _strStatus[TextData::SequenceDetail] = getCurrentSequenceDetail();
          _strStatus[TextData::State] = _testSeq[_currSequence]->getName();
          _strStatus[TextData::TestSubState] = _testSeq[_currSequence]->getTestSubState();
          break;
      }


      _strStatus[TextData::LastTestSequenceResult] = _testSeq[_lastSequence]->getTestStatusText();

    };

    void executeTestSequence()
    {
        Process::ITestSequence::TestStatus currTestStatus;

        try {
            _testSeq[_currSequence]->run();
            currTestStatus = _testSeq[_currSequence]->getTestStatus();
        } catch (Poco::Exception &ex) {
            _log->log(LG_ERR, "POST Exception: %s", ex.message().c_str());
            ///Place the exception message in the last sequence info and update
            _strStatus[TextData::Exceptions] = ex.message();
            ///Flag the test status as failed
            currTestStatus = Process::ITestSequence::TestStatus::Failed;
        }

      switch (currTestStatus)
      {
          case Process::ITestSequence::Warning:
              _bWarningTripped = true;
          case Process::ITestSequence::Complete:
          /**
           * if the test passed, continue to the next test sequence
           */

          _lastSequence = _currSequence;
          if ((_currSequence + 1) >= (Poco::UInt16) _testSeq.size())
          {
            /**
             * All tests are done
             */
              if (_bWarningTripped) {
                  stateChange(POST_Warning);
              } else {
                  stateChange(POST_Complete);
              }
            _currSequence = 0;
          }
          else
          {
            /**
             * Prepare the next set tests
             */
            _currSequence++;
            _testSeq[_currSequence]->setup();

          }

          break;

        case Process::ITestSequence::Failed:
          /**
           * If the test failed, stop testing
           */
          _lastSequence = _currSequence;
          _currSequence = 0;
          stateChange(POST_Test_Faulted);
          break;

        case Process::ITestSequence::Running:
          /**
           * Do nothing
           */
          break;

        default:
          /**
           * Do nothing
           */
          break;
      }

    };

    std::vector<Process::ITestSequence *> _testSeq;

    std::string _strStatus[TextData::Num];

    bool _bWarningTripped = false;
};
};
