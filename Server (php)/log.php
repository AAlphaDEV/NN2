<?php

$file = fopen("log/err.log", "r");

echo '	Error log :';
echo '<br/>';
while(($line = fgets($file)) != FALSE)
{
	echo htmlspecialchars($line);
	echo '<br/>';
}

?>