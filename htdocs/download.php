<?php
	$title = "download";
	require("header.php");

	entry("what is it?", 
		'wsdebug is a debugger for the more or
		less famous whitespace programming language, coming along
		with a rather fast interpreter (wsi).
		[ <a href="index.php">more info</a> ]
		');
?><hr width="40%"><br><?php
	entry("wsdebug currently is in alpha-state",
		'I\'m currently putting much effort into progressing the
		 development of wsdebug. However my time is a quite limited
		 resource. That\'s why there currently is no release,
		 only the CVS thingy (which\'s however working quite reliable).'); 

	entry("how to get most recent CVS wsdebug thingy?",
		'do something like this on your command prompt: <pre>
		cvs -z3 -d:pserver:anonymous@cvs.wsdebug.berlios.de:/cvsroot/wsdebug co wsdebug
		</pre>');

	require("footer.php");
?>
