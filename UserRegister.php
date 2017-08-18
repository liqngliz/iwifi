<!doctype html>

<html lang="en">
<head>
  <meta charset="utf-8">
  <title>The HTML5 Herald</title>
</head>

<?php
include 'mysql.php';
//Using Password_Encrypt and Password_Verify Method, no separate hash insalled
$User = $_POST["UserName"];
$Password = $_POST["Password"];
$Email = $_POST["Email"];
//$Phone = $_Post["Phone"];

//Connection types, not linked to user but timestamped
$DeviceType = $_SERVER['HTTP_USER_AGENT'];
$DeviceIP = getRealIpAddr();

//Get Real IP of Connection
function getRealIpAddr()
{
    if (!empty($_SERVER['HTTP_CLIENT_IP']))   //check ip from share internet
    {
      $ip=$_SERVER['HTTP_CLIENT_IP'];
    }
    elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))   //to check ip is pass from proxy
    {
      $ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
    }
    else
    {
      $ip=$_SERVER['REMOTE_ADDR'];
    }
    return $ip;
}

$options = [
    'cost' => 15,
];

//check that email and user name don't already exist
$checksql = 'SELECT COUNT(*) AS c FROM Users WHERE User = :u';
$params = array( ':u' => $User, );
$c = $pdo->prepare($checksql);
$c->execute($params);
$cresults = $c->fetch();



?>

<body>
<?php

if ($cresults[0] > 0) {
  echo 'User or Email already exists';
} else {

  //Create hash of password
  $hashedPass = password_hash($Password, PASSWORD_BCRYPT);

  //Prepare statement for INSERT

  $sql = 'INSERT INTO Users (UUID, User, Email, Password)
          VALUES(uuid(), :usr, :e, :p)';
  $paramsql = array( ':usr' => $User, ':e' => $Email, ':p' => $hashedPass,);
  $s = $pdo->prepare($sql);
  $s->execute($paramsql);

  //notify that user has been created
  echo $User.'<br>';
  echo $Email.'<br>';
  echo 'Registered User check email for notification';
}

$pdo = null;
?>
<p>test</p>
</body>
</html>
