<?php
	$title = "wspacegen";
	require("header.php");

	entry("what is it?", 
		'wsdebug is a debugger for the more or
		less famous whitespace programming language, coming along
		with a rather fast interpreter (wsi).
		[ <a href="index.php">more info</a> ]
		');

?><hr width="40%"><br><?php

	entry("wspacegen",
		'wspacegen is a nice little tool, allowing to easily put
		 a whitespace program together. It offers you a hierarchic
		 menu system, where you simply choose the commands you\'d
		 like to append, one after another.
		 <br>It\'s using wsdebug\'s approach towards memory management,
		 actually wspacegen and wsdebug used to be one thing, but
		 got forked right after starting.');

	entry("wspacegen's hiccups?",
		'It\'s yet in pre-alpha state, not allowing to edit or delete
		 stuff. But on the whole I guess it\'s a good idea and a new
		 approach towards whitespace coding.'); 
	
	entry("where can I get it?",
		'If you\'d like to have a closer look at wspacegen,
		<a href="http://sf.net/projects/wspacegen">come here</a>');

	require("footer.php");
?>
