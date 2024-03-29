# wsdebug - The Whitespace Debugger

wsdebug is a debugger for the whitespace programming language. On the
whole it's something quite like the GNU debugger 'gdb'. However not so
bulky and featurerich.

In fact it's just a little tool, aiming to help out if you got lost
somewhere in your Whitespace code. If you get to the point where you
can't remember the whole stack and heap, just call this fancy tool,
step to where you lost the way and glance at the stack (or heap).

In future releases I even want to supply an editing thingy, so you
can easily manipulate stack as well as heap memory. In the long run
I'd like to have some state-storing mechanism := so you are able to
say 'save state now', debugging somewhere in the program and step
from there. Then enter 'restore' and try stepping from there again :)

You wonder why your interpreter just stops working somewhere, but
don't have a clue, where? - execute with wsdebug and have a look.

## what about --disable-gnump?

wsdebug makes use of the GNU MP library (a library for arbitrary precision
arithmetic), if you've got that one on your system. The advantage of this
library is, that numbers won't be limited. Thus if you'd like to calculate
the faculty of 1000, you can do so with GNU MP, but not without it.

This program defaults to make use of this library since the original whitespace
interpreter makes use of it as well. The price for using it - however - is,
much slower execution speed and higher memory consumption.

So if you don't really want to calculate with large integers, you probably
want to disable this feature. On very powerful equipment you probably don't
want to hesitate but enable it.

## You got some funny or obscure code, you'd like to share?

Hey, please mail it to me, I'm rather interested in such (weird) stuff.
If you want me to put it into some archives on wsdebug homepage, don't
hesitate, but ask me to.

## Problems and Feature Requests?

Please see the [GitHub repository](https://github.com/wspace/ssiegl-wsdebug)
and [webpage](https://wspace.github.io/ssiegl-wsdebug/) of wsdebug.
There you'll find a Bug-Tracker and probably everything else you need
to have.

If you want to contact me by mail, please write to stesie@gmx.de

<!-- $Id: README,v 1.2 2004/09/11 11:23:43 stesie Exp $ -->
