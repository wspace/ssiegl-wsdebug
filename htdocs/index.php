<?php
	$title = "information";
	require("header.php");

	entry("what is it?", 
		"wsdebug is a debugger for the more or
		less famous whitespace programming language, coming along
		with a rather fast interpreter (wsi).");
	
	entry("what the hell is whitespace?",
	        'Most programming languages like C or Perl do not care for 
		white space characters (like tab, space or newline/linefeed).
		The whitespace programming language works just the other way
		round, don\'t care for any character but those white space ones.
		<br>
		On the whole it\'s just another geeky language like Brainf\'ck
		and others, however more adicting. For details see 
		<a href="http://compsoc.dur.ac.uk/whitespace/">these pages</a>,
		put together by Edwin Brady, the inventor of this language.');

	entry("why do I need a debugger then?",
		'That\'s kinda easy to answer. If you\'ve written a whole lot
		of instructions you\'ll probably reach the point, where you
		get lost. Then just put your script into wsdebug and step
		through your bunch of whitespace instructions and watch how
		each command manipulates the stack (or heap).
		<br>
		You\'re using another interpreter, that just somewhere aborts?
		Just run your prog through either wsi and wsdebug and watch
		out what it tells, i.e. if a ws-program aborts somewhere, it
		always writes out where it crashed along with a stack dump.');

	entry("what now?",
		'<ul>'.
		  '<li><a href="download.php">download</a></li>'.
		  '<li><a href="somecode.php">some whitespace code</a></li>'.
		  '<li><a href="wspacegen.php">wspacegen</a></li>'.
		'</ul>');

	require("footer.php");
?>
