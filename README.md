# xnorseq

A work in progress function scheduler.

trying rust this time... new to rust..


## some psudo-code for the interface

schedule(time, function)

which calls function(with some context)

context has the ability schedule

so a start stop [midi note] method could be something like

let startStopFunction = function(startFunction, stopFunction, offsetFunction) {
	return function(context) {
		context.schedule(context.now() + offsetFunction(context), stopFunction);
		startFunction(context);
	}
}

let midiNote = function(noteNumber, velocity, channel, lengthFunction) {
	return startStopFunction(
		function(context) { context.midiOut(context.now(), noteOn(noteNumber, velocity, channel)); },
		function(context) { context.midiOut(context.now(), noteOff(noteNumber, velocity, channel)); },
		lengthFunction);
}

id = schedule(12, midiNote(12, 127, 1, function(/*don'tcare*/) { return 12; }))

midi notes could take a variant of either a u8 type or a function based on context that returns a u8 type.. then we'd just have a resolve function that gets the value when needed..

we can have update-able values with function wrappers that read from a known location..

each scheduled event returns an id so we can unschedule later..
