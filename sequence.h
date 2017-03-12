#pragma once

namespace xnorseq {
  typedef unsigned int timepoint;
  typedef unsigned int timedur;

  class Event;
  class EphemeralEvent;
  class ScheduleEvent;
  class Schedule;
  class SchedulePlayer;
  class EphemeralSchedulePlayer;
  class Seq;

  class EphemeralEvent {
    public:
      //call data should include ability to:
      //update parent schedule location ['next' exec]
      //create and add new events to schedule
      //add self back to schedule
      //
      //.. should use methods that might not do anything if it doesn't make
      //  sense.. like an 'exec' called by a trigger method that cannot be
      //  rescheduled etc.... use std function?
      class CallData {
        public:
          void seek(timepoint t); //XXX might not do anything

          void schedule(timepoint t, EphemeralEventPtr event);

          //realtime safe alloc
          template <typename T>
            EphemeralEventPtr make_event(Args&&... args);
      };

      void exec(timepoint t, CallData data);
  };

  class Event {
    public:
      void exec(timepoint t, CallData data);
  };

  class ScheduleEvent {
    public:
      //part of linked list for schedule...
      ScheduleEvent(EventPtr event);
  };

  class Schedule {
    public:
      //add event to schedule at time point t
      //realtime safe only
      void schedule(timepoint t, ScheduleEventPtr event);
  };

  class EphemeralSchedulePlayer : public EphemeralEvent {
    public:
      void exec(timepoint t, CallData data) {
        //execute from last t until t... reschedule self if needed.
      }
  };

  class SchedulePlayer : public Event {
    public:
      void exec(timepoint t, CallData data) {
        //insert an EphemeralSchedulePlayer into 'live' schedule..
        EphemeralSchedulePlayerPtr e = data.make_obj<EphemeralSchedulePlayer>(mSchedule);
        data.schedule(t, e);
      }
  };

  class Seq {
    public:
      //main thread side alloc, no need for realtime
      template <typename T>
      ObjectRef make_obj(Args&&... args);

      //realtime safe alloc
      template <typename T>
      EphemeralEventPtr make_event(Args&&... args);

      void exec(timepoint t) {
        //execute EphemeralEvent event graph
      }

      template <typename T>
      void update(ObjectRef o, std::string attr, T v) {
        //XXX make sure that object supports attribute? Should be realtime safe
        //queue up update for updating object in realtime thread.
      }
  };
}
