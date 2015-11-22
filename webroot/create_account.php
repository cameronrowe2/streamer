<?php

session_start();

$req_dir = dirname($_SERVER['DOCUMENT_ROOT']).'//site_content//';
require_once $req_dir.'defs.inc';

setup_err_funcs();

// get args
$name  =  $_POST["name"];
$email =  $_POST["email"];
$pwd_1 =  $_POST["pwd_1"];
$pwd_2 =  $_POST["pwd_2"];
$r_key =  $_POST["r_key"];

create_account($name, $email, $pwd_1, $pwd_2, $r_key);

?>
