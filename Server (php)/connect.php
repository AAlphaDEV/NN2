<?php 

//header('Content-Type: test/plain');

if(!isset($_GET['r']))
{
    exit(0);
}
if(isset($_GET['n']))
{
    $name = htmlspecialchars($_GET['n']);
} else
{
    $name = 'null';
}

include('utils.php');

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

if($_GET['r'] === 'connect')
{
    $ip = get_client_ip();

    $query = $db->prepare('INSERT INTO connected(ip, last_connect, name) VALUES(:ip, NOW(), :name)');
    $query->execute(array(
        'ip' => $ip,
        'name' => $name
    ));

    $id = $db->lastInsertId();

    echo 'connected';
    echo '\\\\id=' . $id;
} else if($_GET['r'] === 'disconnect')
{
    if(!isset($_GET['id']))
    {
        die('error\\\\[code]\\\\not enough parameters');
    }

    $id = htmlspecialchars($_GET['id']);

    $query = $db->prepare('DELETE FROM connected WHERE id=:id');
    $query->execute(array(
        'id' => $id
    ));

    echo 'disconnected';
}

?>