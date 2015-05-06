xnorseq
=======

Sequencer in progress

using c++11.. not likely to be real-time safe, ever

BUILD
------

to use clang:
CXX=clang++ make

TODO/QUESTIONS
------

use state objects for periodic evaluators instead of clone?

Classes should return a state object which is passed back to them to operate on
for start, end, periodic, and the exec\* methods should be const.  This way you
can have multiple copies of the same objects being executed with different data
and ditch the 'clone' method [that would essentially be replaced by some 'get
state object' method].  These would probably be templated, so lambda function typedefs
would have to move into the class.  PeriodicSchedFunc<int>(lambda that takes an int ref or even a shared ptr to one)

use weak_ptr for periodic evaluators?

SchedulePlayer should pass down a boolean to indicate if they've jumped [location changed] in the past tick.  This way periodics can evaluate if they should keep going.

Groups should be able to have their own play rate, so in the normal situation they can just count down ticks and not worry about parent activities..
if the parent jumps back though, just have to figure out the math.

