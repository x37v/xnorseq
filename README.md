xnorseq
=======

Sequencer in progress

using c++11.. not likely to be real-time safe, ever

SchedulePlayer should pass down a boolean to indicate if they've jumped [location changed] in the past tick.  This way periodics can evaluate if they should keep going.

Groups should be able to have their own play rate, so in the normal situation they can just count down ticks and not worry about parent activities..
if the parent jumps back though, just have to figure out the math.

