xnorseq
=======

Sequencer in progress

using c++11.. not likely to be real-time safe


Schedules/Groups should not contain tick state, they should just contain the
schedule and another lighter weight object could contain play state so that 
you don't worry about a group overlapping with itself.. which wouldn't work in
the current scheme..

