xnorseq
=======

Sequencer in progress

Time Type
---

tick.. the clock indicates what that represents.. BPM, seconds.. etc...
ticks_per_second for example..

Event
---
* has attributes
	* attributes would be updated with messages to the object as they might need
	to be reported to listeners.
	* examples:
		* `duration<TimeType>`
		* `display_start_offset<TimeType>`
		Some events will be scheduled earlier than they might be displayed, so you
		can execute an event in a non realtime context at time N - a but the event
		thinks its executing at time N and any GUI would display it at time N.
		* `display_duration<TimeType>`
			* A loop event really only needs to occur at the end of the loop
			(`GoTo(t - 10)`), but it should display as if it were a duration event, so
			it would have a *display_start_offset* and a *display_duration*
		Again, the actual duration may include some event that happens before what 
		a GUI might want to display as to the user.
		* `note_number<uint8_t>` Midi note?
		* `ephemeral` Events that other events schedule but shouldn't be stored in
		the serialized output score.. like a loop scheduling: `Goto(t - 10)` or a
		script event triggering a midi not
		* `realtime<void>` if an event isn't *realtime*, like a script, it should be executed slightly in advance in a non realtime thread.
			* example:

					//javascript script event.. executed at t - a, but the script thinks
					//its at t, can schedule events for t
					execute(Context c, TimeType t) {
						if (someCondition)
							context.schedule(t, GoTo(t - 10));
					}
	* maybe some attributes would be dynamic, like *ephemeral*? Or maybe there is an ephemeral schedule method and a non ephemeral?

* listeners -- if you change an attribute some other objects might care about
	it, for instance a container of events will want to know if one of its
	contained events grows in duration so it can update its reported length.

Clock
---
Clock runs at a fast fixed rate and when it is executed computes its next
timepoint.  This is then used to execute the sequence.. `sequence.execute(now, time_next);`
This way the sequence will evaluate all events between *now* and *time_next*

Should a clock encapsulate all *TimeType*s? Basically, it has ticks, seconds, as well as Bar/Beat?

Segment
---

Just a container for events with a single TimeType location..

`Segment<Second>::schedule(Second t, Event e);`

If its a *BarBeat* segment then it will report some sort of time signature...

Sequence
---

A *Segement* with a clock.  The clock could be synced to a global clock or potentially triggered by an event in some other sequence?

Global State
---
Might be useful if the events can access some realtime safe global state... for
instance, if you want an action to happen based on some input parameter, like,
an OSC message, you might have it reference some global named variable.

Strings would actually be indexed references to strings, so the event would
actually just have a number and that number could get swapped out if the string
changed.  "/path/to/blah" would actually just be some value n..
