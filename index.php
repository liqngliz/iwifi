<!doctype html>

<html lang="en">
<head>
  <meta charset="utf-8">
  <title>The HTML5 Herald</title>
</head>
<?php

//mysqli connection
include 'mysql.php';


//Accept Get Request DeviceID, Humidity Level, Alert No Water, WateredID
//URL + ?DeviceID=18c29d17-71ea-11e7-9c8c-e0db55fe78c8&humlvl=280&Alert=0&WateredID=18c29d31-71ea-11e7-9c8c-e0db55fe78c8

$DeviceID = $_GET["DeviceID"];
$humlvl = $_GET["humlvl"];
$lightlvl = $_GET["lightlvl"];
$Alert = $_GET["Alert"];
$WateredID = $_GET["WateredID"];
$DeviceIP = getRealIpAddr();

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

//Return mode, humlvlmax, humlvlmin, quantity, waternow, timer

?>
<body>
  <?php
  $keepWatering = 1;
  $DeviceID = htmlspecialchars($DeviceID);
  if (strlen($DeviceID) == 36){

    //return information for the micro controller
    $sql = "SELECT * FROM Devices AS d JOIN Settings AS s ON d.USID = s.USID WHERE d.UDID ='".$DeviceID."'";
    $stmt = $pdo->query($sql);
    while ($row = $stmt->fetch())
    {
      if ($WateredID == $row['UWID']){
        echo "mmxmnqtwnt:".$row['Mode']."%".
                          $row['HumMax']."%".
                          $row['HumMin']."%".
                          $row['Quantity']."%".
                          $row['WaterNow']."%".
                          $row['Timer'];
      } else {
        echo "mmxmnqtwnt:".rand(0,3)."%".
                          rand(400,450)."%".
                          rand(100,150)."%".
                          rand(1000,1200)."%".
                          rand(0,2)."%".
                          rand(3,12);
      }

        if ($WateredID == $row['UWID']){
          $keepWatering = 0;
        } else {
          $keepWatering = 1;
        }
    }

    $DeviceIP = "'".$DeviceIP."'";
    $DeviceID = "'".$DeviceID."'";
    //Update device settings on server
    $update = "UPDATE `Devices` SET `Humlvl` = ".$humlvl.", `Lightlvl` = ".$lightlvl.", `Alert` = ".$Alert.", `WaterNow` = ".$keepWatering.", `IPV4/V6` = ".$DeviceIP." WHERE `UDID` = ".$DeviceID;
    //echo $update;
    $stmt2 = $pdo->query($update);

  }
  //Close PDO connection
  $pdo = null;

   ?>

</body>
</html>
