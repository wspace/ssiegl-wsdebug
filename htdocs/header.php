<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>wsdebug :: <?php echo $title; ?></title>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
	<link rel="stylesheet" type="text/css" href="style.css">
</head>

<body bgcolor="#000000" text="#bbbbbb">

<h1 align="center"><strong>wsdebug</strong> - the whitespace debugger</h1>

<table cellpadding="15" width="80%" summary=""><tr><td>

<br>

<?php
function spacy($t) {
	for($a = 0; $a < strlen($t); $a ++) {
		echo substr($t, $a, 1);
		echo "&nbsp;";
	}
	
	echo ".&nbsp;.&nbsp;.&nbsp;";
}

function entry($h,$b) {
	echo '<table width="100%" cellspacing="2"><tr><td colspan="2" class="entry_head">';
	spacy($h);
	echo '</td><tr>';
	
	echo '<tr><td width="75px">&nbsp;</td><td>';
	echo $b;
	echo '</td></tr></table><p><br>';
}
?>
