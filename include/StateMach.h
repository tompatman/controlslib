/********************************************************************
    Premium Power Corp

    Created     : Wed, 12, Nov 2008
    Login       : gmorehead
    File        : StateMach.h
*********************************************************************/


#pragma once


#include "ppcLogger.h"

#include "ITick.h"

#include <cassert>
#include <cstdio>
#include <vector>
#include <queue>
#include <cstdlib>

#include "Poco/Mutex.h"

using Poco::Mutex;

//----------------------------------------------------------------------------
// StateMach.h
//----------------------------------------------------------------------------


// Example state definition
//typedef enum { sm_offline,
//              sm_stateOne,
//              sm_stateTwo,
//              sm_lastState        ///< placeholder, makes it easier to index through arrays, not a real mode
//} example_states;
//
//

//  The following needs to go into the implementation file (cc or cpp)
//
//const char * EXAMPLE_STATE_NAMES[] =  {"Offline",
//                                          "State One",
//                                          "State Two"};

#define INVALID_STATE_TEXT "Invalid state"      // This is returned if the text requested is outside the state array
#define MAX_NAME_LENGTH 51

template<class states>
class StateMach {
public :

    StateMach(std::string name, states initial_state, states num_states, ITick &ticker, int base_log_level);

    virtual ~StateMach();

public :
    // Sets the new state of the machine if allowed.  Returns 1 on sucess.
    int stateChange(states new_state, const char *status_text = "");

    // Process the state.  This should be called by a thread to process the current state.
    // Returns 1 on success.
    int processState();

    // Returns the current state of the machine.
    states getCurrState();

    // Returns the text for the current state of the machine.
    const char *getCurrStateText()
    {
      return getStateText(_currState);
    };

    // Override to provide more verbose information about the current status of the machine.
    virtual std::string getStatusText()
    {
      if(_status_text.empty())
        return getStateTextStr(getCurrState());
      else
        return _status_text;
    }

    // Returns the text associated with the state value passed to the function.
    virtual const char *getStateText(states state) = 0;

    std::string getStateTextStr(states state) { return std::string(getStateText(state)); };

    // Returns the defines state machine name.
    const char *getMachineName()
    {
      return machineName;
    };

    ITick &getTicker()
    {
      return _ticker;
    }

    // Returns the current time, which is simply a mSec counter.
    Poco::Int64 getCurrTime() const;

    // Returns the time the state began.
    Poco::Int64 getStateTime(states _state);

    Poco::Int64 secondsInState();

    int secondsSince(Poco::Int64 start_tm);

    int mSecondsSince(Poco::Int64 start_tm);

    states get_numStates() const
    {
      return _numStates;
    }

private :

    /******************************************************************************************
    * The Purpose:
    *
    *   The StateMach object manages the transitions from one state to the other.
    * It uses the process described below to accomplish this.  In addition it contains
    * a timer which is not dependent on the actual system time and uses this to mark the
    * beginning of every state.  Prior to calling any function the current time is
    * determined and the time elapsed since the beginning of the state change is updated.
    *
    *   ** Every time based process or event should use the timer mechanism from this object.
    *
    * The Process:
    *
    *   stateChange(new_state)
    *       -> okStateChange(new_state)?    - Make sure it is a valid new state
    *               -> exitState(previousState) - clean up the old state.
    *               -> setupState(_pendingState)- setup the new state
    *                                           - change the _currState to _lastState
    *                                           - change the _pendingState to _currState
    *
    *   processState()
    *       -> implementState()                 - Run the state.
    *
    *   ** Note: exitState and setupState will always get called if the state will change.
    *            However, implementState may not get called if another thread preemts the state.
    *            or if the processState function is not called again!
    *
    ********************************************************************************************/


    // Validates that the requested state of the machine if allowed.
    // Returns 1 if allowed.
    virtual int okStateChange(states new_state) = 0;

    // Called within the stateChange request function if a new state has been set.
    // Returns 1 on success.
    // * Not a required function.
    // This function is called after the new state is set, but prior to setupState().
    // Good place to put substates offline and log events to files.
    // Good place to capture high priority exit control.
    // ** No state changes should occur in this function.
    // ** Failure should be avoided.  Processing continues even if this returns failure!!
    virtual int exitState(states previousState)
    {
      return 1;
    };

    // Called within the stateChange request function if a new state has been set.
    // Returns 1 on success.
    // * Not a required function.
    // This function should setup up any initial conditions for the state.
    // Good place to set special timers, manage flags, and start sub-states
    // ** No state changes should occur in this function.
    // ** Failure should be avoided.  Processing continues even if this returns a failure!!
    virtual int setupState(states nextState)
    {
      return 1;
    };

    // Implement the actual state code.
    // Returns 1 on sucess.
    // This is where:   - Hardware is manipulated
    //                  - Conditions and timers are monitored
    //                  - Internal state changes can be initiated
    virtual int implementState() = 0;

protected :

    // Call this to override the beginning time of a state.  Use with caution.
    void setStateStartTime(states _state, Poco::Int64 t);

    // Returns the last state of the machine
    states getLastState();

    // Returns the next pending state if there is one.  Otherwise it returns the current state.
    states getPendingState();

    states &getCurrStateRef() { return _currState; };

    char machineName[MAX_NAME_LENGTH];

    states _numStates;

    // The number of states defined by the derived class

    std::string _status_text;

    Poco::Int64 _msecs_elapsed, _secs_elapsed;      // Time in the current state.

    ppcLogger *_log;

    //The time ticker, which could be a real or simulated type
    ITick &_ticker;

private:
    states _currState, _lastState;

    std::queue<states> _pendingStates;

    Mutex _state_lock;                  // Mutex to synchronize control state changes

    std::vector<Poco::Int64> _STATE_START_TIME;  // Holds the start time for each state.

    int _log_level;
};


template<class states>
StateMach<states>::StateMach(std::string name, states initial_state, states num_states, ITick &ticker, int base_log_level)
        : _numStates(num_states)
          , _msecs_elapsed(0)
          , _secs_elapsed(0)
          , _currState(initial_state)
          , _lastState(initial_state)
          , _ticker(ticker)
{
  if (base_log_level < LG_VERBOSE)
    _log_level = base_log_level;
  else
    _log_level = LG_DEBUG;

  _log = new ppcLogger(name.c_str(), _log_level);

  snprintf(machineName, MAX_NAME_LENGTH, "%s", name.c_str());
  _log->log(LG_DEBUG, "StateMach::%s : num_states:%d initial_state: %d", machineName, _numStates, _currState);
  _STATE_START_TIME.resize(static_cast<unsigned int>(_numStates));

}

template<class states>
StateMach<states>::~StateMach()
{
  delete _log;
}

template<class states>
int StateMach<states>::stateChange(states new_state, const char *status_text)
{
  int retval = -1;
  _log->log(LG_VERBOSE, "%s:stateChange() : Requesting change from: %s to: %s", machineName, getStateText(_currState), getStateText(new_state));
  //lock down state changes so that the processState is safe.
  _state_lock.lock();
  //if(_state_lock.lock() != 1)
  //  ppclog(_log_level-1, "%s *** unable to lock state machine!", machineName);

  try
  {
    if (static_cast<int>(new_state) >= 0 && static_cast<int>(new_state) < static_cast<int>(_numStates))
      retval = okStateChange(new_state);  // Validate the state change.

    //if the state change is allowed process it
    if (retval == 1)
    {
      _pendingStates.push(new_state);

      if (exitState(_currState) != 1)
      {
        _log->log(LG_ERR, "%s *** tear down of state %s failed!  Continuing anyway", machineName, getStateText(_lastState));
      }

      // Only log it if it is a real change.
      if (_currState != new_state)
        _log->log(LG_INFO, "%s changing from %s to %s", machineName, getStateText(_currState), getStateText(new_state));

      if (setupState(new_state) != 1)
      {
        _log->log(LG_ERR, "%s *** setup of state %s failed!  Continuing anyway", machineName, getStateText(new_state));
      }

      _lastState = _currState;
      _currState = new_state;
      _status_text = status_text;
      _STATE_START_TIME[static_cast<int>(_currState)] = getCurrTime();
      _msecs_elapsed = 0;
      _secs_elapsed = 0;
      _pendingStates.pop();
    }
    else
    {
      _log->log(LG_ERR, "%s request to change state from %s to %s rejected!", machineName, getStateText(_currState), getStateText(new_state));
    }
  }
  catch (Poco::Exception &e)
  {
    _log->log(LG_ERR, "%s *** threw an exception during state change from %s to %s!  %s", machineName, getStateText(_currState), getStateText(new_state),
              e.message().c_str());
    _state_lock.unlock();   // Free the state machine
    throw e;
  }
  catch (std::exception &e)
  {
    _log->log(LG_ERR, "%s *** threw an exception during state change from %s to %s!  %s", machineName, getStateText(_currState), getStateText(new_state),
              e.what());
    _state_lock.unlock();   // Free the state machine
    throw e;
  }
  catch (...)
  {
    _log->log(LG_ERR, "%s *** threw an unknown exception during state change from %s to %s!  ", machineName, getStateText(_currState), getStateText(new_state));
    _state_lock.unlock();   // Free the state machine
    throw;
  }


  _state_lock.unlock();
  return retval;
}

// Process the state.  This should be called by a thread to process the current state.
// Returns 1 on success.
template<class states>
int StateMach<states>::
processState()
{
  int retval = 1;
  _state_lock.lock();   //lock down state machine

  try
  {
    _msecs_elapsed = _ticker.mSecondsSince(_STATE_START_TIME[static_cast<int>(getCurrState())]);
    _secs_elapsed = _msecs_elapsed / 1000;
    retval = implementState();  // Actually implement the state.

    if (retval != 1)
    {
      _log->log(LG_ERR, "%s *** failed in %s:%d!", machineName, getStateText(_currState), _currState);
    }
  }
  catch (Poco::Exception &e)
  {
    _log->log(LG_INFO, "%s *** threw (Poco::%s) in [%s] : %s", machineName, e.what(), getStateText(_currState), e.displayText().c_str());
    _state_lock.unlock();   // Free the state machine
    throw e;
  }
  catch (std::exception &e)
  {
    _log->log(LG_ERR, "%s *** threw an exception during implement state %s:%d!  %s", machineName, getStateText(_currState), _currState, e.what());
    _state_lock.unlock();   // Free the state machine
    throw e;
  }

  _state_lock.unlock();   // Free the state machine
  return retval;
}

template<class states>
states StateMach<states>::getCurrState()
{
  return _currState;
}

template<class states>
states StateMach<states>::getLastState()
{
  return _lastState;
}

template<class states>
states StateMach<states>::getPendingState()
{
  if (_pendingStates.size() > 0)
    return _pendingStates.front();

  _log->log(_log_level - 1, "%s *** getPendingState() called with no pending states!!  Returning current state: %s.", machineName, getStateText(_currState));
  return _currState;
}

template<class states>
Poco::Int64 StateMach<states>::getCurrTime() const
{
  return _ticker.get_num_mSeconds();
}

template<class states>
void StateMach<states>::setStateStartTime(states _state, Poco::Int64 t)
{
  _STATE_START_TIME[_state] = t;
}

template<class states>
Poco::Int64 StateMach<states>::getStateTime(states _state)
{
  return _STATE_START_TIME[_state];
}

template<class states>
Poco::Int64 StateMach<states>::secondsInState()
{
  return (((getCurrTime() - _STATE_START_TIME[getCurrState()]) / 1000));
}

template<class states>
int StateMach<states>::secondsSince(const Poco::Int64 start_tm)
{
  return _ticker.secondsSince(start_tm);
}

template<class states>
int StateMach<states>::mSecondsSince(const Poco::Int64 start_tm)
{
  return _ticker.mSecondsSince(start_tm);
}

/*********************************************************************
    File    : StateMach.h
*********************************************************************/

