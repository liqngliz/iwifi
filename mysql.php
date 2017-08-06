<?php
$host = "localhost";
$user = "Irigato";
$pass = "KMv82UvsAK23!a";
$db = "irigyDB";

$dsn = 'mysql:dbname='.$db.';host='.$host;

try {
    $pdo = new PDO($dsn, $user, $pass);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    echo 'Connection failed: ' . $e->getMessage();
}



 ?>
